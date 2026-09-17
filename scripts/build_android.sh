#!/usr/bin/env bash
# Builds libwebzen_core.so for every ABI in platform/android/aar/build.gradle.kts
# (via Gradle's externalNativeBuild -> CMake -> android-arm64-v8a/armeabi-v7a/
# x86_64 presets under the hood) and packages them into one .aar, then stages
# that .aar for both engine bridges. Requires ANDROID_NDK_HOME and a `gradle`
# on PATH (see docs/BUILDING.md).
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

: "${ANDROID_NDK_HOME:?Set ANDROID_NDK_HOME to your NDK install}"
: "${VCPKG_ROOT:?Set VCPKG_ROOT to your vcpkg checkout (provides libcurl for the android-* triplets)}"

cd "$ROOT_DIR/platform/android"
gradle :aar:assembleRelease "$@"

AAR_PATH="$ROOT_DIR/platform/android/aar/build/outputs/aar/aar-release.aar"
UNITY_DEST="$ROOT_DIR/bridge/unity/Plugins/Android"
UNREAL_DEST="$ROOT_DIR/bridge/unreal/ThirdParty/WebzenCore/Android"

mkdir -p "$UNITY_DEST" "$UNREAL_DEST"
cp "$AAR_PATH" "$UNITY_DEST/WebzenSDK.aar"
cp "$AAR_PATH" "$UNREAL_DEST/WebzenSDK.aar"

echo "Android AAR staged:"
echo "  $UNITY_DEST/WebzenSDK.aar"
echo "  $UNREAL_DEST/WebzenSDK.aar"
