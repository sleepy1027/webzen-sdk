#pragma once

// C ABI export macro for the symbols that cross into JNI (Android), a
// directly-linked call (iOS/Windows via Unity DllImport or the Unreal
// bridge) and P/Invoke. Everything else in the core is plain C++ and never
// needs this.
#if defined(_WIN32)
    #if defined(WEBZEN_CORE_BUILD)
        #define WEBZEN_API __declspec(dllexport)
    #else
        #define WEBZEN_API __declspec(dllimport)
    #endif
#else
    #define WEBZEN_API __attribute__((visibility("default")))
#endif
