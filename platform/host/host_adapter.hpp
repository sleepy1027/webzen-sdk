#pragma once

#include "core/platform_adapter.hpp"

#include <unordered_map>

namespace webzen::platform::host {

// In-memory stand-in for the OS keychain, used by dev builds, unit tests
// and CI. Never linked into an Android/iOS/Windows artifact.
class HostSecureStorage : public adapter::ISecureStorage {
public:
    bool Set(std::string_view key, std::string_view value) override;
    std::optional<std::string> Get(std::string_view key) override;
    void Remove(std::string_view key) override;

private:
    std::unordered_map<std::string, std::string> values_;
};

class HostDeviceInfo : public adapter::IDeviceInfo {
public:
    [[nodiscard]] std::string DeviceId() const override;
    [[nodiscard]] std::string OsVersion() const override;
    [[nodiscard]] std::string AppVersion() const override;
};

adapter::PlatformAdapter MakeHostPlatformAdapter(HostSecureStorage& storage, HostDeviceInfo& deviceInfo);

}  // namespace webzen::platform::host
