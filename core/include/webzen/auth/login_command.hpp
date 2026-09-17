#pragma once

#include "webzen/auth/auth_service.hpp"
#include "webzen/command/command_registry.hpp"

namespace webzen::auth {

// Bridges the engine-facing "auth.login" command id (see sdk.hpp /
// Webzen_DispatchCommand) to AuthService::Login. Registered with
// REGISTER_COMMAND in login_command.cpp. Default-constructible (as
// CommandFactory requires) -- it resolves the live AuthService from the Sdk
// facade rather than taking it as a constructor argument.
class LoginCommand : public command::Command {
public:
    LoginCommand();

    asio::awaitable<command::CommandResult> Execute(command::CommandContext context) override;

private:
    AuthService& authService_;
};

}  // namespace webzen::auth
