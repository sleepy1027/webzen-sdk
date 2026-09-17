#pragma once

#include <glaze/glaze.hpp>

#include <concepts>
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

// The default result shape; commands that need to return more than
// success/failure define their own ResultXXX with the same four fields
// plus whatever extra data they need (see auth/auth_types.hpp's
// ResultLogin) instead of stuffing a pre-serialized blob into a generic
// field here -- see Command<TRequest, TResult> in command_registry.hpp.
struct Result {
    std::string RequestId;
    bool Success = false;
    std::string ErrorCode;
    std::string ErrorMessage;
};

// What Command<TRequest, TResult> requires of TRequest/TResult. Formalized
// as concepts so getting a new RequestXXX/ResultXXX wrong (e.g. forgetting
// ErrorCode) fails with "does not satisfy ResultLike" at the Command<...>
// declaration, not a page of template-instantiation noise from deep inside
// command_registry.cpp.
template <typename T>
concept RequestLike = requires(T& t) {
    { t.RequestId } -> std::convertible_to<std::string&>;
};

template <typename T>
concept ResultLike = requires(T& t) {
    { t.RequestId } -> std::convertible_to<std::string&>;
    { t.Success } -> std::convertible_to<bool&>;
    { t.ErrorCode } -> std::convertible_to<std::string&>;
    { t.ErrorMessage } -> std::convertible_to<std::string&>;
};

// The one way to read JSON in this codebase -- never glz::read_json
// directly. Glaze's default opts (what glz::read_json uses) stop parsing at
// the first key the target type doesn't declare, silently leaving every
// field after it at its default value. That bit CommandRegistry once
// (a RequestId placed after an unrecognized field never got read), and the
// same risk applies to any JSON this SDK doesn't fully control the shape of
// -- a server response that gains a field a given SDK version doesn't know
// about yet, or a locally-stored token written by a newer SDK version.
// Same signature/contract as glz::read_json (returns a bool-convertible
// error_ctx; truthy means failure) so it's a drop-in replacement.
template <typename T>
auto ReadJson(T& value, const std::string& json) {
    return glz::read<glz::opts{.error_on_unknown_keys = false}>(value, json);
}

}  // namespace webzen

// glz::snake_case: PascalCase member names get automatically reflected
// (no glz::meta field list to keep in sync by hand) and renamed to
// snake_case JSON keys. WEBZEN_JSON is just that one-liner spelled once --
// every plain DTO in this codebase uses it (see the architecture skill's
// JSON section for why, and for why this only works for structs with no
// base class of their own). Called with a trailing ';', like static_assert:
//   WEBZEN_JSON(webzen::auth::RequestLogin);
#define WEBZEN_JSON(Type) \
    template <>           \
    struct glz::meta<Type> : glz::snake_case {}

WEBZEN_JSON(webzen::Request);
WEBZEN_JSON(webzen::Result);
