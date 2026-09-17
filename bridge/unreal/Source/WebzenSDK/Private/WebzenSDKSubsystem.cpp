#include "WebzenSDKSubsystem.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#if PLATFORM_ANDROID
#include <dlfcn.h>
#else
#include "core/sdk_c_api.h"
#endif

TMap<FString, TFunction<void(const FString&)>>& UWebzenSDKSubsystem::Pending()
{
	static TMap<FString, TFunction<void(const FString&)>> Map;
	return Map;
}

FCriticalSection& UWebzenSDKSubsystem::PendingLock()
{
	static FCriticalSection Lock;
	return Lock;
}

namespace
{
	using DispatchCommandFn = void (*)(const char*, const char*, WebzenResultCallback);

#if PLATFORM_ANDROID
	// libwebzen_core.so lives inside the AAR UBT embeds (WebzenSDK_UPL_Android.xml),
	// not something UBT links against at build time, so its exported C
	// symbols are resolved with dlopen/dlsym at runtime instead. It's
	// already loaded into the process by the time any game code runs --
	// see WebzenContextProvider.kt's onCreate -- so RTLD_NOLOAD just grabs
	// the existing handle rather than loading a second copy; the plain
	// dlopen fallback exists only in case that assumption ever breaks.
	void* GetLibraryHandle()
	{
		static void* Handle = [] {
			void* H = dlopen("libwebzen_core.so", RTLD_NOW | RTLD_NOLOAD);
			return H ? H : dlopen("libwebzen_core.so", RTLD_NOW);
		}();
		return Handle;
	}

	DispatchCommandFn ResolveDispatchCommand()
	{
		static auto Fn = reinterpret_cast<DispatchCommandFn>(dlsym(GetLibraryHandle(), "Webzen_DispatchCommand"));
		return Fn;
	}
#else
	DispatchCommandFn ResolveDispatchCommand()
	{
		return &Webzen_DispatchCommand;
	}
#endif

	// Generates a fresh RequestId per call -- see the comment on
	// FWebzenRequest for why this doesn't come from the request struct.
	FString BuildRequestJson(TFunctionRef<void(TSharedRef<FJsonObject>)> AddFields)
	{
		const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetStringField(TEXT("request_id"), FGuid::NewGuid().ToString(EGuidFormats::Digits));
		AddFields(Json);

		FString Out;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
		FJsonSerializer::Serialize(Json, Writer);
		return Out;
	}

	FString ExtractRequestId(const FString& Json)
	{
		TSharedPtr<FJsonObject> Parsed;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		FString RequestId;
		if (FJsonSerializer::Deserialize(Reader, Parsed) && Parsed.IsValid())
		{
			Parsed->TryGetStringField(TEXT("request_id"), RequestId);
		}
		return RequestId;
	}

	void HandleDispatchResult(const char* ResultJson)
	{
		const FString Payload = UTF8_TO_TCHAR(ResultJson);
		const FString RequestId = ExtractRequestId(Payload);
		if (RequestId.IsEmpty())
		{
			return;
		}

		// Correlates back to the right handler via RequestId instead of a
		// native user_data, exactly like the Unity bridge (WebzenSDK.cs) --
		// see core/request.hpp.
		TFunction<void(const FString&)> Handler;
		{
			FScopeLock Lock(&UWebzenSDKSubsystem::PendingLock());
			if (TFunction<void(const FString&)>* Found = UWebzenSDKSubsystem::Pending().Find(RequestId))
			{
				Handler = *Found;
				UWebzenSDKSubsystem::Pending().Remove(RequestId);
			}
			else
			{
				return;
			}
		}

		// Fires on the SDK's own worker thread; delegates/UObjects must
		// only be touched on the game thread.
		AsyncTask(ENamedThreads::GameThread, [Handler, Payload]() { Handler(Payload); });
	}
}

void UWebzenSDKSubsystem::DispatchRaw(const FString& CommandId, const FString& RequestJson, TFunction<void(const FString&)> OnRawResult)
{
	const FString RequestId = ExtractRequestId(RequestJson);
	if (RequestId.IsEmpty())
	{
		OnRawResult(TEXT(R"({"success":false,"error_code":"invalid_request","error_message":"missing request_id"})"));
		return;
	}

	{
		FScopeLock Lock(&PendingLock());
		Pending().Add(RequestId, OnRawResult);
	}

	const DispatchCommandFn Fn = ResolveDispatchCommand();
	if (!Fn)
	{
		FScopeLock Lock(&PendingLock());
		Pending().Remove(RequestId);
		OnRawResult(FString::Printf(TEXT(R"({"request_id":"%s","success":false,"error_code":"native_library_unavailable"})"), *RequestId));
		return;
	}

	Fn(TCHAR_TO_UTF8(*CommandId), TCHAR_TO_UTF8(*RequestJson), &HandleDispatchResult);
}

void UWebzenSDKSubsystem::Initialize(const FRequestInitialize& Request, const FWebzenResultDelegate& OnComplete)
{
	const FString Json = BuildRequestJson([&](const TSharedRef<FJsonObject>& Obj) {
		Obj->SetStringField(TEXT("base_url"), Request.BaseUrl);
	});

	DispatchRaw(TEXT("core.initialize"), Json, [OnComplete](const FString& ResultJson) {
		FWebzenResult Result;
		TSharedPtr<FJsonObject> Json;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResultJson);
		if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		{
			Json->TryGetStringField(TEXT("request_id"), Result.RequestId);
			Json->TryGetBoolField(TEXT("success"), Result.bSuccess);
			Json->TryGetStringField(TEXT("error_code"), Result.ErrorCode);
			Json->TryGetStringField(TEXT("error_message"), Result.ErrorMessage);
		}
		OnComplete.ExecuteIfBound(Result);
	});
}

void UWebzenSDKSubsystem::Login(const FRequestLogin& Request, const FWebzenLoginResultDelegate& OnComplete)
{
	const FString Json = BuildRequestJson([&](const TSharedRef<FJsonObject>& Obj) {
		Obj->SetStringField(TEXT("provider_id"), Request.ProviderId);
		Obj->SetStringField(TEXT("provider_token"), Request.ProviderToken);
		Obj->SetStringField(TEXT("device_id"), Request.DeviceId);
	});

	DispatchRaw(TEXT("auth.login"), Json, [OnComplete](const FString& ResultJson) {
		FResultLogin Result;
		TSharedPtr<FJsonObject> Json;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResultJson);
		if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		{
			Json->TryGetStringField(TEXT("request_id"), Result.RequestId);
			Json->TryGetBoolField(TEXT("success"), Result.bSuccess);
			Json->TryGetStringField(TEXT("error_code"), Result.ErrorCode);
			Json->TryGetStringField(TEXT("error_message"), Result.ErrorMessage);
			Json->TryGetStringField(TEXT("user_id"), Result.UserId);
			Json->TryGetStringField(TEXT("access_token"), Result.AccessToken);
			Json->TryGetStringField(TEXT("refresh_token"), Result.RefreshToken);
			Json->TryGetNumberField(TEXT("expires_at"), Result.ExpiresAt);
		}
		OnComplete.ExecuteIfBound(Result);
	});
}
