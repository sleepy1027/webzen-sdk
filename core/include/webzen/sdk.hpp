#pragma once

#include "webzen/adapter/platform_adapter.hpp"
#include "webzen/auth/auth_service.hpp"
#include "webzen/net/http_client.hpp"

#include <asio/io_context.hpp>
#include <asio/executor_work_guard.hpp>

#include <functional>
#include <optional>
#include <string>
#include <thread>

namespace webzen::sdk {

struct SdkConfig {
    std::string BaseUrl;
};

// Process-wide facade: owns the io_context that every coroutine in the core
// runs on, and the one instance of each migrated domain service (AuthService
// today; Billing/WebView/Push/Crash/MMP join this list as they migrate out of
// the per-OS libraries). Platform bootstrap code sets the adapter before
// Initialize() is called; see platform/*/adapter.
class Sdk {
public:
    static Sdk& Instance();

    Sdk(const Sdk&) = delete;
    Sdk& operator=(const Sdk&) = delete;

    void SetPlatformAdapter(adapter::PlatformAdapter platform);
    void Initialize(SdkConfig config);
    void Shutdown();

    using DispatchCallback = std::function<void(bool success, std::string jsonPayload)>;
    // Looks `commandId` up in command::CommandRegistry and runs it on the
    // SDK's own io_context thread; `callback` fires back on that same
    // thread, so bridges (JNI/ObjC/C#) must hop back to their own main/UI
    // thread themselves before touching engine APIs.
    void DispatchCommand(std::string commandId, std::string jsonPayload, DispatchCallback callback);

    [[nodiscard]] auth::AuthService& Auth();

private:
    Sdk() = default;

    adapter::PlatformAdapter platform_;
    asio::io_context ioContext_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> workGuard_;
    std::thread ioThread_;
    std::optional<net::HttpClient> httpClient_;
    std::optional<auth::AuthService> authService_;
    bool initialized_ = false;
};

}  // namespace webzen::sdk
