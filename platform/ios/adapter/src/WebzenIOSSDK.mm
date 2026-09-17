#import "WebzenIOSSDK.h"

#include "webzen/sdk_c_api.h"

#import <Foundation/Foundation.h>

namespace {

// Bridges Webzen_DispatchCommand's C callback (function pointer + void*)
// back into an Objective-C block. userData owns one heap-allocated copy of
// the block for the lifetime of a single dispatch.
using LoginCompletion = void (^)(BOOL success, NSString* _Nullable userId, NSString* _Nullable accessToken, NSString* _Nullable errorMessage);

void HandleLoginResult(int success, const char* jsonPayload, void* userData) {
    LoginCompletion completion = (__bridge_transfer LoginCompletion)userData;
    NSData* data = [NSData dataWithBytes:jsonPayload length:strlen(jsonPayload)];
    NSDictionary* json = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];

    if (!success) {
        completion(NO, nil, nil, json[@"error"] ?: @"unknown_error");
        return;
    }
    completion(YES, json[@"user_id"], json[@"access_token"], nil);
}

}  // namespace

@implementation WebzenIOSSDK

+ (void)initializeWithBaseUrl:(NSString *)baseUrl {
    Webzen_Initialize(baseUrl.UTF8String);
}

+ (void)shutdown {
    Webzen_Shutdown();
}

+ (void)loginWithProviderId:(NSString *)providerId
              providerToken:(NSString *)providerToken
                 completion:(void (^)(BOOL, NSString * _Nullable, NSString * _Nullable, NSString * _Nullable))completion {
    NSDictionary* request = @{
        @"provider_id" : providerId,
        @"provider_token" : providerToken,
        @"device_id" : @"",
    };
    NSData* body = [NSJSONSerialization dataWithJSONObject:request options:0 error:nil];
    NSString* json = [[NSString alloc] initWithData:body encoding:NSUTF8StringEncoding];

    // +1 retain via __bridge_retained so the block survives until
    // HandleLoginResult runs (Webzen_DispatchCommand's callback fires on the
    // SDK's own worker thread, not necessarily before this method returns).
    void* userData = (__bridge_retained void*)[completion copy];
    Webzen_DispatchCommand("auth.login", json.UTF8String, &HandleLoginResult, userData);
}

@end
