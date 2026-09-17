// Smoke test for the whole pipeline: platform adapter -> Sdk::Initialize ->
// command dispatch through the C ABI -> AuthService -> HttpClient. Run after
// every core change (`build/host/bin/webzen_host_demo`) as a cheap sanity
// check that the toolchain still produces a working artifact end to end.

#include "webzen/adapter/host_adapter.hpp"
#include "webzen/sdk.hpp"
#include "webzen/sdk_c_api.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

int main() {
    webzen::platform::host::HostSecureStorage storage;
    webzen::platform::host::HostDeviceInfo deviceInfo;
    webzen::sdk::Sdk::Instance().SetPlatformAdapter(webzen::platform::host::MakeHostPlatformAdapter(storage, deviceInfo));

    Webzen_Initialize("http://127.0.0.1:1");  // nothing listens here on purpose

    std::atomic<bool> done{false};
    Webzen_DispatchCommand(
        "auth.login",
        R"({"provider_id":"guest","provider_token":"demo","device_id":""})",
        [](int success, const char* json_payload, void* user_data) {
            std::printf("auth.login -> success=%d payload=%s\n", success, json_payload);
            static_cast<std::atomic<bool>*>(user_data)->store(true);
        },
        &done);

    while (!done.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    Webzen_Shutdown();
    return 0;
}
