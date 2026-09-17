# Minimal iOS toolchain file for the Webzen SDK core.
#
# Selects SDK/arch/sysroot from WEBZEN_IOS_PLATFORM, set by the matching
# CMakePresets.json entry:
#   OS64              -> real devices (arm64)
#   SIMULATORARM64    -> simulator on Apple Silicon hosts
#   SIMULATOR64       -> simulator on Intel hosts
#
# Kept intentionally small (no watchOS/tvOS/bitcode handling) because this
# project only ships an iOS phone/tablet target. If that changes, prefer
# swapping in the community ios-cmake toolchain rather than growing this file
# ad hoc.

if(NOT CMAKE_SYSTEM_NAME)
    set(CMAKE_SYSTEM_NAME iOS)
endif()

set(WEBZEN_IOS_PLATFORM "OS64" CACHE STRING "OS64 | SIMULATORARM64 | SIMULATOR64")

if(WEBZEN_IOS_PLATFORM STREQUAL "OS64")
    set(CMAKE_OSX_SYSROOT iphoneos)
    set(CMAKE_OSX_ARCHITECTURES arm64)
    set(CMAKE_SYSTEM_PROCESSOR arm64)
elseif(WEBZEN_IOS_PLATFORM STREQUAL "SIMULATORARM64")
    set(CMAKE_OSX_SYSROOT iphonesimulator)
    set(CMAKE_OSX_ARCHITECTURES arm64)
    set(CMAKE_SYSTEM_PROCESSOR arm64)
elseif(WEBZEN_IOS_PLATFORM STREQUAL "SIMULATOR64")
    set(CMAKE_OSX_SYSROOT iphonesimulator)
    set(CMAKE_OSX_ARCHITECTURES x86_64)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
else()
    message(FATAL_ERROR "Unknown WEBZEN_IOS_PLATFORM '${WEBZEN_IOS_PLATFORM}' (expected OS64, SIMULATORARM64 or SIMULATOR64)")
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum iOS version")
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# Static libs only: the .xcframework is assembled from per-platform static
# archives by scripts/build_ios.sh, not from a dylib with an install_name.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
