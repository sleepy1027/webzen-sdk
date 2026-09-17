# WebzenNative's external fun declarations and WebzenSecureStorage/
# WebzenDeviceInfo's @JvmStatic methods are only referenced from native code
# (jni_bridge.cpp / android_adapter.cpp) via JNI, so R8 sees no Java/Kotlin
# caller and will otherwise strip them.
-keep class com.webzen.sdk.WebzenNative { *; }
-keep class com.webzen.sdk.WebzenNative$* { *; }
-keep class com.webzen.sdk.WebzenSecureStorage { *; }
-keep class com.webzen.sdk.WebzenDeviceInfo { *; }
-keep class com.webzen.sdk.WebzenContextProvider { *; }

# Public API surface used by Unity (AndroidJavaObject, reflection-based) and
# by the Unreal Java glue generated from WebzenSDK_UPL_Android.xml.
-keep class com.webzen.sdk.WebzenSDK { *; }
-keep class com.webzen.sdk.WebzenSDK$* { *; }
