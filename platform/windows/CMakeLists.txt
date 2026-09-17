# Single webzen_core.dll (+ import .lib) containing the common core, the
# Windows adapter and the C ABI. Linking the webzen::core OBJECT library
# bundles its object files directly into this DLL, so every REGISTER_COMMAND
# translation unit is always present -- no whole-archive trick needed here
# because, unlike the iOS static-lib case, we control this link step fully
# (nothing downstream re-links this DLL's contents through another archive).
add_library(webzen_core SHARED
    adapter/src/windows_adapter.cpp
    adapter/src/windows_bootstrap.cpp
)

target_include_directories(webzen_core PRIVATE adapter/include)
target_link_libraries(webzen_core PRIVATE webzen::core Crypt32 Advapi32 Shell32 Ole32 Version)
target_compile_definitions(webzen_core PRIVATE WEBZEN_CORE_BUILD)

set_target_properties(webzen_core PROPERTIES
    RUNTIME_OUTPUT_NAME webzen_core
    ARCHIVE_OUTPUT_NAME webzen_core
)
