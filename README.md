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
   └─ one call in, one shape out: DispatchCommand(command_id, request_json) -> Result
        └─ one feature module per domain (core/, auth/, C++20)
              └─ OS adapter interfaces (core/platform_adapter.hpp)
                    ├─ Android adapter (platform/android)
                    ├─ iOS adapter     (platform/ios)
                    └─ Windows adapter (platform/windows)
```

Every domain feature is its own top-level directory (`core/` is generic
plumbing, `auth/` is the first migrated domain; `billing/`, `web/`, `push/`,
`analytics/`, `crashreport/` follow the same template as they migrate in).
Each module keeps its headers and sources side by side in one folder — no
`include/` vs `src/` split. There is exactly one entry point into the native
side: `Webzen_DispatchCommand(command_id, request_json, callback)`. Even SDK
bootstrapping is a command (`"core.initialize"`) — there's no separate
`Initialize()`/`Login()` pair of native entry points to keep in sync.
Every request is a plain struct with its own `request_id` field (no shared
base class — see the architecture skill's JSON section for why) and every
result is the same `Result` shape (`core/request.hpp`); a request's
`request_id` is echoed back on its
result so a caller with several requests in flight can tell which callback
goes with which response, without the C ABI needing a per-call `user_data`.

**Status:** the build toolchain (CMake + presets, Gradle/AAR packaging,
xcframework packaging, Windows DLL packaging, Unity/Unreal bridges, CI) is
complete end to end. Auth is the one domain actually migrated into the core
so far, as a working reference for how the rest follow — see
[Migration status](#migration-status).

## Repository layout

| Path | What it is |
|---|---|
| `core/` | Generic plumbing shared by every domain: command dispatch, HTTP+retry, JSON naming, the Sdk facade, the C ABI. Platform-agnostic; never include a platform header here. |
| `auth/` | The Auth domain (reference migration) — `RequestLogin`, `AuthService`, `LoginCommand`. |
| `platform/android/` | JNI adapter + the Gradle module that wraps `libwebzen_core.so` into `WebzenSDK.aar`. |
| `platform/ios/` | Objective-C++ adapter; packaged into `WebzenSDK.xcframework` by `scripts/build_ios.sh`. |
| `platform/windows/` | Windows (DPAPI-backed) adapter; builds `webzen_core.dll`. |
| `platform/host/` | Linux/macOS dev-only adapter, for running the core + tests without any mobile SDK installed. |
| `bridge/unity/` | Unity package (UPM). One `WebzenSDK.Dispatch(Request, callback)` entry point, no per-OS bridge classes — see below. `Plugins/` is where the built `.aar`/`.xcframework`/`.dll` land. |
| `bridge/unreal/` | Unreal plugin. Same one-entry-point shape (`UWebzenSDKSubsystem::Dispatch`). `ThirdParty/WebzenCore/` is where the built artifacts land. |
| `scripts/` | One build script per target: `build_host.sh`, `build_android.sh`, `build_ios.sh`, `build_windows.ps1`, plus `build_all.sh`. |
| `docs/BUILDING.md` | Full build instructions and prerequisites per platform. |

Each feature module's own `tests/` subfolder holds its unit tests
(`core/tests`, `auth/tests`, ...); `tests/CMakeLists.txt` aggregates all of
them into one `webzen_tests` binary.

### Why there's no per-OS bridge class in Unity or Unreal

Android, iOS and Windows all export the *identical* C ABI
(`core/sdk_c_api.h`'s `Webzen_DispatchCommand`) from their native artifact —
Android's inside `libwebzen_core.so` (bundled in `WebzenSDK.aar`), iOS's
statically linked into the app, Windows' in `webzen_core.dll`. So the engine
bridges call it directly and identically everywhere:
- **Unity**: `[DllImport("webzen_core")]` (or `"__Internal"` on iOS, since
  static linking has no loadable library name) resolves straight to the
  native artifact — no Java/Kotlin or Objective-C wrapper API in between.
- **Unreal**: iOS/Windows link the artifact at build time and call the C
  API directly; Android's `.so` lives inside an AAR UBT doesn't build-time
  link against, so it's resolved once via `dlopen`/`dlsym` instead (see
  `WebzenSDKSubsystem.cpp`).

The Kotlin (`platform/android/aar`) and Objective-C++ (`platform/ios`) code
that remains is purely an *internal implementation detail* of each native
artifact — calling Keystore/Keychain APIs that only exist in Java/Objective-C
— never a public surface either engine bridge talks to.

## Quickstart

```sh
# Sanity-check the toolchain on any Linux/macOS machine, no mobile SDKs needed:
./scripts/build_host.sh
```

That configures and builds the core with CMake, runs the unit tests, and
runs a smoke-test binary that exercises the full pipeline: platform adapter
→ `Webzen_DispatchCommand("core.initialize", ...)` → `Webzen_DispatchCommand
("auth.login", ...)` → `CommandRegistry` → `AuthService` → `HttpClient`.

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

- [x] Auth (`auth/`) — reference implementation, exercised end to end by
      `auth/tests` and `platform/host`.
- [ ] Billing / IAP
- [ ] In-app WebView
- [ ] Push
- [ ] Crash reporting (Sentry)
- [ ] MMP (Airbridge, Singular, Firebase)

## License

MIT — see [LICENSE](LICENSE).
