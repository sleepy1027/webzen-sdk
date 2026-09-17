include(FetchContent)

# Header-only deps are vendored through FetchContent everywhere so the exact
# same versions are used on host, Android, iOS and Windows regardless of
# whether the outer build is driven by vcpkg or not. libcurl is the one
# exception (find_package below) because every platform already ships/vendors
# its own TLS-linked curl (system curl on host, vcpkg port on Android/iOS/
# Windows) and vendoring it here would fight that.
FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG        v8.0.0
    GIT_SHALLOW    TRUE
)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-30-2
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glaze)

# asio has no CMakeLists.txt of its own (it ships autotools/bare headers), so
# it's wired up as a plain interface target instead of FetchContent_MakeAvailable.
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()
add_library(asio INTERFACE)
target_include_directories(asio SYSTEM INTERFACE "${asio_SOURCE_DIR}/asio/include")
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

file(GLOB_RECURSE WEBZEN_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# OBJECT (not STATIC): every platform artifact -- the Android .so, the
# Windows .dll, and the flattened .a fed into the iOS .xcframework -- must
# contain every REGISTER_COMMAND translation unit unconditionally. A plain
# STATIC archive lets the linker drop "unreferenced" object files (nothing
# calls LoginCommand's constructor directly -- CommandRegistry finds it by
# string at runtime), silently dropping commands. Consumers link this target
# directly with target_link_libraries(); CMake then always includes every
# object file rather than archiving-and-selectively-pulling them.
add_library(webzen_core_static OBJECT ${WEBZEN_CORE_SOURCES})
add_library(webzen::core ALIAS webzen_core_static)

target_include_directories(webzen_core_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(webzen_core_static
    PUBLIC
        asio
        Threads::Threads
        # Public because DTO headers like auth_types.hpp include
        # <glaze/glaze.hpp> directly for their glz::meta specialization.
        glaze::glaze
    PRIVATE
        CURL::libcurl
)

target_compile_features(webzen_core_static PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(webzen_core_static PRIVATE /W4 /permissive-)
else()
    # -Wno-missing-field-initializers: this codebase leans on designated
    # initializers with default member initializers filling the rest
    # (Result{.Success = false}), which is intentional, not an oversight.
    target_compile_options(webzen_core_static PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
endif()
