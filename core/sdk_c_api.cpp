#include "core/sdk_c_api.h"

#include "core/sdk.hpp"

#include <string>

void Webzen_DispatchCommand(const char* command_id, const char* request_json, WebzenResultCallback callback) {
    if (!callback) {
        return;
    }

    webzen::Sdk::Instance().DispatchCommand(
        command_id ? command_id : "",
        request_json ? request_json : "",
        [callback](std::string resultJson) { callback(resultJson.c_str()); });
}

void Webzen_Shutdown(void) {
    webzen::Sdk::Instance().Shutdown();
}
