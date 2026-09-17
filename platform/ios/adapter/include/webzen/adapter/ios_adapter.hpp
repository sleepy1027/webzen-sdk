#pragma once

#include "webzen/adapter/platform_adapter.hpp"

namespace webzen::platform::ios {

// Keychain-backed (Security.framework), see ios_adapter.mm. Objective-C++
// because SecItem* and the Keychain access-control APIs are Apple-only.
class IOSSecureStorage : public adapter::ISecureStorage {
public:
    bool Set(std::string_view key, std::string_view value) override;
    std::optional<std::string> Get(std::string_view key) override;
    void Remove(std::string_view key) override;
};

class IOSDeviceInfo : public adapter::IDeviceInfo {
public:
    [[nodiscard]] std::string DeviceId() const override;
    [[nodiscard]] std::string OsVersion() const override;
    [[nodiscard]] std::string AppVersion() const override;
};

}  // namespace webzen::platform::ios
