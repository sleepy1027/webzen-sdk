#pragma once

#include "auth/auth_types.hpp"
#include "core/http_client.hpp"
#include "core/platform_adapter.hpp"

#include <asio/awaitable.hpp>

#include <optional>
#include <string>

namespace webzen::auth {

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
//
// Takes HttpClient/PlatformAdapter explicitly (not read from Sdk::Instance()
// internally) so it stays unit-testable in isolation -- see auth/tests. The
// production singleton (GetAuthService()) wires these to the real Sdk.
// BaseUrl is the one exception: it's read fresh from Sdk::Instance() on
// every call rather than captured at construction, because it's set later,
// by the "core.initialize" command (core/initialize_command.hpp) -- there's
// no guarantee it's known yet when this object is constructed.
class AuthService {
public:
    AuthService(HttpClient& httpClient, adapter::PlatformAdapter platform);

    asio::awaitable<AuthResult> Login(RequestLogin request);
    asio::awaitable<AuthResult> RefreshSession();
    asio::awaitable<void> Logout();

    [[nodiscard]] std::optional<AuthTokenDto> CurrentToken() const;

private:
    void PersistToken(const AuthTokenDto& token);
    [[nodiscard]] std::optional<AuthTokenDto> LoadPersistedToken() const;

    HttpClient& httpClient_;
    adapter::PlatformAdapter platform_;
    std::optional<AuthTokenDto> currentToken_;
};

// Lazily-constructed process-wide instance wired to the real Sdk, used by
// LoginCommand. Tests construct their own AuthService directly instead.
AuthService& GetAuthService();

}  // namespace webzen::auth
