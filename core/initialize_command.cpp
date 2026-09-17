#include "core/initialize_command.hpp"

#include "core/sdk.hpp"

#include <glaze/glaze.hpp>

namespace webzen {

asio::awaitable<CommandOutcome> InitializeCommand::Execute(std::string requestJson) {
    RequestInitialize request;
    if (glz::read_json(request, requestJson)) {
        co_return CommandOutcome{.Success = false, .ErrorCode = "invalid_request", .ErrorMessage = "malformed core.initialize request"};
    }

    Sdk::Instance().SetBaseUrl(std::move(request.BaseUrl));
    co_return CommandOutcome{.Success = true};
}

}  // namespace webzen

REGISTER_COMMAND("core.initialize", webzen::InitializeCommand)
