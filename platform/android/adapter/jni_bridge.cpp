// The only JNI entry point this .so exposes. Nothing calls into
// com.webzen.sdk from outside the .so any more (see the architecture
// discussion in the game-sdk-core-architecture skill on why per-engine
// Kotlin facades were removed): Unity uses [DllImport("webzen_core")]
// straight against libwebzen_core.so, and Unreal resolves the same C
// symbols via dlopen/dlsym (see WebzenSDKSubsystem.cpp) since UBT can't
// easily link against a .so bundled inside an .aar at build time. JNI here
// exists purely so this .so's own adapter code (android_adapter.cpp) can
// call back into Kotlin for Keystore access.

#include "core/sdk.hpp"
#include "platform/android/adapter/android_adapter.hpp"

#include <jni.h>

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
    webzen::Sdk::Instance().SetPlatformAdapter(
        webzen::adapter::PlatformAdapter{.SecureStorage = &g_secureStorage, .DeviceInfo = &g_deviceInfo});

    return JNI_VERSION_1_6;
}
