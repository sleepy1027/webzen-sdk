# Same OBJECT-library pattern as core/ -- see the comment there. Every
# feature module (billing/, web/, push/, analytics/, crashreport/ as they
# migrate in) gets its own directory following this exact template.
file(GLOB WEBZEN_AUTH_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(webzen_module_auth OBJECT ${WEBZEN_AUTH_SOURCES})
add_library(webzen::auth ALIAS webzen_module_auth)

target_link_libraries(webzen_module_auth PUBLIC webzen::core)
target_compile_features(webzen_module_auth PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(webzen_module_auth PRIVATE /W4 /permissive-)
else()
    target_compile_options(webzen_module_auth PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
endif()
