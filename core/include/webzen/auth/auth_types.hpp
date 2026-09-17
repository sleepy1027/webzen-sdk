#pragma once

#include "webzen/json/json_naming.hpp"

#include <glaze/glaze.hpp>

#include <cstdint>
#include <string>

namespace webzen::auth {

struct LoginRequestDto {
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

template <>
struct glz::meta<webzen::auth::LoginRequestDto> {
    using T = webzen::auth::LoginRequestDto;
    static constexpr auto value = glz::object(
        SDK_FIELD(T, ProviderId),
        SDK_FIELD(T, ProviderToken),
        SDK_FIELD(T, DeviceId)
    );
};

template <>
struct glz::meta<webzen::auth::AuthTokenDto> {
    using T = webzen::auth::AuthTokenDto;
    static constexpr auto value = glz::object(
        SDK_FIELD(T, UserId),
        SDK_FIELD(T, AccessToken),
        SDK_FIELD(T, RefreshToken),
        SDK_FIELD(T, ExpiresAt)
    );
};
