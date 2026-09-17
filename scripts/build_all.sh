#!/usr/bin/env bash
# Best-effort "build everything this machine can build". Android and iOS
# still need their own SDKs installed (NDK / Xcode); Windows can't be
# cross-built from here at all -- run scripts/build_windows.ps1 on Windows.
# CI (.github/workflows/) runs each platform script on its own dedicated
# runner OS instead of relying on this script.
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

"$ROOT_DIR/scripts/build_host.sh"

if [[ "${ANDROID_NDK_HOME:-}" != "" ]]; then
    "$ROOT_DIR/scripts/build_android.sh"
else
    echo "Skipping Android: ANDROID_NDK_HOME is not set."
fi

if [[ "$(uname -s)" == "Darwin" ]] && command -v xcodebuild >/dev/null; then
    "$ROOT_DIR/scripts/build_ios.sh"
else
    echo "Skipping iOS: not running on macOS with Xcode."
fi

echo "Skipping Windows: run scripts/build_windows.ps1 on a Windows machine."
