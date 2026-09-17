#pragma once

#include <asio/awaitable.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace webzen {

// What a Command::Execute actually produces, before RequestId gets stitched
// back in. Kept separate from the public webzen::Result (request.hpp)
// because a command implementation never needs to see or forward the
// RequestId itself -- CommandRegistry::Dispatch owns that entirely, so a
// command can't forget to copy it (there's nothing to copy).
struct CommandOutcome {
    bool Success = false;
    std::string ErrorCode;
    std::string ErrorMessage;
    std::string Data;
};

class Command {
public:
    virtual ~Command() = default;
    // requestJson is the raw JSON body of whatever Request subtype this
    // command expects (RequestLogin, RequestInitialize, ...); the command
    // parses it itself via glz::read_json.
    virtual asio::awaitable<CommandOutcome> Execute(std::string requestJson) = 0;
};

using CommandFactory = std::function<std::unique_ptr<Command>()>;

// Runtime lookup table standing in for reflection (see "동적 커맨드 생성" in the
// architecture skill -- C++ has no runtime reflection usable across
// NDK/Xcode/MSVC today). Populated by REGISTER_COMMAND at static init time.
class CommandRegistry {
public:
    static CommandRegistry& Instance();

    void Register(std::string_view commandId, CommandFactory factory);
    [[nodiscard]] std::unique_ptr<Command> Create(std::string_view commandId) const;

    // Runs the command and returns the full, ready-to-send webzen::Result
    // JSON (RequestId echoed from requestJson, Success/ErrorCode/
    // ErrorMessage/Data filled from the command's CommandOutcome). This is
    // the one place RequestId correlation happens -- see request.hpp.
    asio::awaitable<std::string> Dispatch(std::string commandId, std::string requestJson) const;

private:
    std::unordered_map<std::string, CommandFactory> factories_;
};

struct CommandRegistrar {
    CommandRegistrar(std::string_view commandId, CommandFactory factory) {
        CommandRegistry::Instance().Register(commandId, std::move(factory));
    }
};

}  // namespace webzen

#define WEBZEN_CONCAT_INNER(a, b) a##b
#define WEBZEN_CONCAT(a, b) WEBZEN_CONCAT_INNER(a, b)

// Registers `ClassName` (may be namespace-qualified) under `id` at
// static-init time, e.g.:
//   REGISTER_COMMAND("auth.login", LoginCommand)
// Never skip this on a new command -- without it CommandRegistry::Dispatch
// simply won't find the command id at runtime (no compile error).
#define REGISTER_COMMAND(id, ClassName)                                     \
    namespace {                                                             \
    const ::webzen::CommandRegistrar WEBZEN_CONCAT(webzen_registrar_, __COUNTER__){ \
        id, [] { return std::make_unique<ClassName>(); }};                  \
    }
