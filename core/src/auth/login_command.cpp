#include "webzen/auth/login_command.hpp"

#include "webzen/sdk.hpp"

#include <glaze/glaze.hpp>

namespace webzen::auth {

LoginCommand::LoginCommand() : authService_(sdk::Sdk::Instance().Auth()) {}

asio::awaitable<command::CommandResult> LoginCommand::Execute(command::CommandContext context) {
    LoginRequestDto request;
    if (glz::read_json(request, context.Payload)) {
        co_return command::CommandResult{.Success = false, .Payload = R"({"error":"invalid_request"})"};
    }

    const AuthResult result = co_await authService_.Login(std::move(request));

    std::string responseJson;
    if (result.Success && !glz::write_json(result.Token, responseJson)) {
        co_return command::CommandResult{.Success = true, .Payload = std::move(responseJson)};
    }

    co_return command::CommandResult{.Success = false, .Payload = R"({"error":"login_failed"})"};
}

}  // namespace webzen::auth

REGISTER_COMMAND("auth.login", webzen::auth::LoginCommand)
