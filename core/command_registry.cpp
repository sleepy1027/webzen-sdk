#include "core/command_registry.hpp"

#include "core/request.hpp"

#include <glaze/glaze.hpp>

namespace webzen {

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

asio::awaitable<std::string> CommandRegistry::Dispatch(std::string commandId, std::string requestJson) const {
    // Only RequestId is read here -- deliberately not the concrete Request
    // subtype, since CommandRegistry has no idea which one a given command
    // id expects. Glaze's default opts (plain read_json) error out on the
    // first unknown key and stop parsing right there, so if request_id
    // happened to come after e.g. provider_id in the JSON, it would never
    // get read -- error_on_unknown_keys=false is required here, not optional.
    Request baseRequest;
    (void)glz::read<glz::opts{.error_on_unknown_keys = false}>(baseRequest, requestJson);

    CommandOutcome outcome;
    auto command = Create(commandId);
    if (!command) {
        outcome = CommandOutcome{.Success = false, .ErrorCode = "unknown_command", .ErrorMessage = "no command registered for id"};
    } else {
        try {
            outcome = co_await command->Execute(std::move(requestJson));
        } catch (const std::exception& ex) {
            outcome = CommandOutcome{.Success = false, .ErrorCode = "exception", .ErrorMessage = ex.what()};
        }
    }

    Result result{
        .RequestId = baseRequest.RequestId,
        .Success = outcome.Success,
        .ErrorCode = outcome.ErrorCode,
        .ErrorMessage = outcome.ErrorMessage,
        .Data = outcome.Data,
    };

    std::string resultJson;
    (void)glz::write_json(result, resultJson);  // Result's own fields are all plain strings/bool; this cannot fail
    co_return resultJson;
}

}  // namespace webzen
