#ifndef WEBZEN_SDK_C_API_H
#define WEBZEN_SDK_C_API_H

#include "webzen/export.h"

/*
 * Flat C ABI for the engine bridges: JNI native methods (Android AAR),
 * the Objective-C wrapper (iOS .xcframework) and P/Invoke (Windows .dll,
 * called directly from the Unity C# bridge). Every other header under
 * webzen/ is C++ and is never included from engine-side code.
 *
 * Callers must invoke Webzen_Initialize exactly once after the platform
 * adapter has been installed (native adapter bootstrap runs before this,
 * from each platform's own init path -- see the platform adapter directories
 * under platform/android, platform/ios and platform/windows).
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*WebzenCommandCallback)(int success, const char* json_payload, void* user_data);

WEBZEN_API void Webzen_Initialize(const char* base_url);
WEBZEN_API void Webzen_Shutdown(void);

/* commandId examples: "auth.login". jsonPayload is the command's JSON
 * request body; the result comes back on `callback`, invoked from the SDK's
 * own worker thread (not necessarily the caller's thread). */
WEBZEN_API void Webzen_DispatchCommand(const char* command_id, const char* json_payload, WebzenCommandCallback callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif  /* WEBZEN_SDK_C_API_H */
