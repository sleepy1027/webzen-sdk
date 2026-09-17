cmake_minimum_required(VERSION 3.24)

project(webzen_sdk
    VERSION 0.1.0
    DESCRIPTION "Webzen cross-platform game SDK (common C++ core + Android/iOS/Windows adapters)"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Single output directory so every platform build drops its artifact in a
# predictable place for the packaging scripts under scripts/.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(WEBZEN_BUILD_TESTS "Build the core unit test suite" ON)
option(WEBZEN_BUILD_PLATFORM "Build the current platform's adapter + wrapped artifact (.so/.framework/.dll)" ON)

# Common business logic. Never platform-specific: see
# .claude/skills/game-sdk-core-architecture/SKILL.md before adding anything
# here that only makes sense on one OS.
add_subdirectory(core)

if(WEBZEN_BUILD_PLATFORM)
    if(ANDROID)
        add_subdirectory(platform/android)
    elseif(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
        add_subdirectory(platform/ios)
    elseif(WIN32)
        add_subdirectory(platform/windows)
    else()
        # Linux/macOS host build: no shippable engine artifact, but the host
        # adapter lets the core + unit tests build and run anywhere, which is
        # what CI uses to validate the toolchain on every commit.
        add_subdirectory(platform/host)
    endif()
endif()

if(WEBZEN_BUILD_TESTS AND NOT ANDROID AND NOT IOS AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    enable_testing()
    add_subdirectory(core/tests)
endif()
