#include "core/initialize_command.hpp"

#include "core/sdk.hpp"

namespace webzen {

asio::awaitable<CommandOutcome> InitializeCommand::ExecuteTyped(RequestInitialize request) {
    Sdk::Instance().SetBaseUrl(std::move(request.BaseUrl));
    co_return CommandOutcome{.Success = true};
}

}  // namespace webzen

REGISTER_COMMAND("core.initialize", webzen::InitializeCommand)
