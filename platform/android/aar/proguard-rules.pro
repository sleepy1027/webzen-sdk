# WebzenSecureStorage/WebzenDeviceInfo's @JvmStatic methods are only
# referenced from native code (android_adapter.cpp) via JNI, so R8 sees no
# Java/Kotlin caller and will otherwise strip them. WebzenContextProvider is
# only referenced from AndroidManifest.xml, same story.
-keep class com.webzen.sdk.WebzenSecureStorage { *; }
-keep class com.webzen.sdk.WebzenDeviceInfo { *; }
-keep class com.webzen.sdk.WebzenContextProvider { *; }
