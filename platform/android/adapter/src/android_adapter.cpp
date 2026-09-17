#include "webzen/adapter/android_adapter.hpp"

#include <string>

namespace webzen::platform::android {

namespace {

JavaVM* g_vm = nullptr;

jclass g_secureStorageClass = nullptr;
jmethodID g_secureStorageSet = nullptr;
jmethodID g_secureStorageGet = nullptr;
jmethodID g_secureStorageRemove = nullptr;

jclass g_deviceInfoClass = nullptr;
jmethodID g_deviceId = nullptr;
jmethodID g_osVersion = nullptr;
jmethodID g_appVersion = nullptr;

}  // namespace

// Called once from JNI_OnLoad (jni_bridge.cpp), on the thread the JVM calls
// it on -- FindClass only reliably resolves app classes from a JVM-attached
// thread with the app's classloader in scope, which a later background
// thread (e.g. the SDK's own io_context thread) is not guaranteed to have.
// Caching classes/method ids as GlobalRefs here sidesteps that.
void InitJniCache(JavaVM* vm, JNIEnv* env) {
    g_vm = vm;

    jclass secureStorageLocal = env->FindClass("com/webzen/sdk/WebzenSecureStorage");
    g_secureStorageClass = static_cast<jclass>(env->NewGlobalRef(secureStorageLocal));
    g_secureStorageSet = env->GetStaticMethodID(g_secureStorageClass, "set", "(Ljava/lang/String;Ljava/lang/String;)Z");
    g_secureStorageGet = env->GetStaticMethodID(g_secureStorageClass, "get", "(Ljava/lang/String;)Ljava/lang/String;");
    g_secureStorageRemove = env->GetStaticMethodID(g_secureStorageClass, "remove", "(Ljava/lang/String;)V");

    jclass deviceInfoLocal = env->FindClass("com/webzen/sdk/WebzenDeviceInfo");
    g_deviceInfoClass = static_cast<jclass>(env->NewGlobalRef(deviceInfoLocal));
    g_deviceId = env->GetStaticMethodID(g_deviceInfoClass, "deviceId", "()Ljava/lang/String;");
    g_osVersion = env->GetStaticMethodID(g_deviceInfoClass, "osVersion", "()Ljava/lang/String;");
    g_appVersion = env->GetStaticMethodID(g_deviceInfoClass, "appVersion", "()Ljava/lang/String;");
}

JNIEnv* AttachCurrentThread() {
    JNIEnv* env = nullptr;
    if (g_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        g_vm->AttachCurrentThread(&env, nullptr);
    }
    return env;
}

namespace {
std::string JStringToStd(JNIEnv* env, jstring value) {
    if (!value) return {};
    const char* chars = env->GetStringUTFChars(value, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    env->DeleteLocalRef(value);
    return result;
}
}  // namespace

bool AndroidSecureStorage::Set(std::string_view key, std::string_view value) {
    JNIEnv* env = AttachCurrentThread();
    jstring jKey = env->NewStringUTF(std::string(key).c_str());
    jstring jValue = env->NewStringUTF(std::string(value).c_str());
    const jboolean ok = env->CallStaticBooleanMethod(g_secureStorageClass, g_secureStorageSet, jKey, jValue);
    env->DeleteLocalRef(jKey);
    env->DeleteLocalRef(jValue);
    return ok == JNI_TRUE;
}

std::optional<std::string> AndroidSecureStorage::Get(std::string_view key) {
    JNIEnv* env = AttachCurrentThread();
    jstring jKey = env->NewStringUTF(std::string(key).c_str());
    auto* jValue = static_cast<jstring>(env->CallStaticObjectMethod(g_secureStorageClass, g_secureStorageGet, jKey));
    env->DeleteLocalRef(jKey);
    if (!jValue) {
        return std::nullopt;
    }
    return JStringToStd(env, jValue);
}

void AndroidSecureStorage::Remove(std::string_view key) {
    JNIEnv* env = AttachCurrentThread();
    jstring jKey = env->NewStringUTF(std::string(key).c_str());
    env->CallStaticVoidMethod(g_secureStorageClass, g_secureStorageRemove, jKey);
    env->DeleteLocalRef(jKey);
}

std::string AndroidDeviceInfo::DeviceId() const {
    JNIEnv* env = AttachCurrentThread();
    return JStringToStd(env, static_cast<jstring>(env->CallStaticObjectMethod(g_deviceInfoClass, g_deviceId)));
}

std::string AndroidDeviceInfo::OsVersion() const {
    JNIEnv* env = AttachCurrentThread();
    return JStringToStd(env, static_cast<jstring>(env->CallStaticObjectMethod(g_deviceInfoClass, g_osVersion)));
}

std::string AndroidDeviceInfo::AppVersion() const {
    JNIEnv* env = AttachCurrentThread();
    return JStringToStd(env, static_cast<jstring>(env->CallStaticObjectMethod(g_deviceInfoClass, g_appVersion)));
}

void SetApplicationContext(JNIEnv*, jobject) {
    // Reserved for adapters that need the Context directly (e.g. a future
    // Push adapter registering with the system notification manager).
    // SecureStorage/DeviceInfo go through WebzenSecureStorage/WebzenDeviceInfo
    // instead, which already hold the application Context on the Kotlin side.
}

}  // namespace webzen::platform::android
