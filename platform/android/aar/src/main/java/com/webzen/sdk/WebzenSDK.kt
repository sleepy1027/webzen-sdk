package com.webzen.sdk

import org.json.JSONObject

/**
 * Public entry point shipped in the .aar. Unity calls this via
 * AndroidJavaClass/AndroidJavaObject (see bridge/unity/Runtime/Android);
 * Unreal calls it from its own Java glue registered through
 * WebzenSDK_UPL_Android.xml (see bridge/unreal). Every method here is a thin
 * wrapper over WebzenNative -- no business logic lives on this side of the
 * JNI boundary, per the architecture skill.
 */
object WebzenSDK {
    fun interface LoginCallback {
        fun onResult(success: Boolean, userId: String?, accessToken: String?, errorMessage: String?)
    }

    @Volatile
    private var initialized = false

    @Synchronized
    fun initialize(baseUrl: String) {
        if (initialized) return
        WebzenNative.nativeInitialize(baseUrl)
        initialized = true
    }

    fun shutdown() {
        WebzenNative.nativeShutdown()
        initialized = false
    }

    fun login(providerId: String, providerToken: String, callback: LoginCallback) {
        check(initialized) { "WebzenSDK.initialize(baseUrl) must be called first" }

        val request = JSONObject()
            .put("provider_id", providerId)
            .put("provider_token", providerToken)
            .put("device_id", "")

        WebzenNative.nativeDispatchCommand("auth.login", request.toString()) { success, jsonPayload ->
            if (!success) {
                callback.onResult(false, null, null, JSONObject(jsonPayload).optString("error", "unknown_error"))
                return@nativeDispatchCommand
            }
            val response = JSONObject(jsonPayload)
            callback.onResult(true, response.optString("user_id"), response.optString("access_token"), null)
        }
    }
}
