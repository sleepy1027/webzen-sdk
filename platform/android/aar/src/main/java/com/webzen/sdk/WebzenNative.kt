package com.webzen.sdk

/**
 * Raw JNI surface backed by libwebzen_core.so (see
 * platform/android/adapter/src/jni_bridge.cpp). Game code should use
 * [WebzenSDK] instead -- this object exists so the JNI boundary has exactly
 * one entry point to keep in sync with the C++ side.
 */
internal object WebzenNative {
    init {
        System.loadLibrary("webzen_core")
    }

    fun interface ResultCallback {
        fun onResult(success: Boolean, jsonPayload: String)
    }

    external fun nativeInitialize(baseUrl: String)
    external fun nativeShutdown()
    external fun nativeDispatchCommand(commandId: String, jsonPayload: String, callback: ResultCallback)
}
