# Webzen SDK

A cross-platform game SDK for Unity and Unreal: Auth, Billing/IAP, in-app
WebView, Push, Crash reporting (Sentry) and MMP integrations (Airbridge,
Singular, Firebase). The business logic behind all of that lives once, in a
shared C++20 core, instead of being duplicated across separate Android/iOS/
Windows libraries.

See [`.claude/skills/game-sdk-core-architecture/SKILL.md`](.claude/skills/game-sdk-core-architecture/SKILL.md)
for the full architecture rationale and conventions. The short version:

```
Unreal / Unity
   └─ thin interface + bridge (bridge/unity, bridge/unreal)
        └─ common core (core/, C++20)
              ├─ Auth / Billing / WebView / Push / Crash / MMP domain logic
              └─ OS adapter interfaces (core/include/webzen/adapter)
                    ├─ Android adapter (platform/android)
                    ├─ iOS adapter     (platform/ios)
                    └─ Windows adapter (platform/windows)
```

**Status:** the build toolchain (CMake + presets, Gradle/AAR packaging,
xcframework packaging, Windows DLL packaging, Unity/Unreal bridges, CI) is
complete end to end. Auth is the one domain actually migrated into the core
so far, as a working reference for how the rest follow — see
[Migration status](#migration-status).

## Repository layout

| Path | What it is |
|---|---|
| `core/` | The shared C++20 business logic. Platform-agnostic; never include a platform header here. |
| `platform/android/` | JNI adapter + the Gradle module that wraps `libwebzen_core.so` into `WebzenSDK.aar`. |
| `platform/ios/` | Objective-C++ adapter; packaged into `WebzenSDK.xcframework` by `scripts/build_ios.sh`. |
| `platform/windows/` | Windows (DPAPI-backed) adapter; builds `webzen_core.dll`. |
| `platform/host/` | Linux/macOS dev-only adapter, for running the core + tests without any mobile SDK installed. |
| `bridge/unity/` | Unity package (UPM). `Plugins/` is where the built `.aar`/`.xcframework`/`.dll` land. |
| `bridge/unreal/` | Unreal plugin. `ThirdParty/WebzenCore/` is where the built artifacts land. |
| `scripts/` | One build script per target: `build_host.sh`, `build_android.sh`, `build_ios.sh`, `build_windows.ps1`, plus `build_all.sh`. |
| `docs/BUILDING.md` | Full build instructions and prerequisites per platform. |

## Quickstart

```sh
# Sanity-check the toolchain on any Linux/macOS machine, no mobile SDKs needed:
./scripts/build_host.sh
```

That configures and builds the core with CMake, runs the unit tests, and
runs a smoke-test binary that exercises the full pipeline (platform adapter
→ `Sdk::Initialize` → command dispatch → `AuthService` → `HttpClient`).

For the actual shippable artifacts:

```sh
ANDROID_NDK_HOME=/path/to/ndk VCPKG_ROOT=/path/to/vcpkg ./scripts/build_android.sh   # -> WebzenSDK.aar
VCPKG_ROOT=/path/to/vcpkg ./scripts/build_ios.sh                                     # -> WebzenSDK.xcframework (macOS only)
$env:VCPKG_ROOT = "C:\path\to\vcpkg"; .\scripts\build_windows.ps1                     # -> webzen_core.dll/.lib (Windows only)
```

Each script stages its artifact directly into both `bridge/unity/Plugins/`
and `bridge/unreal/ThirdParty/WebzenCore/`, so you can drop either `bridge/`
folder into a game project afterward. See [`docs/BUILDING.md`](docs/BUILDING.md)
for prerequisites, CI, and the `-force_load` requirement on iOS.

## Migration status

Per the architecture skill, features move into the common core one at a
time; existing per-OS implementations aren't assumed to exist yet in this
repository, and everything not listed below is not implemented:

- [x] Auth (`core/include/webzen/auth`) — reference implementation, exercised
      end to end by `core/tests` and `platform/host`.
- [ ] Billing / IAP
- [ ] In-app WebView
- [ ] Push
- [ ] Crash reporting (Sentry)
- [ ] MMP (Airbridge, Singular, Firebase)

## License

MIT — see [LICENSE](LICENSE).
