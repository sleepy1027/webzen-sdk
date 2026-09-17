#pragma once

#include "core/command_registry.hpp"

#include <string>

namespace webzen {

struct RequestInitialize {
    std::string RequestId;
    std::string BaseUrl;
};

// "core.initialize" -- setting BaseUrl is plumbing (every domain reads it
// back via Sdk::Instance().BaseUrl()), not a business feature, but it still
// goes through DispatchCommand like everything else: there is no separate
// Initialize entry point on the C ABI (see core/sdk_c_api.h). Uses the
// default Result (no extra fields to report).
class InitializeCommand : public Command<RequestInitialize> {
protected:
    asio::awaitable<Result> ExecuteTyped(RequestInitialize request) override;
};

}  // namespace webzen

WEBZEN_JSON(webzen::RequestInitialize);
