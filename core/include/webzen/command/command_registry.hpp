#pragma once

#include <asio/awaitable.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace webzen::command {

// Every command's input/output crosses the engine bridge (Unreal/Unity) as
// an opaque JSON string -- the C ABI in sdk.hpp has no way to express a
// typed payload, and command implementations already own their DTOs via
// glz::meta, so a second typed layer here would just duplicate that.
struct CommandContext {
    std::string Payload;
};

struct CommandResult {
    bool Success = false;
    std::string Payload;
};

class Command {
public:
    virtual ~Command() = default;
    virtual asio::awaitable<CommandResult> Execute(CommandContext context) = 0;
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

    // Takes commandId by value (not string_view): as a coroutine, this
    // doesn't actually run until asio::co_spawn resumes it later on the
    // executor, by which point anything the caller only borrowed (e.g. a
    // std::string temporary bound to a const&) may already be gone. Owning
    // the string in the coroutine frame keeps it alive for the right span.
    asio::awaitable<CommandResult> Dispatch(std::string commandId, CommandContext context) const;

private:
    std::unordered_map<std::string, CommandFactory> factories_;
};

struct CommandRegistrar {
    CommandRegistrar(std::string_view commandId, CommandFactory factory) {
        CommandRegistry::Instance().Register(commandId, std::move(factory));
    }
};

}  // namespace webzen::command

#define WEBZEN_CONCAT_INNER(a, b) a##b
#define WEBZEN_CONCAT(a, b) WEBZEN_CONCAT_INNER(a, b)

// Registers `ClassName` (may be namespace-qualified) under `id` at
// static-init time, e.g.:
//   REGISTER_COMMAND("auth.login", LoginCommand)
// Never skip this on a new command -- without it CommandRegistry::Dispatch
// simply won't find the command id at runtime (no compile error).
#define REGISTER_COMMAND(id, ClassName)                                                     \
    namespace {                                                                             \
    const ::webzen::command::CommandRegistrar WEBZEN_CONCAT(webzen_registrar_, __COUNTER__){ \
        id, [] { return std::make_unique<ClassName>(); }};                                  \
    }
