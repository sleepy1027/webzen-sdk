#pragma once

#include "core/json_naming.hpp"

#include <glaze/glaze.hpp>

#include <string>

namespace webzen {

// Every domain request derives from this (RequestLogin : Request, see
// auth/auth_types.hpp) and every DispatchCommand call gets exactly one
// Result back, whatever the command. RequestId is plumbing, not business
// data: CommandRegistry::Dispatch reads it out of the raw incoming JSON and
// echoes it into the outgoing Result without any domain code touching it,
// so a caller with several in-flight requests can tell which callback goes
// with which response without the C ABI needing a per-call user_data.
struct Request {
    std::string RequestId;
};

// The one shape every DispatchCommand callback receives. Domain-specific
// output (e.g. AuthTokenDto) is carried pre-serialized in Data; callers
// parse it themselves once they know which request they sent.
struct Result {
    std::string RequestId;
    bool Success = false;
    std::string ErrorCode;
    std::string ErrorMessage;
    std::string Data;
};

}  // namespace webzen

template <>
struct glz::meta<webzen::Request> {
    using T = webzen::Request;
    static constexpr auto value = glz::object(SDK_FIELD(T, RequestId));
};

template <>
struct glz::meta<webzen::Result> {
    using T = webzen::Result;
    static constexpr auto value = glz::object(
        SDK_FIELD(T, RequestId),
        SDK_FIELD(T, Success),
        SDK_FIELD(T, ErrorCode),
        SDK_FIELD(T, ErrorMessage),
        SDK_FIELD(T, Data)
    );
};
