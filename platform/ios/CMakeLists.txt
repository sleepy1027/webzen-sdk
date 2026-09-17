# Produces a single, flat static webzen_core.a per platform (device /
# simulator arm64 / simulator x86_64) -- linking the webzen::core/webzen::auth
# OBJECT libraries below bundles their object files directly into this
# archive rather than nesting more .a files inside it. scripts/build_ios.sh
# then merges the three into one .xcframework.
#
# That flat .a is still just a static archive from the *game's* Xcode
# project's point of view, though, and nothing in an app calls
# LoginCommand's constructor directly (CommandRegistry finds it by string at
# runtime) -- so Xcode's linker will drop it as "unreferenced" unless the
# game links this framework with -force_load. docs/BUILDING.md and both
# engine bridges (bridge/unity's iOS postprocessor, bridge/unreal's UPL) call
# this out; it is not something this CMake project can fix on its own,
# because that link step happens outside of it.
#
# There's no Objective-C wrapper class any more -- Unity's [DllImport
# ("__Internal")] and Unreal's direct C++ link both call
# core/sdk_c_api.h's Webzen_DispatchCommand straight into this archive.
add_library(webzen_core STATIC
    adapter/ios_adapter.mm
    adapter/ios_bootstrap.mm
)

find_library(SECURITY_FRAMEWORK Security REQUIRED)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
find_library(UIKIT_FRAMEWORK UIKit REQUIRED)
target_link_libraries(webzen_core PUBLIC webzen::core webzen::auth PRIVATE ${SECURITY_FRAMEWORK} ${FOUNDATION_FRAMEWORK} ${UIKIT_FRAMEWORK})

set_target_properties(webzen_core PROPERTIES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
)
