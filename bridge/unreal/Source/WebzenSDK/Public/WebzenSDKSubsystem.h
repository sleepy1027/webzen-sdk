#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebzenSDKSubsystem.generated.h"

// USTRUCTs don't support virtual dispatch the way ordinary C++ classes do,
// so FRequestLogin/FRequestInitialize below only inherit FWebzenRequest's
// data; each typed Dispatch wrapper (Login, Initialize) passes its own
// command id explicitly rather than through a virtual method.
//
// Deliberately has no fields: a RequestId generated here as a UPROPERTY
// default would risk getting baked into a Blueprint's stored default value
// at edit time instead of regenerated per call. UWebzenSDKSubsystem::Dispatch
// generates a fresh one for every call instead -- see the .cpp.
USTRUCT(BlueprintType)
struct FWebzenRequest
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FRequestInitialize : public FWebzenRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Webzen SDK")
	FString BaseUrl;
};

USTRUCT(BlueprintType)
struct FRequestLogin : public FWebzenRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Webzen SDK")
	FString ProviderId;

	UPROPERTY(BlueprintReadWrite, Category = "Webzen SDK")
	FString ProviderToken;

	UPROPERTY(BlueprintReadWrite, Category = "Webzen SDK")
	FString DeviceId;
};

// The one shape every Dispatch callback receives, whatever the command
// (core/request.hpp's webzen::Result, mirrored field for field). Parse Data
// yourself once you know which request you sent.
USTRUCT(BlueprintType)
struct FWebzenResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	FString RequestId;

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	FString ErrorCode;

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	FString Data;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWebzenResultDelegate, const FWebzenResult&, Result);

/**
 * Blueprint- and C++-facing entry point, one per game instance. Calls
 * webzen/core/sdk_c_api.h's Webzen_DispatchCommand directly on every
 * platform -- Android, iOS and Windows all export the identical C ABI, so
 * there's no per-OS bridge class here, just one difference in how the
 * symbol is resolved (see ResolveDispatchCommand in the .cpp): iOS/Windows
 * link the native artifact at build time (WebzenSDK.Build.cs), while
 * Android's libwebzen_core.so lives inside the AAR UBT embeds, so it's
 * resolved with dlopen/dlsym once at runtime instead.
 */
UCLASS()
class WEBZENSDK_API UWebzenSDKSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Generic entry point every typed wrapper below is built on.
	void Dispatch(const FString& CommandId, const FString& RequestJson, const FWebzenResultDelegate& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "Webzen SDK")
	void Initialize(const FRequestInitialize& Request, const FWebzenResultDelegate& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "Webzen SDK")
	void Login(const FRequestLogin& Request, const FWebzenResultDelegate& OnComplete);

	// Exposed so the free function in WebzenSDKSubsystem.cpp that receives
	// the native callback (which can't be a UObject member -- it has to be
	// a plain C function pointer) can look a RequestId back up to the
	// delegate Dispatch stored for it.
	static TMap<FString, FWebzenResultDelegate>& Pending();
	static FCriticalSection& PendingLock();
};
