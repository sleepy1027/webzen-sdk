# Building

## Host (Linux/macOS) — no mobile SDK required

Prerequisites: CMake 3.24+, Ninja, a C++20 compiler, libcurl development
headers (`sudo apt-get install libcurl4-openssl-dev` on Debian/Ubuntu, or
whatever your distro/macOS's package manager calls it).

```sh
./scripts/build_host.sh
```

This is the one to run after any change under `core/`. It configures with
the `host` CMake preset, builds `webzen_core_tests` and `webzen_host_demo`,
runs the test suite, then runs the demo binary as an end-to-end smoke test
of the whole pipeline (platform adapter → `Sdk::Initialize` → command
dispatch → `AuthService` → `HttpClient`, against a deliberately unreachable
URL so a graceful `login_failed` — not a crash or `unknown_command` — is the
expected, successful result).

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
with `webzen/*.h` and `WebzenIOSSDK.h` bundled as its public headers.

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
and stages `webzen/sdk_c_api.h` + `webzen/export.h` under
`bridge/unreal/ThirdParty/WebzenCore/include/webzen/` (the Unreal plugin
needs its own copy of the public C headers since it ships standalone once
copied into a separate Unreal project).

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
`bridge/unreal/ThirdParty/WebzenCore/include/webzen/`, same reasoning as
iOS above).

## CI

`.github/workflows/` builds each target on its own dedicated runner OS:
`ci.yml` (host build + tests, every push/PR), `build-android.yml`
(`ubuntu-latest` + NDK), `build-ios.yml` (`macos-14`), `build-windows.yml`
(`windows-latest`). Each bootstraps its own throwaway vcpkg checkout and
uploads the resulting artifact.

## Adding a third-party dependency

Per the architecture skill: vcpkg or Conan only, never a manually committed
binary. This project uses vcpkg (`vcpkg.json`); add new dependencies there
and to the relevant `find_package()`/`target_link_libraries()` calls in
`core/CMakeLists.txt`. Header-only libraries the core already depends on
(glaze, asio) are the one exception, fetched via CMake `FetchContent` in
`core/CMakeLists.txt` so the host build never needs vcpkg — follow that
existing pattern only for genuinely header-only additions; anything with a
real per-platform build (like libcurl) goes through vcpkg instead.
