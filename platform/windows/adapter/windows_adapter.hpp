#pragma once

#include "core/platform_adapter.hpp"

namespace webzen::platform::windows {

// DPAPI-backed (CryptProtectData/CryptUnprotectData), scoped to the current
// Windows user account -- the closest per-user-secret equivalent Windows has
// to Android Keystore / iOS Keychain. Values are stored as one encrypted
// file per key under %LOCALAPPDATA%\WebzenSDK.
class WindowsSecureStorage : public adapter::ISecureStorage {
public:
    bool Set(std::string_view key, std::string_view value) override;
    std::optional<std::string> Get(std::string_view key) override;
    void Remove(std::string_view key) override;
};

class WindowsDeviceInfo : public adapter::IDeviceInfo {
public:
    [[nodiscard]] std::string DeviceId() const override;
    [[nodiscard]] std::string OsVersion() const override;
    [[nodiscard]] std::string AppVersion() const override;
};

}  // namespace webzen::platform::windows
