#include "webzen/sdk_c_api.h"

#include "webzen/sdk.hpp"

#include <cstring>
#include <string>

void Webzen_Initialize(const char* base_url) {
    webzen::sdk::Sdk::Instance().Initialize(webzen::sdk::SdkConfig{.BaseUrl = base_url ? base_url : ""});
}

void Webzen_Shutdown(void) {
    webzen::sdk::Sdk::Instance().Shutdown();
}

void Webzen_DispatchCommand(const char* command_id, const char* json_payload, WebzenCommandCallback callback, void* user_data) {
    if (!callback) {
        return;
    }

    webzen::sdk::Sdk::Instance().DispatchCommand(
        command_id ? command_id : "",
        json_payload ? json_payload : "",
        [callback, user_data](bool success, std::string payload) {
            callback(success ? 1 : 0, payload.c_str(), user_data);
        });
}
