#pragma once

// C ABI export macro for the symbols that cross into JNI (Android), the
// Objective-C wrapper (iOS) and P/Invoke (Windows/Unity/Unreal C#). Everything
// else in the core is plain C++ and never needs this.
#if defined(_WIN32)
    #if defined(WEBZEN_CORE_BUILD)
        #define WEBZEN_API __declspec(dllexport)
    #else
        #define WEBZEN_API __declspec(dllimport)
    #endif
#else
    #define WEBZEN_API __attribute__((visibility("default")))
#endif
