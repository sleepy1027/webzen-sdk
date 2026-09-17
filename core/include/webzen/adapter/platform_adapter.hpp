#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace webzen::adapter {

// Persistent key/value storage backed by whatever the OS considers "secure":
// Android Keystore-backed EncryptedSharedPreferences, iOS Keychain, Windows
// DPAPI/Credential Manager. This has to be an adapter because the storage
// backend is the one part of session persistence that is genuinely different
// per OS; the retry/refresh logic built on top of it lives in the core.
class ISecureStorage {
public:
    virtual ~ISecureStorage() = default;
    virtual bool Set(std::string_view key, std::string_view value) = 0;
    virtual std::optional<std::string> Get(std::string_view key) = 0;
    virtual void Remove(std::string_view key) = 0;
};

// Device/app identifiers the auth backend expects on every request. Values
// come from OS-specific APIs (Settings.Secure, identifierForVendor, WMI/registry).
class IDeviceInfo {
public:
    virtual ~IDeviceInfo() = default;
    [[nodiscard]] virtual std::string DeviceId() const = 0;
    [[nodiscard]] virtual std::string OsVersion() const = 0;
    [[nodiscard]] virtual std::string AppVersion() const = 0;
};

// Non-owning bundle of adapter implementations. Each platform bootstrap path
// (JNI_OnLoad on Android, an Objective-C++ init call on iOS, an explicit
// bootstrap call on Windows) constructs the concrete adapters and keeps them
// alive for the process lifetime; the core only ever borrows them.
//
// Add a new member here only when a feature being migrated into the core
// actually needs it -- see "판단 순서" step 2 in the architecture skill.
struct PlatformAdapter {
    ISecureStorage* SecureStorage = nullptr;
    IDeviceInfo* DeviceInfo = nullptr;
};

}  // namespace webzen::adapter
