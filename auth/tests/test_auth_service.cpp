#include "auth/auth_service.hpp"
#include "core/sdk.hpp"

#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/use_future.hpp>

#include <catch2/catch_test_macros.hpp>

#include <unordered_map>

namespace {

class InMemorySecureStorage : public webzen::adapter::ISecureStorage {
public:
    bool Set(std::string_view key, std::string_view value) override {
        values_[std::string(key)] = std::string(value);
        return true;
    }
    std::optional<std::string> Get(std::string_view key) override {
        const auto it = values_.find(std::string(key));
        if (it == values_.end()) return std::nullopt;
        return it->second;
    }
    void Remove(std::string_view key) override { values_.erase(std::string(key)); }

private:
    std::unordered_map<std::string, std::string> values_;
};

class FakeDeviceInfo : public webzen::adapter::IDeviceInfo {
public:
    [[nodiscard]] std::string DeviceId() const override { return "test-device"; }
    [[nodiscard]] std::string OsVersion() const override { return "test-os"; }
    [[nodiscard]] std::string AppVersion() const override { return "0.0.0"; }
};

}  // namespace

TEST_CASE("AuthService.Login surfaces a network error instead of hanging or throwing", "[auth_service]") {
    // Nothing listens on this loopback port, so the request must fail fast
    // with a network error rather than a valid HTTP response. Read fresh by
    // AuthService on every call (see auth/auth_service.hpp), not captured at
    // construction, so it has to be set on the real Sdk singleton here too.
    webzen::Sdk::Instance().SetBaseUrl("http://127.0.0.1:1");

    asio::io_context io;
    webzen::HttpClient httpClient(io.get_executor(), webzen::RetryPolicy{.MaxAttempts = 1});

    InMemorySecureStorage storage;
    FakeDeviceInfo deviceInfo;
    webzen::adapter::PlatformAdapter platform{.SecureStorage = &storage, .DeviceInfo = &deviceInfo};

    webzen::auth::AuthService authService(httpClient, platform);

    auto future = asio::co_spawn(
        io, authService.Login(webzen::auth::RequestLogin{{}, "guest", "token", ""}), asio::use_future);
    io.run();

    const auto result = future.get();
    CHECK_FALSE(result.Success);
    CHECK(result.Error == webzen::auth::AuthError::NetworkError);
}

TEST_CASE("AuthService persists and clears the session token via the platform adapter", "[auth_service]") {
    asio::io_context io;
    webzen::HttpClient httpClient(io.get_executor());

    InMemorySecureStorage storage;
    FakeDeviceInfo deviceInfo;
    webzen::adapter::PlatformAdapter platform{.SecureStorage = &storage, .DeviceInfo = &deviceInfo};

    storage.Set("webzen.auth.token", R"({"user_id":"u1","access_token":"a","refresh_token":"r","expires_at":123})");

    webzen::auth::AuthService authService(httpClient, platform);
    REQUIRE(authService.CurrentToken().has_value());
    CHECK(authService.CurrentToken()->UserId == "u1");

    auto future = asio::co_spawn(io, authService.Logout(), asio::use_future);
    io.run();
    future.get();

    CHECK_FALSE(authService.CurrentToken().has_value());
    CHECK_FALSE(storage.Get("webzen.auth.token").has_value());
}
