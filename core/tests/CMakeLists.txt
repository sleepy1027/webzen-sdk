include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")

add_executable(webzen_core_tests
    test_json_naming.cpp
    test_command_registry.cpp
    test_auth_service.cpp
)

target_link_libraries(webzen_core_tests PRIVATE webzen::core Catch2::Catch2WithMain)
target_compile_features(webzen_core_tests PRIVATE cxx_std_20)

include(CTest)
include(Catch)
catch_discover_tests(webzen_core_tests)
