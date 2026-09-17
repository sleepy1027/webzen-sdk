#include "webzen/sdk.hpp"

#include "webzen/command/command_registry.hpp"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>

namespace webzen::sdk {

Sdk& Sdk::Instance() {
    static Sdk instance;
    return instance;
}

void Sdk::SetPlatformAdapter(adapter::PlatformAdapter platform) {
    platform_ = platform;
}

void Sdk::Initialize(SdkConfig config) {
    if (initialized_) {
        return;
    }

    workGuard_.emplace(asio::make_work_guard(ioContext_));
    ioThread_ = std::thread([this] { ioContext_.run(); });

    httpClient_.emplace(ioContext_.get_executor());
    authService_.emplace(auth::AuthConfig{.BaseUrl = config.BaseUrl}, *httpClient_, platform_);

    initialized_ = true;
}

void Sdk::Shutdown() {
    if (!initialized_) {
        return;
    }

    workGuard_.reset();
    ioContext_.stop();
    if (ioThread_.joinable()) {
        ioThread_.join();
    }
    ioContext_.restart();

    authService_.reset();
    httpClient_.reset();
    initialized_ = false;
}

void Sdk::DispatchCommand(std::string commandId, std::string jsonPayload, DispatchCallback callback) {
    asio::co_spawn(
        ioContext_,
        command::CommandRegistry::Instance().Dispatch(std::move(commandId), command::CommandContext{.Payload = std::move(jsonPayload)}),
        [callback = std::move(callback)](std::exception_ptr, command::CommandResult result) {
            callback(result.Success, std::move(result.Payload));
        });
}

auth::AuthService& Sdk::Auth() {
    return *authService_;
}

}  // namespace webzen::sdk
