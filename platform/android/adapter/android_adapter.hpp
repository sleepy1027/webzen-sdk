#pragma once

#include "core/platform_adapter.hpp"

#include <jni.h>

namespace webzen::platform::android {

// Delegates to a Kotlin singleton (com.webzen.sdk.WebzenSecureStorage,
// EncryptedSharedPreferences-backed) via JNI rather than reimplementing
// Android Keystore access in C++ -- the Keystore/EncryptedSharedPreferences
// APIs are Java/Kotlin-only. Purely an implementation detail of this .so:
// nothing outside it (Unity, Unreal, or their Kotlin/JNI wrappers) ever
// calls into com.webzen.sdk directly -- see jni_bridge.cpp.
class AndroidSecureStorage : public adapter::ISecureStorage {
public:
    bool Set(std::string_view key, std::string_view value) override;
    std::optional<std::string> Get(std::string_view key) override;
    void Remove(std::string_view key) override;
};

class AndroidDeviceInfo : public adapter::IDeviceInfo {
public:
    [[nodiscard]] std::string DeviceId() const override;
    [[nodiscard]] std::string OsVersion() const override;
    [[nodiscard]] std::string AppVersion() const override;
};

// Cached by JNI_OnLoad (jni_bridge.cpp) so the adapters above can attach to
// the calling thread and call back into Kotlin from any thread, including
// the SDK's own io_context thread.
void InitJniCache(JavaVM* vm, JNIEnv* env);
JNIEnv* AttachCurrentThread();

}  // namespace webzen::platform::android
