#pragma once

#include "core/request.hpp"

#include <cstdint>
#include <string>

namespace webzen::auth {

struct RequestLogin {
    std::string RequestId;
    std::string ProviderId;     // "guest" | "google" | "apple" | ...
    std::string ProviderToken;
    std::string DeviceId;
};

// Domain/persisted representation of a session token -- used internally by
// AuthService (including what gets written to secure storage). Distinct
// from ResultLogin (the wire shape for "auth.login") on purpose: they
// happen to have the same fields today, but one is "what we remember" and
// the other is "what a command returns", and those are free to diverge
// later (e.g. ResultLogin dropping RefreshToken from the wire response).
struct AuthTokenDto {
    std::string UserId;
    std::string AccessToken;
    std::string RefreshToken;
    std::int64_t ExpiresAt = 0;
};

// "auth.login"'s result -- the default Result plus the token. See
// Command<TRequest, TResult> in core/command_registry.hpp.
struct ResultLogin {
    std::string RequestId;
    bool Success = false;
    std::string ErrorCode;
    std::string ErrorMessage;
    std::string UserId;
    std::string AccessToken;
    std::string RefreshToken;
    std::int64_t ExpiresAt = 0;
};

}  // namespace webzen::auth

WEBZEN_JSON(webzen::auth::RequestLogin);
WEBZEN_JSON(webzen::auth::AuthTokenDto);
WEBZEN_JSON(webzen::auth::ResultLogin);
