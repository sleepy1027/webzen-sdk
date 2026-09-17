#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// Public Objective-C entry point shipped in the .xcframework's headers.
/// Unity's iOS plugin and Unreal's iOS wrapper both call this instead of
/// webzen/sdk_c_api.h directly -- it exists so callers get NSString/blocks
/// instead of raw C strings and a C callback pointer.
@interface WebzenIOSSDK : NSObject

+ (void)initializeWithBaseUrl:(NSString *)baseUrl;
+ (void)shutdown;

+ (void)loginWithProviderId:(NSString *)providerId
              providerToken:(NSString *)providerToken
                 completion:(void (^)(BOOL success, NSString *_Nullable userId, NSString *_Nullable accessToken, NSString *_Nullable errorMessage))completion;

@end

NS_ASSUME_NONNULL_END
