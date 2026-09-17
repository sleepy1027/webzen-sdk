#include "core/request.hpp"

#include <glaze/glaze.hpp>

#include <catch2/catch_test_macros.hpp>

// Every DTO in this codebase relies on glz::meta<T> : glz::snake_case {}
// (automatic reflection + automatic PascalCase -> snake_case renaming,
// see core/request.hpp) instead of hand-listing fields. This is Glaze's own
// behavior, not this codebase's, but it's load-bearing for every command's
// wire format, so a round trip is worth guarding here.
TEST_CASE("webzen::Result round-trips through snake_case JSON keys", "[request]") {
    webzen::Result result{
        .RequestId = "r1",
        .Success = true,
        .ErrorCode = "e",
        .ErrorMessage = "m",
        .Data = "d",
    };

    std::string json;
    REQUIRE_FALSE(glz::write_json(result, json));
    CHECK(json == R"({"request_id":"r1","success":true,"error_code":"e","error_message":"m","data":"d"})");

    webzen::Result parsed;
    REQUIRE_FALSE(glz::read_json(parsed, json));
    CHECK(parsed.RequestId == "r1");
    CHECK(parsed.Success);
    CHECK(parsed.ErrorCode == "e");
    CHECK(parsed.ErrorMessage == "m");
    CHECK(parsed.Data == "d");
}
