#include "auth/login_command.hpp"

#include <glaze/glaze.hpp>

namespace webzen::auth {

LoginCommand::LoginCommand() : authService_(GetAuthService()) {}

asio::awaitable<CommandOutcome> LoginCommand::ExecuteTyped(RequestLogin request) {
    const AuthResult result = co_await authService_.Login(std::move(request));

    if (!result.Success) {
        co_return CommandOutcome{.Success = false, .ErrorCode = "login_failed"};
    }

    std::string data;
    if (glz::write_json(result.Token, data)) {
        co_return CommandOutcome{.Success = false, .ErrorCode = "serialization_failed"};
    }

    co_return CommandOutcome{.Success = true, .Data = std::move(data)};
}

}  // namespace webzen::auth

REGISTER_COMMAND("auth.login", webzen::auth::LoginCommand)
