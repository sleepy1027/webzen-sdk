#!/usr/bin/env bash
# Builds webzen_core for device + both simulator slices and packages them
# into one WebzenSDK.xcframework, staged for both engine bridges. Must run
# on macOS with Xcode installed; requires VCPKG_ROOT (provides libcurl for
# the ios-* triplets).
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

: "${VCPKG_ROOT:?Set VCPKG_ROOT to your vcpkg checkout}"

for preset in ios-device ios-simulator-arm64 ios-simulator-x86_64; do
    cmake --preset "$preset"
    cmake --build --preset "$preset" --target webzen_core --config Release
done

find_lib() {
    find "$ROOT_DIR/build/$1" -name libwebzen_core.a -print -quit
}

DEVICE_LIB="$(find_lib ios-device)"
SIM_ARM64_LIB="$(find_lib ios-simulator-arm64)"
SIM_X86_64_LIB="$(find_lib ios-simulator-x86_64)"

STAGE="$ROOT_DIR/build/ios-xcframework-stage"
rm -rf "$STAGE"
mkdir -p "$STAGE"

# One universal simulator slice (arm64 Apple Silicon + x86_64 Intel) --
# xcframework wants at most one library per platform+environment.
lipo -create "$SIM_ARM64_LIB" "$SIM_X86_64_LIB" -output "$STAGE/libwebzen_core_simulator.a"

HEADERS_DIR="$STAGE/headers"
mkdir -p "$HEADERS_DIR"
cp -R "$ROOT_DIR/core/include/webzen" "$HEADERS_DIR/"
cp "$ROOT_DIR/platform/ios/adapter/include/WebzenIOSSDK.h" "$HEADERS_DIR/"

XCFRAMEWORK_OUT="$ROOT_DIR/build/WebzenSDK.xcframework"
rm -rf "$XCFRAMEWORK_OUT"
xcodebuild -create-xcframework \
    -library "$DEVICE_LIB" -headers "$HEADERS_DIR" \
    -library "$STAGE/libwebzen_core_simulator.a" -headers "$HEADERS_DIR" \
    -output "$XCFRAMEWORK_OUT"

for dest in "$ROOT_DIR/bridge/unity/Plugins/iOS" "$ROOT_DIR/bridge/unreal/ThirdParty/WebzenCore/IOS"; do
    mkdir -p "$dest"
    rm -rf "${dest:?}/WebzenSDK.xcframework"
    cp -R "$XCFRAMEWORK_OUT" "$dest/WebzenSDK.xcframework"
done

# Same public C headers as scripts/build_windows.ps1 stages -- the Unreal
# plugin ships standalone once copied into a separate project, so it can't
# reach back into this repo's core/include.
UNREAL_INCLUDE_DEST="$ROOT_DIR/bridge/unreal/ThirdParty/WebzenCore/include/webzen"
mkdir -p "$UNREAL_INCLUDE_DEST"
cp "$ROOT_DIR/core/include/webzen/sdk_c_api.h" "$ROOT_DIR/core/include/webzen/export.h" "$UNREAL_INCLUDE_DEST/"

echo "iOS xcframework staged:"
echo "  $ROOT_DIR/bridge/unity/Plugins/iOS/WebzenSDK.xcframework"
echo "  $ROOT_DIR/bridge/unreal/ThirdParty/WebzenCore/IOS/WebzenSDK.xcframework"
echo
echo "Reminder: the consuming Xcode project MUST link this with -force_load"
echo "(both engine bridges already set this -- see docs/BUILDING.md) or every"
echo "REGISTER_COMMAND command will silently fail to run."
