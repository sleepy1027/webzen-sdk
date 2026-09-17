#include "auth/login_command.hpp"

namespace webzen::auth {

LoginCommand::LoginCommand() : authService_(GetAuthService()) {}

asio::awaitable<ResultLogin> LoginCommand::ExecuteTyped(RequestLogin request) {
    const AuthResult result = co_await authService_.Login(std::move(request));

    if (!result.Success) {
        co_return ResultLogin{.Success = false, .ErrorCode = "login_failed"};
    }

    co_return ResultLogin{
        .Success = true,
        .UserId = result.Token.UserId,
        .AccessToken = result.Token.AccessToken,
        .RefreshToken = result.Token.RefreshToken,
        .ExpiresAt = result.Token.ExpiresAt,
    };
}

}  // namespace webzen::auth

REGISTER_COMMAND("auth.login", webzen::auth::LoginCommand)
