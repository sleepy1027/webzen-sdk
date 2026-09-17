#include "core/sdk.hpp"

#include "core/command_registry.hpp"

#include <asio/co_spawn.hpp>

namespace webzen {

Sdk& Sdk::Instance() {
    static Sdk instance;
    return instance;
}

void Sdk::SetPlatformAdapter(adapter::PlatformAdapter platform) {
    platform_ = platform;
}

adapter::PlatformAdapter& Sdk::Platform() {
    return platform_;
}

void Sdk::EnsureStarted() {
    std::lock_guard lock(lifecycleMutex_);
    if (started_) {
        return;
    }
    workGuard_.emplace(asio::make_work_guard(ioContext_));
    ioThread_ = std::thread([this] { ioContext_.run(); });
    httpClient_.emplace(ioContext_.get_executor());
    started_ = true;
}

HttpClient& Sdk::Http() {
    EnsureStarted();
    return *httpClient_;
}

std::string Sdk::BaseUrl() const {
    std::lock_guard lock(baseUrlMutex_);
    return baseUrl_;
}

void Sdk::SetBaseUrl(std::string baseUrl) {
    std::lock_guard lock(baseUrlMutex_);
    baseUrl_ = std::move(baseUrl);
}

void Sdk::DispatchCommand(std::string commandId, std::string requestJson, DispatchCallback callback) {
    EnsureStarted();
    asio::co_spawn(
        ioContext_,
        CommandRegistry::Instance().Dispatch(std::move(commandId), std::move(requestJson)),
        [callback = std::move(callback)](std::exception_ptr, std::string resultJson) {
            callback(std::move(resultJson));
        });
}

void Sdk::Shutdown() {
    std::lock_guard lock(lifecycleMutex_);
    if (!started_) {
        return;
    }
    workGuard_.reset();
    ioContext_.stop();
    if (ioThread_.joinable()) {
        ioThread_.join();
    }
    httpClient_.reset();
    ioContext_.restart();  // so a later EnsureStarted() can run() it again
    started_ = false;
}

}  // namespace webzen
