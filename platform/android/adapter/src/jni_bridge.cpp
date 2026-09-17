// JNI surface for com.webzen.sdk (see aar/src/main/java/com/webzen/sdk/WebzenNative.kt).
// This is the only file in the Android adapter that Kotlin calls into
// directly; everything else here is implementation detail.

#include "webzen/adapter/android_adapter.hpp"
#include "webzen/sdk.hpp"
#include "webzen/sdk_c_api.h"

#include <jni.h>

#include <memory>

namespace {

webzen::platform::android::AndroidSecureStorage g_secureStorage;
webzen::platform::android::AndroidDeviceInfo g_deviceInfo;

}  // namespace

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    webzen::platform::android::InitJniCache(vm, env);
    webzen::sdk::Sdk::Instance().SetPlatformAdapter(
        webzen::adapter::PlatformAdapter{.SecureStorage = &g_secureStorage, .DeviceInfo = &g_deviceInfo});

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL Java_com_webzen_sdk_WebzenNative_nativeInitialize(JNIEnv* env, jclass, jstring baseUrl) {
    const char* url = env->GetStringUTFChars(baseUrl, nullptr);
    Webzen_Initialize(url);
    env->ReleaseStringUTFChars(baseUrl, url);
}

extern "C" JNIEXPORT void JNICALL Java_com_webzen_sdk_WebzenNative_nativeShutdown(JNIEnv*, jclass) {
    Webzen_Shutdown();
}

namespace {

struct DispatchCallbackContext {
    JavaVM* vm = nullptr;
    jobject callback = nullptr;  // GlobalRef to a WebzenNative.ResultCallback
};

void InvokeCallback(int success, const char* jsonPayload, void* userData) {
    auto* context = static_cast<DispatchCallbackContext*>(userData);
    JNIEnv* env = nullptr;
    const bool needsDetach = context->vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED;
    if (needsDetach) {
        context->vm->AttachCurrentThread(&env, nullptr);
    }

    jclass callbackClass = env->GetObjectClass(context->callback);
    jmethodID onResult = env->GetMethodID(callbackClass, "onResult", "(ZLjava/lang/String;)V");
    jstring jPayload = env->NewStringUTF(jsonPayload);
    env->CallVoidMethod(context->callback, onResult, static_cast<jboolean>(success != 0), jPayload);
    env->DeleteLocalRef(jPayload);
    env->DeleteLocalRef(callbackClass);
    env->DeleteGlobalRef(context->callback);

    if (needsDetach) {
        context->vm->DetachCurrentThread();
    }
    delete context;
}

}  // namespace

extern "C" JNIEXPORT void JNICALL Java_com_webzen_sdk_WebzenNative_nativeDispatchCommand(
    JNIEnv* env, jclass, jstring commandId, jstring jsonPayload, jobject callback) {
    JavaVM* vm = nullptr;
    env->GetJavaVM(&vm);

    auto* context = new DispatchCallbackContext{.vm = vm, .callback = env->NewGlobalRef(callback)};

    const char* commandIdChars = env->GetStringUTFChars(commandId, nullptr);
    const char* payloadChars = env->GetStringUTFChars(jsonPayload, nullptr);
    Webzen_DispatchCommand(commandIdChars, payloadChars, &InvokeCallback, context);
    env->ReleaseStringUTFChars(commandId, commandIdChars);
    env->ReleaseStringUTFChars(jsonPayload, payloadChars);
}
