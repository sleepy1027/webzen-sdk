#pragma once

#include "auth/auth_service.hpp"
#include "core/command_registry.hpp"

namespace webzen::auth {

// Bridges the engine-facing "auth.login" command id (see core/sdk_c_api.h)
// to AuthService::Login. Registered with REGISTER_COMMAND in
// login_command.cpp. Default-constructible (as CommandFactory requires) --
// it resolves the live AuthService via GetAuthService() rather than taking
// it as a constructor argument.
class LoginCommand : public Command<RequestLogin, ResultLogin> {
public:
    LoginCommand();

protected:
    asio::awaitable<ResultLogin> ExecuteTyped(RequestLogin request) override;

private:
    AuthService& authService_;
};

}  // namespace webzen::auth
