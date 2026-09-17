# One test binary aggregating every module's own tests/ subfolder (tests
# live next to the code they test -- core/tests, auth/tests, and so on for
# each feature module as it migrates in -- not in a separate top-level tree).
include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

file(GLOB_RECURSE WEBZEN_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/core/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/auth/tests/*.cpp"
)

add_executable(webzen_tests ${WEBZEN_TEST_SOURCES})
target_link_libraries(webzen_tests PRIVATE webzen::core webzen::auth Catch2::Catch2WithMain)
target_compile_features(webzen_tests PRIVATE cxx_std_20)

include(CTest)
include(Catch)
catch_discover_tests(webzen_tests)
