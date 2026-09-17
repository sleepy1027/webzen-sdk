// Installs the iOS platform adapter before any command can run. A global's
// constructor is used here (rather than Android's JNI_OnLoad, which iOS has
// no equivalent of) -- it runs when the app process loads this translation
// unit's object file, which is guaranteed to happen exactly when the rest
// of this static library is force-loaded (see the CMakeLists.txt comment on
// -force_load).

#include "core/sdk.hpp"
#include "platform/ios/adapter/ios_adapter.hpp"

namespace webzen::platform::ios {
namespace {

IOSSecureStorage g_secureStorage;
IOSDeviceInfo g_deviceInfo;

struct Bootstrap {
    Bootstrap() {
        Sdk::Instance().SetPlatformAdapter(
            adapter::PlatformAdapter{.SecureStorage = &g_secureStorage, .DeviceInfo = &g_deviceInfo});
    }
} g_bootstrap;

}  // namespace
}  // namespace webzen::platform::ios
