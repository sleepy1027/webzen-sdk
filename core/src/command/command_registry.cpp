#include "webzen/command/command_registry.hpp"

#include <asio/co_spawn.hpp>
#include <asio/this_coro.hpp>

namespace webzen::command {

CommandRegistry& CommandRegistry::Instance() {
    static CommandRegistry instance;
    return instance;
}

void CommandRegistry::Register(std::string_view commandId, CommandFactory factory) {
    factories_.emplace(std::string(commandId), std::move(factory));
}

std::unique_ptr<Command> CommandRegistry::Create(std::string_view commandId) const {
    const auto it = factories_.find(std::string(commandId));
    if (it == factories_.end()) {
        return nullptr;
    }
    return it->second();
}

asio::awaitable<CommandResult> CommandRegistry::Dispatch(std::string commandId, CommandContext context) const {
    auto command = Create(commandId);
    if (!command) {
        co_return CommandResult{.Success = false, .Payload = R"({"error":"unknown_command"})"};
    }

    try {
        co_return co_await command->Execute(std::move(context));
    } catch (const std::exception& ex) {
        co_return CommandResult{.Success = false, .Payload = std::string(R"({"error":"exception","message":")") + ex.what() + "\"}"};
    }
}

}  // namespace webzen::command
