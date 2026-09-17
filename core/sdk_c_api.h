#ifndef WEBZEN_SDK_C_API_H
#define WEBZEN_SDK_C_API_H

#include "core/export.h"

/*
 * Flat C ABI for the engine bridges (Unity's [DllImport], the Unreal
 * bridge, and Android JNI_OnLoad bootstrap on the native side). Every other
 * header under core/, auth/, etc. is C++ and never included from
 * engine-side code.
 *
 * There is exactly one operation: dispatch a command by id with a JSON
 * request body, get a JSON webzen::Result body back on `callback`. Even
 * SDK bootstrapping goes through this ("core.initialize", see
 * core/initialize_command.hpp) -- there is no separate Initialize entry
 * point. No user_data: the request's `request_id` field (base
 * webzen::Request, see core/request.hpp) is echoed into the result, so a
 * caller with several requests in flight can correlate a result back to
 * its own pending callback using that instead.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*WebzenResultCallback)(const char* result_json);

/* command_id examples: "core.initialize", "auth.login". result_json is
 * always a full webzen::Result (request_id, success, error_code,
 * error_message, data), delivered on the SDK's own worker thread -- not
 * necessarily the caller's thread. */
WEBZEN_API void Webzen_DispatchCommand(const char* command_id, const char* request_json, WebzenResultCallback callback);

/* Plumbing, not a business command (see core/sdk.hpp): stops the SDK's
 * worker thread. Safe to call even if no command was ever dispatched. */
WEBZEN_API void Webzen_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif  /* WEBZEN_SDK_C_API_H */
