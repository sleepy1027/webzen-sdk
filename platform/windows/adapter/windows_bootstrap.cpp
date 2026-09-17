// Installs the Windows platform adapter before any exported function runs.
// A global's constructor is used (CRT-initialized when webzen_core.dll is
// loaded, before any exported entry point can be called) since Windows has
// no JNI_OnLoad/+load equivalent, and since -- unlike iOS -- we build this
// DLL ourselves (see the CMakeLists.txt comment), there's no risk of a
// downstream linker dropping this translation unit as unreferenced.

#include "core/sdk.hpp"
#include "platform/windows/adapter/windows_adapter.hpp"

namespace webzen::platform::windows {
namespace {

WindowsSecureStorage g_secureStorage;
WindowsDeviceInfo g_deviceInfo;

struct Bootstrap {
    Bootstrap() {
        Sdk::Instance().SetPlatformAdapter(
            adapter::PlatformAdapter{.SecureStorage = &g_secureStorage, .DeviceInfo = &g_deviceInfo});
    }
} g_bootstrap;

}  // namespace
}  // namespace webzen::platform::windows
