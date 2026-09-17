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

TMap<FString, FWebzenResultDelegate>& UWebzenSDKSubsystem::Pending()
{
	static TMap<FString, FWebzenResultDelegate> Map;
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

	void HandleDispatchResult(const char* ResultJson)
	{
		const FString Payload = UTF8_TO_TCHAR(ResultJson);

		FWebzenResult Result;
		TSharedPtr<FJsonObject> Json;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
		if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		{
			Json->TryGetStringField(TEXT("request_id"), Result.RequestId);
			Json->TryGetBoolField(TEXT("success"), Result.bSuccess);
			Json->TryGetStringField(TEXT("error_code"), Result.ErrorCode);
			Json->TryGetStringField(TEXT("error_message"), Result.ErrorMessage);
			Json->TryGetStringField(TEXT("data"), Result.Data);
		}

		if (Result.RequestId.IsEmpty())
		{
			return;
		}

		// Correlates back to the right delegate via Result.RequestId
		// instead of a native user_data, exactly like the Unity bridge
		// (WebzenSDK.cs) -- see core/request.hpp.
		FWebzenResultDelegate OnComplete;
		{
			FScopeLock Lock(&UWebzenSDKSubsystem::PendingLock());
			if (FWebzenResultDelegate* Found = UWebzenSDKSubsystem::Pending().Find(Result.RequestId))
			{
				OnComplete = *Found;
				UWebzenSDKSubsystem::Pending().Remove(Result.RequestId);
			}
			else
			{
				return;
			}
		}

		// Fires on the SDK's own worker thread; delegates/UObjects must
		// only be touched on the game thread.
		AsyncTask(ENamedThreads::GameThread, [OnComplete, Result]() { OnComplete.Broadcast(Result); });
	}
}

void UWebzenSDKSubsystem::Dispatch(const FString& CommandId, const FString& RequestJson, const FWebzenResultDelegate& OnComplete)
{
	FString RequestId;
	{
		TSharedPtr<FJsonObject> Json;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RequestJson);
		if (FJsonSerializer::Deserialize(Reader, Json) && Json.IsValid())
		{
			Json->TryGetStringField(TEXT("request_id"), RequestId);
		}
	}

	if (RequestId.IsEmpty())
	{
		OnComplete.ExecuteIfBound(FWebzenResult{.ErrorCode = TEXT("invalid_request"), .ErrorMessage = TEXT("missing request_id")});
		return;
	}

	{
		FScopeLock Lock(&PendingLock());
		Pending().Add(RequestId, OnComplete);
	}

	const DispatchCommandFn Fn = ResolveDispatchCommand();
	if (!Fn)
	{
		FScopeLock Lock(&PendingLock());
		Pending().Remove(RequestId);
		OnComplete.ExecuteIfBound(FWebzenResult{.RequestId = RequestId, .ErrorCode = TEXT("native_library_unavailable")});
		return;
	}

	Fn(TCHAR_TO_UTF8(*CommandId), TCHAR_TO_UTF8(*RequestJson), &HandleDispatchResult);
}

void UWebzenSDKSubsystem::Initialize(const FRequestInitialize& Request, const FWebzenResultDelegate& OnComplete)
{
	const FString Json = BuildRequestJson([&](const TSharedRef<FJsonObject>& Obj) {
		Obj->SetStringField(TEXT("base_url"), Request.BaseUrl);
	});
	Dispatch(TEXT("core.initialize"), Json, OnComplete);
}

void UWebzenSDKSubsystem::Login(const FRequestLogin& Request, const FWebzenResultDelegate& OnComplete)
{
	const FString Json = BuildRequestJson([&](const TSharedRef<FJsonObject>& Obj) {
		Obj->SetStringField(TEXT("provider_id"), Request.ProviderId);
		Obj->SetStringField(TEXT("provider_token"), Request.ProviderToken);
		Obj->SetStringField(TEXT("device_id"), Request.DeviceId);
	});
	Dispatch(TEXT("auth.login"), Json, OnComplete);
}
