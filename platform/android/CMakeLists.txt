# Built by Gradle's externalNativeBuild (see aar/build.gradle.kts), once per
# ABI. The resulting libwebzen_core.so is what Gradle packages into
# jni/<abi>/libwebzen_core.so inside the .aar -- Unity/Unreal never see a
# raw .so, only the .aar produced by aar/.
add_library(webzen_core SHARED
    adapter/src/android_adapter.cpp
    adapter/src/jni_bridge.cpp
)

target_include_directories(webzen_core PRIVATE adapter/include)
target_compile_definitions(webzen_core PRIVATE WEBZEN_CORE_BUILD)

find_library(ANDROID_LOG_LIB log)
target_link_libraries(webzen_core PRIVATE webzen::core ${ANDROID_LOG_LIB})

set_target_properties(webzen_core PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
