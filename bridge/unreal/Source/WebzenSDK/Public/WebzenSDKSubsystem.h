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

// The four fields every command's result starts with (core/request.hpp's
// webzen::Result). A command with nothing more to report uses this
// directly (Initialize does); one that returns more has its own result
// struct with these same four fields plus its own -- see FResultLogin.
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
};

// "auth.login"'s result -- the wire JSON has UserId/AccessToken/... as
// top-level fields alongside RequestId/Success/..., not nested inside a
// generic blob, so this mirrors core::auth::ResultLogin field for field.
USTRUCT(BlueprintType)
struct FResultLogin
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
	FString UserId;

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	FString AccessToken;

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	FString RefreshToken;

	UPROPERTY(BlueprintReadOnly, Category = "Webzen SDK")
	int64 ExpiresAt = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWebzenResultDelegate, const FWebzenResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWebzenLoginResultDelegate, const FResultLogin&, Result);

/**
 * Blueprint- and C++-facing entry point, one per game instance. Calls
 * webzen/core/sdk_c_api.h's Webzen_DispatchCommand directly on every
 * platform -- Android, iOS and Windows all export the identical C ABI, so
 * there's no per-OS bridge class here, just one difference in how the
 * symbol is resolved (see ResolveDispatchCommand in the .cpp): iOS/Windows
 * link the native artifact at build time (WebzenSDK.Build.cs), while
 * Android's libwebzen_core.so lives inside the AAR UBT embeds, so it's
 * resolved with dlopen/dlsym once at runtime instead.
 *
 * Blueprint dynamic delegates need a fixed signature, so unlike the Unity
 * bridge's single generic Dispatch<TResult>(), each command gets its own
 * UFUNCTION with its own result struct/delegate type (Login/FResultLogin/
 * FWebzenLoginResultDelegate vs. Initialize/FWebzenResult/
 * FWebzenResultDelegate) -- both are thin wrappers over the same private
 * DispatchRaw.
 */
UCLASS()
class WEBZENSDK_API UWebzenSDKSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Webzen SDK")
	void Initialize(const FRequestInitialize& Request, const FWebzenResultDelegate& OnComplete);

	UFUNCTION(BlueprintCallable, Category = "Webzen SDK")
	void Login(const FRequestLogin& Request, const FWebzenLoginResultDelegate& OnComplete);

	// Exposed so the free function in WebzenSDKSubsystem.cpp that receives
	// the native callback (which can't be a UObject member -- it has to be
	// a plain C function pointer) can look a RequestId back up to the
	// handler DispatchRaw stored for it. The handler takes the raw result
	// JSON rather than a fixed struct because different pending calls may
	// expect different result types (FWebzenResult vs. FResultLogin, ...).
	static TMap<FString, TFunction<void(const FString&)>>& Pending();
	static FCriticalSection& PendingLock();

private:
	// Generic entry point every typed wrapper above is built on. OnRawResult
	// receives the full result JSON string; the wrapper deserializes it into
	// whatever struct its own delegate expects.
	void DispatchRaw(const FString& CommandId, const FString& RequestJson, TFunction<void(const FString&)> OnRawResult);
};
