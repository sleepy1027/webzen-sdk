# Built by Gradle's externalNativeBuild (see aar/build.gradle.kts), once per
# ABI. The resulting libwebzen_core.so is what Gradle packages into
# jni/<abi>/libwebzen_core.so inside the .aar -- Unity/Unreal call straight
# into this .so's exported C API (core/sdk_c_api.h); nothing wraps it in a
# public Kotlin facade any more (see jni_bridge.cpp).
add_library(webzen_core SHARED
    adapter/android_adapter.cpp
    adapter/jni_bridge.cpp
)

target_compile_definitions(webzen_core PRIVATE WEBZEN_CORE_BUILD)

find_library(ANDROID_LOG_LIB log)
target_link_libraries(webzen_core PRIVATE webzen::core webzen::auth ${ANDROID_LOG_LIB})

set_target_properties(webzen_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
