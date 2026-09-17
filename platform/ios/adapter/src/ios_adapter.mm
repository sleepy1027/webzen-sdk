#include "webzen/adapter/ios_adapter.hpp"

#import <Foundation/Foundation.h>
#import <Security/Security.h>
#import <UIKit/UIKit.h>

namespace webzen::platform::ios {

namespace {
NSString* const kServiceName = @"com.webzen.sdk";

NSString* ToNSString(std::string_view value) {
    return [[NSString alloc] initWithBytes:value.data() length:value.size() encoding:NSUTF8StringEncoding];
}
}  // namespace

bool IOSSecureStorage::Set(std::string_view key, std::string_view value) {
    NSString* nsKey = ToNSString(key);
    NSData* data = [ToNSString(value) dataUsingEncoding:NSUTF8StringEncoding];

    NSDictionary* query = @{
        (__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService : kServiceName,
        (__bridge id)kSecAttrAccount : nsKey,
    };
    SecItemDelete((__bridge CFDictionaryRef)query);

    NSMutableDictionary* addQuery = [query mutableCopy];
    addQuery[(__bridge id)kSecValueData] = data;
    addQuery[(__bridge id)kSecAttrAccessible] = (__bridge id)kSecAttrAccessibleAfterFirstUnlock;

    return SecItemAdd((__bridge CFDictionaryRef)addQuery, nullptr) == errSecSuccess;
}

std::optional<std::string> IOSSecureStorage::Get(std::string_view key) {
    NSDictionary* query = @{
        (__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService : kServiceName,
        (__bridge id)kSecAttrAccount : ToNSString(key),
        (__bridge id)kSecReturnData : @YES,
        (__bridge id)kSecMatchLimit : (__bridge id)kSecMatchLimitOne,
    };

    CFTypeRef result = nullptr;
    if (SecItemCopyMatching((__bridge CFDictionaryRef)query, &result) != errSecSuccess) {
        return std::nullopt;
    }

    NSData* data = (__bridge_transfer NSData*)result;
    NSString* value = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    return std::string([value UTF8String]);
}

void IOSSecureStorage::Remove(std::string_view key) {
    NSDictionary* query = @{
        (__bridge id)kSecClass : (__bridge id)kSecClassGenericPassword,
        (__bridge id)kSecAttrService : kServiceName,
        (__bridge id)kSecAttrAccount : ToNSString(key),
    };
    SecItemDelete((__bridge CFDictionaryRef)query);
}

std::string IOSDeviceInfo::DeviceId() const {
    NSString* vendorId = [[UIDevice currentDevice].identifierForVendor UUIDString];
    return vendorId ? std::string([vendorId UTF8String]) : "unknown";
}

std::string IOSDeviceInfo::OsVersion() const {
    return std::string([[NSString stringWithFormat:@"iOS %@", [UIDevice currentDevice].systemVersion] UTF8String]);
}

std::string IOSDeviceInfo::AppVersion() const {
    NSString* version = [[NSBundle mainBundle].infoDictionary objectForKey:@"CFBundleShortVersionString"];
    return version ? std::string([version UTF8String]) : "unknown";
}

}  // namespace webzen::platform::ios
