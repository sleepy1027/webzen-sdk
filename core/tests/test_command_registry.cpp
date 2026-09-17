#include "webzen/command/command_registry.hpp"

#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/use_future.hpp>

#include <catch2/catch_test_macros.hpp>

#include <future>
#include <string>

namespace {

class EchoCommand : public webzen::command::Command {
public:
    asio::awaitable<webzen::command::CommandResult> Execute(webzen::command::CommandContext context) override {
        co_return webzen::command::CommandResult{.Success = true, .Payload = context.Payload};
    }
};

}  // namespace

REGISTER_COMMAND("test.echo", EchoCommand)

TEST_CASE("CommandRegistry dispatches a registered command", "[command_registry]") {
    asio::io_context io;
    auto future = asio::co_spawn(
        io,
        webzen::command::CommandRegistry::Instance().Dispatch("test.echo", webzen::command::CommandContext{.Payload = "hello"}),
        asio::use_future);
    io.run();

    const auto result = future.get();
    CHECK(result.Success);
    CHECK(result.Payload == "hello");
}

TEST_CASE("CommandRegistry reports unknown commands instead of crashing", "[command_registry]") {
    asio::io_context io;
    auto future = asio::co_spawn(
        io,
        webzen::command::CommandRegistry::Instance().Dispatch("does.not.exist", webzen::command::CommandContext{}),
        asio::use_future);
    io.run();

    const auto result = future.get();
    CHECK_FALSE(result.Success);
}

namespace {

// Mirrors how Sdk::DispatchCommand actually calls Dispatch: a wrapper takes
// the id by const&, so the string only lives for this call's full
// expression, then the coroutine is spawned and only actually runs once
// io.run() executes below -- i.e. after this function has returned. Dispatch
// must own its own copy of the id or this dangles (regression test for
// exactly that bug: Dispatch used to take std::string_view).
std::future<webzen::command::CommandResult> DispatchLikeSdkDoes(asio::io_context& io, const std::string& commandId) {
    return asio::co_spawn(io, webzen::command::CommandRegistry::Instance().Dispatch(commandId, webzen::command::CommandContext{.Payload = "hello"}), asio::use_future);
}

}  // namespace

TEST_CASE("CommandRegistry.Dispatch does not dangle when the id string is a short-lived temporary", "[command_registry]") {
    asio::io_context io;
    // Built at runtime (not a string literal) so it isn't in static storage,
    // and passed through a const&-taking wrapper on purpose.
    const std::string commandId = std::string("test") + ".echo";
    auto future = DispatchLikeSdkDoes(io, commandId);
    io.run();

    const auto result = future.get();
    CHECK(result.Success);
    CHECK(result.Payload == "hello");
}
