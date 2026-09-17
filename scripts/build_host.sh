#!/usr/bin/env bash
# Builds the common core + unit tests on whatever machine you're on (Linux
# or macOS). No NDK/Xcode/MSVC required -- this is the toolchain sanity
# check to run after any core/ change, before touching a platform build.
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

cmake --preset host
cmake --build --preset host -j"$(command -v nproc >/dev/null && nproc || sysctl -n hw.ncpu)"
ctest --preset host --output-on-failure

echo
echo "Smoke test (auth.login against a deliberately unreachable URL, so a"
echo "graceful login_failed -- not a crash or unknown_command -- is success):"
"$ROOT_DIR/build/host/bin/webzen_host_demo"
