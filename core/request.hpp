#pragma once

#include <glaze/glaze.hpp>

#include <string>

namespace webzen {

// Every domain request is a plain, unrelated struct that happens to also
// declare RequestId (see e.g. auth/auth_types.hpp's RequestLogin) -- not a
// subtype of this one. RequestId is plumbing, not business data:
// CommandRegistry::Dispatch reads it out of the raw incoming JSON with this
// type (which is why it has to exist as its own type, even though nothing
// derives from it) and echoes it into the outgoing Result without any
// domain code touching it, so a caller with several in-flight requests can
// tell which callback goes with which response without the C ABI needing a
// per-call user_data.
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

// glz::snake_case: PascalCase member names get automatically reflected
// (no glz::meta field list to keep in sync by hand) and renamed to
// snake_case JSON keys. Every plain DTO in this codebase uses this same
// one-liner -- see the architecture skill's JSON section for why, and for
// why this only works for structs with no base class of their own.
template <>
struct glz::meta<webzen::Request> : glz::snake_case {};

template <>
struct glz::meta<webzen::Result> : glz::snake_case {};
