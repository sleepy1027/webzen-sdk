#pragma once

#include "webzen/adapter/platform_adapter.hpp"
#include "webzen/auth/auth_types.hpp"
#include "webzen/net/http_client.hpp"

#include <asio/awaitable.hpp>

#include <optional>
#include <string>

namespace webzen::auth {

struct AuthConfig {
    std::string BaseUrl;
};

enum class AuthError {
    None,
    InvalidCredentials,
    NetworkError,
    NotAuthenticated,
};

struct AuthResult {
    bool Success = false;
    AuthError Error = AuthError::None;
    AuthTokenDto Token;
};

// Reference implementation of the "migrate one feature at a time" model
// described in the architecture skill: this is the first (and so far only)
// domain moved out of the per-OS libraries into the common core. Billing,
// WebView, Push, Crash and MMP still live in the legacy Android/iOS/Windows
// libraries until they get the same treatment.
class AuthService {
public:
    AuthService(AuthConfig config, net::HttpClient& httpClient, adapter::PlatformAdapter platform);

    asio::awaitable<AuthResult> Login(LoginRequestDto request);
    asio::awaitable<AuthResult> RefreshSession();
    asio::awaitable<void> Logout();

    [[nodiscard]] std::optional<AuthTokenDto> CurrentToken() const;

private:
    void PersistToken(const AuthTokenDto& token);
    [[nodiscard]] std::optional<AuthTokenDto> LoadPersistedToken() const;

    AuthConfig config_;
    net::HttpClient& httpClient_;
    adapter::PlatformAdapter platform_;
    std::optional<AuthTokenDto> currentToken_;
};

}  // namespace webzen::auth
