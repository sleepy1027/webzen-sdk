#pragma once

#include "core/request.hpp"

#include <asio/awaitable.hpp>

#include <glaze/glaze.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace webzen {

// Raw, type-erased command interface. CommandRegistry stores commands by
// string id (self-registering factory, see REGISTER_COMMAND below) and
// can't know the concrete request/result types at that point, so it needs
// a common non-template base to hold them behind -- this is that base.
// Almost nothing should implement this directly; derive from
// Command<TRequest, TResult> below instead, which handles the JSON parsing/
// serialization this interface leaves as a raw string in and raw string out.
class ICommand {
public:
    virtual ~ICommand() = default;
    // requestJson in, a full webzen::Result-shaped (or ResultXXX-shaped)
    // JSON string out -- NOT just a domain payload; RequestId, Success,
    // ErrorCode, ErrorMessage are already in there. CommandRegistry::Dispatch
    // only adds this wrapping itself for the unknown-command/exception cases,
    // where there's no command to have done it.
    virtual asio::awaitable<std::string> Execute(std::string requestJson) = 0;
};

// What almost every command actually derives from. Parses requestJson into
// TRequest, calls ExecuteTyped, stamps RequestId onto whatever TResult it
// returns (so ExecuteTyped never has to -- see request.hpp on why that's
// plumbing, not business logic), and serializes the result. TResult
// defaults to the plain Result for commands with nothing more to report
// than success/failure; give it a richer ResultXXX (see auth/auth_types.hpp's
// ResultLogin) when a command needs to return more.
template <RequestLike TRequest, ResultLike TResult = Result>
class Command : public ICommand {
public:
    asio::awaitable<std::string> Execute(std::string requestJson) final {
        TRequest request;
        // error_on_unknown_keys=false: requestJson may legitimately carry
        // fields this command doesn't declare, and Glaze's default stops
        // parsing at the first such field -- see the CommandRegistry bug
        // this exact policy fixed.
        const bool parseFailed = bool(glz::read<glz::opts{.error_on_unknown_keys = false}>(request, requestJson));

        std::string resultJson;
        if (parseFailed) {
            TResult errorResult{};
            errorResult.RequestId = request.RequestId;  // may still be set even if some other field failed to parse
            errorResult.Success = false;
            errorResult.ErrorCode = "invalid_request";
            (void)glz::write_json(errorResult, resultJson);
            co_return resultJson;
        }

        TResult result = co_await ExecuteTyped(request);
        result.RequestId = request.RequestId;
        (void)glz::write_json(result, resultJson);
        co_return resultJson;
    }

protected:
    virtual asio::awaitable<TResult> ExecuteTyped(TRequest request) = 0;
};

using CommandFactory = std::function<std::unique_ptr<ICommand>()>;

// Runtime lookup table standing in for reflection (see "동적 커맨드 생성" in the
// architecture skill -- C++ has no runtime reflection usable across
// NDK/Xcode/MSVC today). Populated by REGISTER_COMMAND at static init time.
class CommandRegistry {
public:
    static CommandRegistry& Instance();

    void Register(std::string_view commandId, CommandFactory factory);
    [[nodiscard]] std::unique_ptr<ICommand> Create(std::string_view commandId) const;

    // Runs the command and returns its Result JSON as-is; for the
    // unknown-command/exception cases (no command to have produced one)
    // builds a plain Result JSON itself, still echoing RequestId.
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
