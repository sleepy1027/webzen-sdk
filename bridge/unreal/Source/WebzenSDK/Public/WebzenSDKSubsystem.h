#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebzenSDKSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FWebzenLoginResult, bool, bSuccess, const FString&, UserId, const FString&, AccessToken, const FString&, ErrorMessage);

/**
 * Blueprint- and C++-facing entry point, one per game instance. Platform
 * dispatch happens entirely in the .cpp (PLATFORM_ANDROID / PLATFORM_IOS /
 * PLATFORM_WINDOWS): Android goes through JNI to platform/android's Kotlin
 * WebzenSDK facade, iOS/Windows call webzen/sdk_c_api.h directly since
 * WebzenSDK.Build.cs links their native artifact straight into the game.
 */
UCLASS()
class WEBZENSDK_API UWebzenSDKSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Webzen SDK")
	void Initialize(const FString& BaseUrl);

	UFUNCTION(BlueprintCallable, Category = "Webzen SDK")
	void Login(const FString& ProviderId, const FString& ProviderToken, const FWebzenLoginResult& OnComplete);

private:
	void DispatchLogin(const FString& JsonPayload, const FWebzenLoginResult& OnComplete);
};
