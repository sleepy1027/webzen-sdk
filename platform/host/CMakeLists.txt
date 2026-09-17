# Linux/macOS "host" adapter: not shipped to games. It exists so the common
# core, the command dispatch pipeline and the C ABI can be exercised end to
# end (unit tests + this demo binary) on any dev machine or CI runner without
# needing an NDK, Xcode or MSVC install.

add_library(webzen_host_adapter STATIC
    src/host_adapter.cpp
)
target_include_directories(webzen_host_adapter PUBLIC include)
target_link_libraries(webzen_host_adapter PUBLIC webzen::core)

add_executable(webzen_host_demo src/main.cpp)
target_link_libraries(webzen_host_demo PRIVATE webzen_host_adapter webzen::core)
