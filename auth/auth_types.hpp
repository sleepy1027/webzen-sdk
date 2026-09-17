#pragma once

#include <glaze/glaze.hpp>

#include <cstdint>
#include <string>

namespace webzen::auth {

struct RequestLogin {
    std::string RequestId;
    std::string ProviderId;     // "guest" | "google" | "apple" | ...
    std::string ProviderToken;
    std::string DeviceId;
};

struct AuthTokenDto {
    std::string UserId;
    std::string AccessToken;
    std::string RefreshToken;
    std::int64_t ExpiresAt = 0;
};

}  // namespace webzen::auth

// glz::snake_case: see core/request.hpp -- every DTO in this codebase uses
// this instead of hand-listing fields, since these are plain structs with
// no base class.
template <>
struct glz::meta<webzen::auth::RequestLogin> : glz::snake_case {};

template <>
struct glz::meta<webzen::auth::AuthTokenDto> : glz::snake_case {};
