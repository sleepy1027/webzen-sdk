#include "platform/host/host_adapter.hpp"

namespace webzen::platform::host {

bool HostSecureStorage::Set(std::string_view key, std::string_view value) {
    values_[std::string(key)] = std::string(value);
    return true;
}

std::optional<std::string> HostSecureStorage::Get(std::string_view key) {
    const auto it = values_.find(std::string(key));
    if (it == values_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void HostSecureStorage::Remove(std::string_view key) {
    values_.erase(std::string(key));
}

std::string HostDeviceInfo::DeviceId() const { return "host-dev-device"; }
std::string HostDeviceInfo::OsVersion() const { return "host"; }
std::string HostDeviceInfo::AppVersion() const { return "0.0.0-dev"; }

adapter::PlatformAdapter MakeHostPlatformAdapter(HostSecureStorage& storage, HostDeviceInfo& deviceInfo) {
    return adapter::PlatformAdapter{.SecureStorage = &storage, .DeviceInfo = &deviceInfo};
}

}  // namespace webzen::platform::host
