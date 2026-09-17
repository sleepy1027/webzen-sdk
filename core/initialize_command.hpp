#pragma once

#include "core/command_registry.hpp"
#include "core/request.hpp"

#include <glaze/glaze.hpp>

#include <string>

namespace webzen {

struct RequestInitialize : Request {
    std::string BaseUrl;
};

// "core.initialize" -- setting BaseUrl is plumbing (every domain reads it
// back via Sdk::Instance().BaseUrl()), not a business feature, but it still
// goes through DispatchCommand like everything else: there is no separate
// Initialize entry point on the C ABI (see core/sdk_c_api.h).
class InitializeCommand : public Command {
public:
    asio::awaitable<CommandOutcome> Execute(std::string requestJson) override;
};

}  // namespace webzen

template <>
struct glz::meta<webzen::RequestInitialize> {
    using T = webzen::RequestInitialize;
    static constexpr auto value = glz::object(SDK_FIELD(T, RequestId), SDK_FIELD(T, BaseUrl));
};
