#pragma once

#include <asio/awaitable.hpp>

#include <glaze/glaze.hpp>

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
    // requestJson is the raw JSON body of whatever request type this
    // command expects (RequestLogin, RequestInitialize, ...). Most commands
    // should derive from TypedCommand<TRequest> below instead of
    // implementing this directly -- it does the JSON parsing for you.
    virtual asio::awaitable<CommandOutcome> Execute(std::string requestJson) = 0;
};

// Request DTOs are plain, unrelated structs -- no shared base class (see
// request.hpp for why: Glaze's automatic reflection, which is what lets a
// DTO skip writing glz::meta by hand, doesn't support base classes). So the
// "one common way to call Execute" the architecture skill wants for command
// implementations comes from a template instead of from inheriting a common
// Request type: TypedCommand<TRequest> parses requestJson into TRequest
// once, here, so every command implementation is written against a typed
// request and never touches glz::read directly.
template <typename TRequest>
class TypedCommand : public Command {
public:
    asio::awaitable<CommandOutcome> Execute(std::string requestJson) final {
        TRequest request;
        // error_on_unknown_keys=false: requestJson may legitimately carry
        // fields this command doesn't declare (e.g. request_id is on every
        // request but plenty of commands never need to read it back out),
        // and Glaze's default stops parsing at the first such field --
        // see the CommandRegistry::Dispatch bug this exact policy fixed.
        if (glz::read<glz::opts{.error_on_unknown_keys = false}>(request, requestJson)) {
            co_return CommandOutcome{.Success = false, .ErrorCode = "invalid_request"};
        }
        co_return co_await ExecuteTyped(std::move(request));
    }

protected:
    virtual asio::awaitable<CommandOutcome> ExecuteTyped(TRequest request) = 0;
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
