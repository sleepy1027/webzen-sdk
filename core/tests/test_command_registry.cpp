#include "core/command_registry.hpp"
#include "core/request.hpp"

#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/use_future.hpp>

#include <glaze/glaze.hpp>

#include <catch2/catch_test_macros.hpp>

#include <future>
#include <string>

namespace {

struct RequestEcho {
    std::string RequestId;
    std::string Message;
};

struct ResultEcho {
    std::string RequestId;
    bool Success = false;
    std::string ErrorCode;
    std::string ErrorMessage;
    std::string Message;
};

}  // namespace

WEBZEN_JSON(RequestEcho);
WEBZEN_JSON(ResultEcho);

namespace {

class EchoCommand : public webzen::Command<RequestEcho, ResultEcho> {
protected:
    asio::awaitable<ResultEcho> ExecuteTyped(RequestEcho request) override {
        co_return ResultEcho{.Success = true, .Message = std::move(request.Message)};
    }
};

}  // namespace

REGISTER_COMMAND("test.echo", EchoCommand)

namespace {
webzen::Result ParseResult(const std::string& json) {
    webzen::Result result;
    REQUIRE_FALSE(glz::read<glz::opts{.error_on_unknown_keys = false}>(result, json));
    return result;
}

ResultEcho ParseEchoResult(const std::string& json) {
    ResultEcho result;
    REQUIRE_FALSE(glz::read_json(result, json));
    return result;
}
}  // namespace

TEST_CASE("CommandRegistry dispatches a registered command", "[command_registry]") {
    asio::io_context io;
    auto future = asio::co_spawn(
        io, webzen::CommandRegistry::Instance().Dispatch("test.echo", R"({"request_id":"r1","message":"hello"})"),
        asio::use_future);
    io.run();

    const auto result = ParseEchoResult(future.get());
    CHECK(result.Success);
    CHECK(result.RequestId == "r1");
    CHECK(result.Message == "hello");
}

TEST_CASE("CommandRegistry.Dispatch reads request_id even when other fields come first in the JSON", "[command_registry]") {
    // Regression test: Glaze's default opts (error_on_unknown_keys=true)
    // stop parsing at the first key CommandRegistry's bare `Request` type
    // doesn't recognize. If request_id happened to come after such a key,
    // a plain glz::read_json would leave RequestId empty -- Dispatch must
    // use error_on_unknown_keys=false, not the default, or this silently
    // breaks the whole RequestId-based callback correlation.
    asio::io_context io;
    auto future = asio::co_spawn(
        io,
        webzen::CommandRegistry::Instance().Dispatch("test.echo", R"({"message":"hello","request_id":"r3"})"),
        asio::use_future);
    io.run();

    const auto result = ParseEchoResult(future.get());
    CHECK(result.RequestId == "r3");
}

TEST_CASE("CommandRegistry reports unknown commands instead of crashing", "[command_registry]") {
    asio::io_context io;
    auto future = asio::co_spawn(io, webzen::CommandRegistry::Instance().Dispatch("does.not.exist", "{}"), asio::use_future);
    io.run();

    const auto result = ParseResult(future.get());
    CHECK_FALSE(result.Success);
    CHECK(result.ErrorCode == "unknown_command");
}

namespace {

// Mirrors how Sdk::DispatchCommand actually calls Dispatch: a wrapper takes
// the id/json by const&, so the strings only live for this call's full
// expression, then the coroutine is spawned and only actually runs once
// io.run() executes below -- i.e. after this function has returned. Dispatch
// must own its own copies or this dangles (regression test for exactly that
// bug, once caught here when Dispatch took std::string_view).
std::future<std::string> DispatchLikeSdkDoes(asio::io_context& io, const std::string& commandId, const std::string& requestJson) {
    return asio::co_spawn(io, webzen::CommandRegistry::Instance().Dispatch(commandId, requestJson), asio::use_future);
}

}  // namespace

TEST_CASE("CommandRegistry.Dispatch does not dangle when the id/json strings are short-lived temporaries", "[command_registry]") {
    asio::io_context io;
    const std::string commandId = std::string("test") + ".echo";
    const std::string requestJson = std::string(R"({"request_id":"r2"})");
    auto future = DispatchLikeSdkDoes(io, commandId, requestJson);
    io.run();

    const auto result = ParseEchoResult(future.get());
    CHECK(result.Success);
    CHECK(result.RequestId == "r2");
}
