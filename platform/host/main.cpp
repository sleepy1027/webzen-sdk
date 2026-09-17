// Smoke test for the whole pipeline: platform adapter -> DispatchCommand
// ("core.initialize" then "auth.login") -> CommandRegistry -> AuthService ->
// HttpClient. Run after every core/auth change
// (`build/host/bin/webzen_host_demo`) as a cheap sanity check that the
// toolchain still produces a working artifact end to end.

#include "core/sdk.hpp"
#include "core/sdk_c_api.h"
#include "platform/host/host_adapter.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

namespace {

std::atomic<bool> g_initDone{false};
std::atomic<bool> g_loginDone{false};

void OnInitializeResult(const char* resultJson) {
    std::printf("core.initialize -> %s\n", resultJson);
    g_initDone.store(true);
}

void OnLoginResult(const char* resultJson) {
    std::printf("auth.login -> %s\n", resultJson);
    g_loginDone.store(true);
}

}  // namespace

int main() {
    webzen::platform::host::HostSecureStorage storage;
    webzen::platform::host::HostDeviceInfo deviceInfo;
    webzen::Sdk::Instance().SetPlatformAdapter(webzen::platform::host::MakeHostPlatformAdapter(storage, deviceInfo));

    // BaseUrl deliberately points at a port nothing listens on, so
    // auth.login below is expected to fail gracefully (login_failed), not
    // hang, crash, or come back unknown_command.
    Webzen_DispatchCommand("core.initialize", R"({"request_id":"init-1","base_url":"http://127.0.0.1:1"})", &OnInitializeResult);
    while (!g_initDone.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    Webzen_DispatchCommand(
        "auth.login",
        R"({"request_id":"login-1","provider_id":"guest","provider_token":"demo","device_id":""})",
        &OnLoginResult);
    while (!g_loginDone.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    Webzen_Shutdown();
    return 0;
}
