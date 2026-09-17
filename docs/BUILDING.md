# Building

## Host (Linux/macOS) — no mobile SDK required

Prerequisites: CMake 3.24+, Ninja, a C++20 compiler, libcurl development
headers (`sudo apt-get install libcurl4-openssl-dev` on Debian/Ubuntu, or
whatever your distro/macOS's package manager calls it).

```sh
./scripts/build_host.sh
```

This is the one to run after any change under `core/` or a feature module
(`auth/`, ...). It configures with the `host` CMake preset, builds
`webzen_tests` and `webzen_host_demo`, runs the test suite, then runs the
demo binary as an end-to-end smoke test of the whole pipeline: platform
adapter → `Webzen_DispatchCommand("core.initialize", ...)` →
`Webzen_DispatchCommand("auth.login", ...)` → `CommandRegistry` →
`AuthService` → `HttpClient`, against a deliberately unreachable URL so a
graceful `login_failed` — not a crash or `unknown_command` — is the
expected, successful result.

Dependencies (glaze, asio, Catch2) are fetched via CMake `FetchContent` on
every platform including this one, so no vcpkg is needed just to build and
test the core.

## Android — `WebzenSDK.aar`

Prerequisites:
- Android NDK (set `ANDROID_NDK_HOME`)
- [vcpkg](https://github.com/microsoft/vcpkg) (set `VCPKG_ROOT`) — supplies
  libcurl for the `arm64-android` / `arm-neon-android` / `x64-android`
  triplets, per this project's dependency-management convention (see the
  architecture skill: vcpkg or Conan only, never a committed binary).
- `gradle` on `PATH` (or install the Gradle wrapper once with `gradle
  wrapper` inside `platform/android/` — a wrapper jar isn't committed to
  this repo)

```sh
ANDROID_NDK_HOME=/path/to/ndk VCPKG_ROOT=/path/to/vcpkg ./scripts/build_android.sh
```

Gradle's `externalNativeBuild` drives CMake once per ABI
(`arm64-v8a`/`armeabi-v7a`/`x86_64`, from `platform/android/aar/build.gradle.kts`),
so you never invoke the `android-*` CMake presets directly for a normal
build — they exist for IDE integration / manually testing one ABI. The
resulting `.so` files are packaged into `jni/<abi>/libwebzen_core.so` inside
`WebzenSDK.aar` automatically; nothing in this repo ships a raw `.so`.

The script copies the built `.aar` to both `bridge/unity/Plugins/Android/`
and `bridge/unreal/ThirdParty/WebzenCore/Android/`.

## iOS — `WebzenSDK.xcframework`

macOS + Xcode only. Prerequisites: vcpkg (set `VCPKG_ROOT`) for libcurl on
the `arm64-ios` / `arm64-ios-simulator` / `x64-ios-simulator` triplets.

```sh
VCPKG_ROOT=/path/to/vcpkg ./scripts/build_ios.sh
```

This builds three static `libwebzen_core.a` slices (device arm64, simulator
arm64, simulator x86_64), `lipo`s the two simulator slices into one, and
runs `xcodebuild -create-xcframework` to produce a single
`WebzenSDK.xcframework` containing both a device and a simulator library,
with `core/sdk_c_api.h` and `core/export.h` bundled as its public headers
(the only public surface — see "Why there's no per-OS bridge class" in the
README; everything else in `core/`/`auth/` is an internal implementation
detail neither engine bridge includes directly).

**Required in the consuming Xcode project: link with `-force_load`.**
Commands register themselves in `CommandRegistry` via static initializers
(`REGISTER_COMMAND`, see the architecture skill) — nothing in the app calls
`LoginCommand`'s constructor directly, since it's looked up by string
(`"auth.login"`) at runtime. Xcode's linker treats that as "unreferenced"
and drops it from a normal static-library link, so every command silently
stops working. Both engine bridges already set this for you:
- Unity: `bridge/unity/Editor/WebzenIOSPostProcessBuild.cs` adds the flag to
  `OTHER_LDFLAGS` on every iOS build.
- Unreal: `bridge/unreal/WebzenSDK_UPL_IOS.xml` adds it via `<linkerFlags>`.

If you're integrating the static library some other way, add
`-force_load <path-to>/WebzenSDK.framework/WebzenSDK` yourself.

The script copies the built `.xcframework` to both
`bridge/unity/Plugins/iOS/` and `bridge/unreal/ThirdParty/WebzenCore/IOS/`,
and stages `core/sdk_c_api.h` + `core/export.h` under
`bridge/unreal/ThirdParty/WebzenCore/include/core/` (the Unreal plugin
needs its own copy of the public C headers since it ships standalone once
copied into a separate Unreal project — kept at `include/core/...` so
`WebzenSDKSubsystem.cpp`'s `#include "core/sdk_c_api.h"` resolves the same
way it does inside this repo).

## Windows — `webzen_core.dll`

Windows + Visual Studio 2022 ("Desktop development with C++") only.
Prerequisites: vcpkg (set `VCPKG_ROOT`) for libcurl on `x64-windows`.

```powershell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
.\scripts\build_windows.ps1
```

Builds `webzen_core.dll` + the `webzen_core.lib` import library and copies
them to `bridge/unity/Plugins/x86_64/` and
`bridge/unreal/ThirdParty/WebzenCore/Win64/` (plus the public C headers into
`bridge/unreal/ThirdParty/WebzenCore/include/core/`, same reasoning as iOS
above).

## CI

`.github/workflows/` builds each target on its own dedicated runner OS:
`ci.yml` (host build + tests, every push/PR), `build-android.yml`
(`ubuntu-latest` + NDK), `build-ios.yml` (`macos-14`), `build-windows.yml`
(`windows-latest`). Each bootstraps its own throwaway vcpkg checkout and
uploads the resulting artifact.

## Adding a new feature module

Follow `auth/` as the template when migrating the next domain (Billing,
WebView, Push, Crash, MMP) out of the legacy per-OS libraries:

1. New top-level directory (`billing/`, `web/`, `push/`, `analytics/`,
   `crashreport/`) with headers and sources side by side — no `include/` vs
   `src/` split.
2. Its own `CMakeLists.txt` building an `OBJECT` library (copy `auth/CMakeLists.txt`
   and rename), linking `webzen::core` and aliased as `webzen::<module>`.
   `add_subdirectory(<module>)` in the root `CMakeLists.txt`.
3. `Request<Feature>`/`Result` payload types deriving from `webzen::Request`
   (`core/request.hpp`) and a `Command` implementation registered with
   `REGISTER_COMMAND("<module>.<action>", ...)`.
4. Link the new module into **every** platform artifact that ships it
   (`platform/android/CMakeLists.txt`, `platform/ios/CMakeLists.txt`,
   `platform/windows/CMakeLists.txt` each need `webzen::<module>` added to
   their `target_link_libraries()` — a module that migrates in but isn't
   linked into the shipped artifact silently never runs).
5. Its own `<module>/tests/` — picked up automatically by `tests/CMakeLists.txt`'s glob.

Nothing in `bridge/unity` or `bridge/unreal` needs to change: both already
call `DispatchCommand`/`Dispatch` generically by command id, so a new
command id just starts working once the module above ships in the native
artifact. Add a typed convenience wrapper (a `RequestFoo` class/struct + a
thin method) on the bridge side only if it's worth the ergonomics —
`WebzenSDK.Dispatch(request, callback)` / `UWebzenSDKSubsystem::Dispatch(...)`
work today with just the command id and hand-built JSON.

## Adding a third-party dependency

Per the architecture skill: vcpkg or Conan only, never a manually committed
binary. This project uses vcpkg (`vcpkg.json`); add new dependencies there
and to the relevant `find_package()`/`target_link_libraries()` calls in
`core/CMakeLists.txt`. Header-only libraries the core already depends on
(glaze, asio) are the one exception, fetched via CMake `FetchContent` in
`core/CMakeLists.txt` so the host build never needs vcpkg — follow that
existing pattern only for genuinely header-only additions; anything with a
real per-platform build (like libcurl) goes through vcpkg instead.
