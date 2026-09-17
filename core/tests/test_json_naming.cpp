#include "core/json_naming.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("to_snake_case converts PascalCase to snake_case", "[json_naming]") {
    constexpr auto userId = webzen::to_snake_case<webzen::fixed_string("UserId")>();
    CHECK(userId.view() == "user_id");

    constexpr auto accessToken = webzen::to_snake_case<webzen::fixed_string("AccessToken")>();
    CHECK(accessToken.view() == "access_token");

    constexpr auto expiresAt = webzen::to_snake_case<webzen::fixed_string("ExpiresAt")>();
    CHECK(expiresAt.view() == "expires_at");

    constexpr auto url = webzen::to_snake_case<webzen::fixed_string("Url")>();
    CHECK(url.view() == "url");

    constexpr auto single = webzen::to_snake_case<webzen::fixed_string("Id")>();
    CHECK(single.view() == "id");
}
