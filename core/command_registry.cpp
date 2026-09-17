#include "core/command_registry.hpp"

namespace webzen {

namespace {
std::string ExtractRequestId(const std::string& requestJson) {
    Request baseRequest;
    (void)glz::read<glz::opts{.error_on_unknown_keys = false}>(baseRequest, requestJson);
    return baseRequest.RequestId;
}

std::string BuildErrorResult(std::string requestId, std::string errorCode, std::string errorMessage = {}) {
    Result result{
        .RequestId = std::move(requestId),
        .Success = false,
        .ErrorCode = std::move(errorCode),
        .ErrorMessage = std::move(errorMessage),
    };
    std::string json;
    (void)glz::write_json(result, json);
    return json;
}
}  // namespace

CommandRegistry& CommandRegistry::Instance() {
    static CommandRegistry instance;
    return instance;
}

void CommandRegistry::Register(std::string_view commandId, CommandFactory factory) {
    factories_.emplace(std::string(commandId), std::move(factory));
}

std::unique_ptr<ICommand> CommandRegistry::Create(std::string_view commandId) const {
    const auto it = factories_.find(std::string(commandId));
    if (it == factories_.end()) {
        return nullptr;
    }
    return it->second();
}

asio::awaitable<std::string> CommandRegistry::Dispatch(std::string commandId, std::string requestJson) const {
    auto command = Create(commandId);
    if (!command) {
        co_return BuildErrorResult(ExtractRequestId(requestJson), "unknown_command", "no command registered for id");
    }

    // Extracted before the move below: requestJson is consumed by
    // Execute(), so it can't be re-parsed from the catch block afterward.
    std::string requestId = ExtractRequestId(requestJson);
    try {
        co_return co_await command->Execute(std::move(requestJson));
    } catch (const std::exception& ex) {
        co_return BuildErrorResult(std::move(requestId), "exception", ex.what());
    }
}

}  // namespace webzen
