#pragma once

#include "core/http_client.hpp"
#include "core/platform_adapter.hpp"

#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>

#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace webzen {

// Process-wide facade. Owns only generic plumbing -- the io_context every
// coroutine in the core runs on, the one HttpClient, the platform adapter,
// and the current BaseUrl (set via the "core.initialize" command, see
// initialize_command.hpp). It deliberately does NOT own any domain service
// (AuthService, etc.): each domain manages its own lazily-constructed
// singleton internally, reading Http()/Platform()/BaseUrl() from here, so
// adding a new feature module never means touching this class.
//
// There is no separate Initialize() entry point -- DispatchCommand is the
// only way in, per the architecture skill, so the io_context starts lazily
// on first use instead of on some earlier lifecycle call.
class Sdk {
public:
    static Sdk& Instance();

    Sdk(const Sdk&) = delete;
    Sdk& operator=(const Sdk&) = delete;

    void SetPlatformAdapter(adapter::PlatformAdapter platform);
    [[nodiscard]] adapter::PlatformAdapter& Platform();

    [[nodiscard]] HttpClient& Http();

    [[nodiscard]] std::string BaseUrl() const;
    void SetBaseUrl(std::string baseUrl);

    using DispatchCallback = std::function<void(std::string resultJson)>;
    // Looks `commandId` up in CommandRegistry and runs it on the SDK's own
    // io_context thread (started on first call); `callback` fires back on
    // that same thread, so bridges (JNI/native/P-Invoke) must hop back to
    // their own main/UI thread themselves before touching engine APIs.
    void DispatchCommand(std::string commandId, std::string requestJson, DispatchCallback callback);

    // Plumbing, not a business command: stops the io thread. Exposed
    // separately from DispatchCommand because a command can't join its own
    // io_context's thread from within a coroutine running on it.
    void Shutdown();

private:
    Sdk() = default;
    void EnsureStarted();

    adapter::PlatformAdapter platform_;

    asio::io_context ioContext_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> workGuard_;
    std::thread ioThread_;
    std::optional<HttpClient> httpClient_;
    std::mutex lifecycleMutex_;
    bool started_ = false;

    mutable std::mutex baseUrlMutex_;
    std::string baseUrl_;
};

}  // namespace webzen
