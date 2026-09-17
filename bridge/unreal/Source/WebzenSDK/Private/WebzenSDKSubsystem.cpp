#include "WebzenSDKSubsystem.h"

#include "Async/Async.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#elif PLATFORM_IOS || PLATFORM_WINDOWS
#include "webzen/sdk_c_api.h"
#endif

namespace
{
	struct FLoginCallbackContext
	{
		FWebzenLoginResult OnComplete;
	};

#if PLATFORM_IOS || PLATFORM_WINDOWS
	void HandleDispatchResult(int Success, const char* JsonPayload, void* UserData)
	{
		auto* Context = static_cast<FLoginCallbackContext*>(UserData);
		const FString Payload = UTF8_TO_TCHAR(JsonPayload);
		const bool bSuccess = Success != 0;

		TSharedPtr<FJsonObject> Json;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Payload);
		FJsonSerializer::Deserialize(Reader, Json);

		FString UserId, AccessToken, ErrorMessage;
		if (Json.IsValid())
		{
			Json->TryGetStringField(TEXT("user_id"), UserId);
			Json->TryGetStringField(TEXT("access_token"), AccessToken);
			Json->TryGetStringField(TEXT("error"), ErrorMessage);
		}

		// Webzen_DispatchCommand's callback fires on the SDK's own worker
		// thread; delegates/UObjects must only be touched on the game thread.
		FWebzenLoginResult OnComplete = Context->OnComplete;
		AsyncTask(ENamedThreads::GameThread, [OnComplete, bSuccess, UserId, AccessToken, ErrorMessage]() {
			OnComplete.ExecuteIfBound(bSuccess, UserId, AccessToken, ErrorMessage);
		});
		delete Context;
	}
#endif
}

void UWebzenSDKSubsystem::Initialize(const FString& BaseUrl)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jclass Class = FAndroidApplication::FindJavaClass("com/webzen/sdk/WebzenSDK");
		jmethodID Method = Env->GetStaticMethodID(Class, "initialize", "(Ljava/lang/String;)V");
		jstring JBaseUrl = Env->NewStringUTF(TCHAR_TO_UTF8(*BaseUrl));
		Env->CallStaticVoidMethod(Class, Method, JBaseUrl);
		Env->DeleteLocalRef(JBaseUrl);
		Env->DeleteLocalRef(Class);
	}
#elif PLATFORM_IOS || PLATFORM_WINDOWS
	Webzen_Initialize(TCHAR_TO_UTF8(*BaseUrl));
#endif
}

void UWebzenSDKSubsystem::Login(const FString& ProviderId, const FString& ProviderToken, const FWebzenLoginResult& OnComplete)
{
	const TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
	Request->SetStringField(TEXT("provider_id"), ProviderId);
	Request->SetStringField(TEXT("provider_token"), ProviderToken);
	Request->SetStringField(TEXT("device_id"), TEXT(""));

	FString JsonPayload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
	FJsonSerializer::Serialize(Request, Writer);

	DispatchLogin(JsonPayload, OnComplete);
}

void UWebzenSDKSubsystem::DispatchLogin(const FString& JsonPayload, const FWebzenLoginResult& OnComplete)
{
#if PLATFORM_ANDROID
	// Mirrors WebzenSDK.login's Kotlin fun-interface callback (see
	// platform/android/aar) using Unreal's FJavaClassObject-free raw JNI
	// helpers -- this keeps the Android path in one file instead of adding a
	// second generated Java class just for Unreal.
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jclass Class = FAndroidApplication::FindJavaClass("com/webzen/sdk/WebzenSDK");
		jmethodID Method = Env->GetStaticMethodID(
			Class, "loginWithoutCallbackInterface", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
		// NOTE: com.webzen.sdk.WebzenSDK.login() takes a fun-interface
		// callback that isn't directly constructible from raw JNI without a
		// generated proxy class. Games needing Unreal+Android should add a
		// small companion Java class analogous to Unity's
		// AndroidJavaProxy-based LoginCallbackProxy; left as a follow-up
		// once a real Unreal+Android integration exercises this path.
		(void)Method;
		(void)Class;
		OnComplete.ExecuteIfBound(false, TEXT(""), TEXT(""), TEXT("android_not_yet_wired_for_unreal"));
	}
#elif PLATFORM_IOS || PLATFORM_WINDOWS
	auto* Context = new FLoginCallbackContext{OnComplete};
	Webzen_DispatchCommand("auth.login", TCHAR_TO_UTF8(*JsonPayload), &HandleDispatchResult, Context);
#else
	OnComplete.ExecuteIfBound(false, TEXT(""), TEXT(""), TEXT("platform_not_supported"));
#endif
}
