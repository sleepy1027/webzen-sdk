---
name: game-sdk-core-architecture
description: Unreal/Unity용 크로스플랫폼 게임 SDK(Android/iOS/Windows 네이티브 구현, 인증/빌링(IAP)/인앱웹뷰/푸시/크래시리포팅(Sentry)/MMP(Airbridge·Singular·Firebase) 기능 포함)의 공통 비즈니스 로직을 하나의 C++ 코어로 통합하는 리팩토링 작업에 사용. OS별로 중복 구현된 비즈니스 로직 이관, OS Adapter 인터페이스 설계, Unreal/Unity 브릿지·빌드스크립트 연동, 새 SDK 기능 추가, 이 프로젝트의 C++ 코딩 컨벤션(빌드, HTTP, JSON, 비동기, 커맨드 디스패치)이 관련된 모든 작업에서 반드시 참고할 것. "SDK 코어", "OS adapter", "공통 비즈니스 레이어" 같은 표현이 나오면 이 스킬을 먼저 확인.
---

# 게임 SDK 공통 코어 아키텍처

Unreal/Unity 엔진에 들어가는 게임 SDK를 위한 프로젝트다. 이 스킬은 "왜 이렇게 짜야 하는지"와 "이 프로젝트에서 이미 정한 기술적 결정"을 함께 담아뒀다. 결정된 부분은 이유 없이 바꾸지 말고, 이유가 설명되지 않은 부분은 실제 코드베이스를 먼저 살펴보고 기존 패턴을 따르는 걸 우선한다.

## 프로젝트가 뭘 하는가

Unreal / Unity로 만든 게임에 붙는 SDK로, 다음 기능을 제공한다:

* 인증 (Auth)
* 빌링 / 인앱결제 (IAP)
* 인앱 웹뷰
* 푸시 알림
* 크래시 리포팅 (Sentry)
* MMP 연동 (Airbridge, Singular, Firebase)

각 기능은 Unreal/Unity 레이어가 아니라 네이티브 Android / iOS / Windows에서 직접 구현된다. Unreal/Unity 쪽에는 인터페이스, 네이티브 라이브러리로 연결해주는 얇은 브릿지 코드, 빌드 시 의존성·설정을 자동으로 넣어주는 빌드스크립트만 존재한다.

## AS-IS: 왜 이 리팩토링이 필요한가

지금은 위 기능들의 공통 비즈니스 로직이 Android/iOS/Windows 라이브러리 3세트에 각각 중복 구현돼 있다. 실제 유지보수 비용의 대부분은 OS별 특수성이 아니라 이 공통 비즈니스 로직(요청 재시도, 상태 관리, 에러 처리, 직렬화 등)을 수정하는 일에서 나온다. 즉 버그 하나를 고치거나 로직 하나를 바꾸려면 3곳을 동일하게 고쳐야 하는 게 핵심 문제다.

## TO-BE: 목표 아키텍처

```
Unreal / Unity
   └─ 인터페이스 + 얇은 브릿지 (기존과 동일한 역할, 유지)
        └─ 공통 비즈니스 로직 코어 (C++, 신규)
              ├─ Auth / Billing / WebView / Push / Crash / MMP 도메인 로직
              └─ OS Adapter 인터페이스 (필요한 기능만 정의)
                    ├─ Android Adapter (구현)
                    ├─ iOS Adapter (구현)
                    └─ Windows Adapter (구현)
```

핵심 원칙:

* 비즈니스 로직은 공통 코어에 단 한 번만 존재한다. OS별로 다시 구현하지 않는다. 이게 리팩토링의 존재 이유이므로, 코드를 작성할 때 "이 로직이 OS마다 달라야 할 이유가 있는가?"를 먼저 묻는다. 없다면 공통 코어로.
* OS별로 실제 달라야 하는 부분만 Adapter로 분리한다. Adapter는 "OS가 할 수 있는 모든 것"을 노출하는 게 아니라, 공통 코어가 필요로 하는 기능만 최소한의 인터페이스로 요청해서 쓰는 형태다 (예: HTTP 전송, 시스템 키체인 접근, 네이티브 벤더 SDK 호출). 필요하지 않은 기능을 미리 어댑터에 만들어두지 않는다.
* Unreal/Unity 브릿지와 빌드스크립트는 역할이 바뀐다. 기존엔 "OS 네이티브 라이브러리와 연결"이었다면, 이제는 "미리 빌드된 공통 코어 아티팩트(정적/동적 라이브러리)를 끼워넣는" 역할이 된다. 인터페이스 자체는 최대한 그대로 유지해서 게임 개발자(SDK 사용자) 쪽 API가 깨지지 않게 한다.

## 모듈 구조 & 디스패치 규약 (결정됨)

### 디렉토리: 기능별 모듈, 헤더/소스 같은 폴더

공통 코어는 도메인별로 최상위 디렉토리를 하나씩 둔다: `core/`(범용 배관 — 커맨드 디스패치, HTTP, JSON 네이밍, Sdk 파사드, C ABI), `auth/`, 그리고 이후 `billing/`, `web/`, `push/`, `analytics/`, `crashreport/`가 같은 패턴으로 추가된다. 각 모듈 디렉토리 안에는 헤더(`.hpp`)와 소스(`.cpp`)를 같은 폴더에 둔다 — `include/` vs `src/` 분리는 쓰지 않는다. `#include`는 리포 루트 기준 경로를 그대로 쓴다 (`#include "auth/auth_service.hpp"`, `#include "core/sdk.hpp"`) — 이 include 경로가 모듈 디렉토리와 정확히 일치하므로, 새 모듈을 어디에 둬야 하는지 헷갈릴 일이 없다.

각 모듈은 CMake `OBJECT` 라이브러리로 빌드하고 `webzen::<module>` 별칭을 붙인다 (`auth/CMakeLists.txt` 참고). `STATIC`이 아니라 `OBJECT`인 이유: self-registering factory 패턴(아래 "동적 커맨드 생성") 때문에 최종 플랫폼 아티팩트(Android `.so`, Windows `.dll`, iOS `.xcframework`용 `.a`)가 모든 `REGISTER_COMMAND` 번역 단위를 무조건 포함해야 하는데, 일반 `STATIC` 아카이브는 링커가 "참조되지 않는" 객체 파일을 조용히 빼버릴 수 있다. `OBJECT` 라이브러리를 `target_link_libraries()`로 직접 링크하면 CMake가 객체 파일 전체를 항상 포함시킨다. 단, iOS는 예외다 — 최종적으로 게임 쪽 Xcode 프로젝트가 우리가 만든 `.a`를 또 하나의 서드파티 정적 라이브러리로 취급해 같은 문제가 재발하므로, 그쪽에서 `-force_load`로 링크하는 게 필수다 (`docs/BUILDING.md` 참고).

새 기능 모듈을 이관할 때 반드시 할 일:
1. `<module>/` 디렉토리 + `CMakeLists.txt`(OBJECT 라이브러리, `webzen::core` 링크) + 루트 `CMakeLists.txt`에 `add_subdirectory(<module>)`.
2. `platform/android`, `platform/ios`, `platform/windows` 각각의 `CMakeLists.txt`에 새 `webzen::<module>`을 `target_link_libraries()`로 추가한다 — 이걸 빠뜨리면 코드는 컴파일되지만 실제로 출하되는 아티팩트엔 포함되지 않아 조용히 동작하지 않는다.
3. `<module>/tests/`에 테스트를 두면 `tests/CMakeLists.txt`의 glob이 자동으로 주워간다.

### 네이티브 엔트리 포인트: DispatchCommand 하나로 통일

C ABI(`core/sdk_c_api.h`)는 함수 하나만 노출한다: `Webzen_DispatchCommand(command_id, request_json, callback)`. `Webzen_Initialize(base_url)` 같은 별도의 라이프사이클 진입점을 두지 않는다 — SDK 초기화조차 `"core.initialize"`라는 커맨드로 다룬다 (`core/initialize_command.hpp`). `Sdk`는 첫 `DispatchCommand` 호출 시 내부적으로 io_context 스레드를 lazy하게 시작한다. 새 기능이 "매번 호출 전에 별도로 초기화해야 하는" 진입점을 만들고 싶어지면, 그 대신 커맨드로 표현할 방법을 먼저 찾는다.

콜백에는 `user_data`를 넘기지 않는다. 모든 요청 타입은 `RequestId` 필드를 직접 선언한다 (상속으로 공유하지 않는다 — 아래 JSON 절 참고). `CommandRegistry::Dispatch`가 `webzen::Request`(`core/request.hpp`, `RequestId` 하나만 있는 독립 구조체)로 들어온 JSON을 부분 파싱해 `RequestId`만 뽑아내고, 커맨드 실행 결과(`webzen::Result`)에 그대로 echo해준다 — 도메인 커맨드 코드는 이걸 신경 쓸 필요가 없다. 언어별 바인딩(Kotlin/Obj-C/C#/Unreal C++)은 정적 콜백 트램폴린 하나만 등록하고, `RequestId → 콜백` 맵으로 상관관계를 처리한다. 이렇게 하면 콜백 컨텍스트를 JNI GlobalRef나 Obj-C 블록, C# GCHandle 같은 걸로 FFI 경계 너머까지 들고 다닐 필요가 없다.

콜백이 받는 결과 타입은 항상 하나, `webzen::Result`다 (`RequestId`, `Success`, `ErrorCode`, `ErrorMessage`, 그리고 도메인별 페이로드가 이미 직렬화된 JSON 문자열인 `Data`). 도메인마다 다른 콜백 타입을 만들지 않는다.

### 엔진 브릿지: Unity/Unreal 모두 OS별 브릿지 클래스가 없다

Android/iOS/Windows 아티팩트는 전부 동일한 C ABI(`Webzen_DispatchCommand`)를 노출한다. Android는 `.so`(`.aar` 안에 들어있음), iOS는 정적 링크, Windows는 `.dll` — 노출하는 심볼과 시그니처는 완전히 같다. 그래서 엔진 브릿지 쪽에 "OS별 구현체"를 두지 않는다:

* Unity: `[DllImport("webzen_core")]` 하나로 끝난다 (iOS만 정적 링크라 라이브러리 이름이 `"__Internal"`로 다름 — 이 한 줄 차이 말고는 플랫폼 분기가 없다). `.aar`에 들어있는 `.so`라도 Unity 플레이어가 프로세스에 로드해두므로 SONAME으로 바로 찾아진다.
* Unreal: iOS/Windows는 빌드 타임에 네이티브 아티팩트를 직접 링크해서 C API를 바로 호출한다. Android는 UBT가 AAR 내부의 `.so`를 빌드 타임에 링크하기 애매해서 런타임에 `dlopen`/`dlsym`으로 심볼을 한 번 resolve해서 쓴다 (그 시점엔 이미 로드돼 있어야 하므로, Android에서는 `WebzenContextProvider`가 `System.loadLibrary("webzen_core")`도 같이 해준다).

Kotlin(`platform/android/aar`)이나 Objective-C++(`platform/ios`) 코드는 여전히 존재하지만, 그건 각 네이티브 아티팩트 **내부에서만** 쓰는 구현 디테일이다 (Keystore/Keychain처럼 Java/Obj-C로만 접근 가능한 OS API를 감싸는 용도) — 엔진 브릿지가 호출하는 공개 API가 절대 아니다. "OS별로 브릿지가 왜 필요하지?"라는 질문이 나오면, 답은 대부분 "필요 없다, C ABI를 직접 부르면 된다"이다.

## 기술 스택 (결정됨)

### 언어 / 빌드

* 공통 코어는 C++20 이상.
* 빌드 시스템은 CMake, 플랫폼별 설정은 `CMakePresets.json`에 모아둔다 (Android는 NDK 툴체인 파일, iOS는 Xcode용 toolchain 파일, Windows는 MSVC 네이티브).
* 의존성 관리는 vcpkg 또는 Conan (둘 다 Android/iOS/Windows 타겟을 지원한다). 새 서드파티 라이브러리를 추가할 땐 반드시 이 중 하나로 관리하고, 수동으로 바이너리를 커밋하지 않는다.
* 기존에 있던 "빌드할 때 의존성/설정을 자동으로 추가해주는 빌드스크립트"는 그대로 유지하되, 이제는 공통 코어의 빌드 산출물(라이브러리 파일)을 Unreal/Unity 프로젝트에 끼워넣는 역할로 재정의한다.

### HTTP

* libcurl 사용.
* 단, HTTP 전송 자체를 OS Adapter에 위임할지(회사 프록시/인증서 피닝 등 OS 네트워크 정책을 자연스럽게 상속) 아니면 공통 코어에서 libcurl을 직접 쓸지는 기능별로 판단한다. 확실하지 않으면 기존 코드에 있는 패턴을 따른다.
* 재시도, 타임아웃, 에러 매핑 같은 로직은 어느 쪽을 택하든 공통 코어에 둔다. 이게 이 리팩토링의 핵심이다.

### JSON

* Glaze 사용 (nlohmann::json 아님). 이유: 컴파일타임 리플렉션이라 매크로 부담이 적고, 성능이 훨씬 빠르며, 필드별 키 이름 커스터마이징이 쉽다.
* JSON 키는 서버/클라이언트 관례에 맞춰 snake_case를 쓰고, C++ 멤버는 PascalCase를 쓴다. 약어는 `Url`, `Id`처럼 단일 캐피탈 토큰으로 정규화한다 (`URL`, `ID` 금지) — 이래야 케이스 변환이 항상 예측 가능하다.
* 키 이름은 손으로 문자열을 적지 말고, Glaze의 자동 리플렉션 + `glz::snake_case`로 자동 계산한다. DTO가 상속 없는 순수 aggregate 구조체(공개 멤버만, 커스텀 생성자 없음)면 이 한 줄이면 끝이다:

```cpp
struct AuthTokenDto {
    std::string UserId;
    std::string AccessToken;
    std::int64_t ExpiresAt = 0;
};

template <>
struct glz::meta<AuthTokenDto> : glz::snake_case {};
// -> JSON: {"user_id":..., "access_token":..., "expires_at":...}
```

  멤버 이름 그대로 자동으로 리플렉션되고 snake_case로 변환되므로, 멤버를 추가/삭제해도 `glz::meta` 쪽엔 손댈 게 없다. (예전엔 `SDK_FIELD(T, Member)`를 멤버마다 손으로 나열하는 자체 매크로를 썼는데, Glaze가 이미 이 기능을 내장하고 있어서 걷어냈다 — 자체 구현판은 멤버 하나 추가할 때마다 별도로 등록해줘야 했고, 그걸 잊어도 컴파일 에러가 안 나는 게 문제였다.)
* **DTO에 상속을 쓰지 않는다.** Glaze v8.0.0의 자동 리플렉션은 베이스 클래스가 있는 타입에서 컴파일 에러를 낸다 (직접 확인함 — 이후 버전에서 고쳐질 수도 있지만 지금은 그렇다). `RequestLogin`, `RequestInitialize`처럼 공통 필드(`RequestId`)가 필요한 타입들은 각자 그 필드를 직접 선언한다 (한 줄 중복). 여러 Request 타입에 걸쳐 공통으로 동작해야 하는 로직(파싱, 디스패치)은 대신 템플릿으로 처리한다 — `core/command_registry.hpp`의 `TypedCommand<TRequest>` 참고. 커맨드를 새로 만들 땐 `Command`를 직접 상속하지 말고 `TypedCommand<TRequest>`를 상속해서 `ExecuteTyped(TRequest)`만 구현한다; JSON 파싱과 에러 처리는 템플릿이 대신 해준다.
* `glz::read`/`glz::read_json` 기본 옵션(`error_on_unknown_keys = true`)은 모르는 키를 만나면 그 자리에서 파싱을 멈춘다 — 그 뒤에 나오는, 그 타입이 원래 알고 있는 필드조차 못 읽는다. 그래서 요청 JSON을 읽는 곳은 전부 `glz::read<glz::opts{.error_on_unknown_keys = false}>`를 명시적으로 쓴다 (`TypedCommand::Execute`, `CommandRegistry::Dispatch`가 이미 그렇게 함). 이 프로젝트에서 한 번 실제로 이 순서 의존성 때문에 `RequestId` correlation이 조용히 깨지는 버그가 났었다 — `glz::read_json`(기본 옵션) 쓰지 않는다.

### 비동기 처리 (async/await 대체)

* C++20 코루틴(`co_await`/`co_return`) 문법을 쓰되, 실행기(executor)는 asio(`asio::awaitable<T>` + `co_spawn`)를 사용한다. 표준 라이브러리엔 실행기가 없어서 asio 없이는 코루틴 문법만으로 실제 비동기 흐름을 돌릴 수 없다.
* Kotlin coroutine이나 Swift async/await에 익숙한 팀원이라면, `co_await`로 순차적으로 읽히는 코드 스타일이 비슷하게 나온다는 걸 참고해도 된다.
* cppcoro는 쓰지 않는다 (유지보수가 활발하지 않음, 실험적 성격).

### 동적 커맨드 생성 (리플렉션 대체)

* C++엔 런타임 리플렉션이 없다 (C++26에 표준 리플렉션(P2996)이 들어갈 예정이지만 아직 실험적 컴파일러 포크에서만 부분 구현돼 있어서, 실제 NDK/Xcode/MSVC 툴체인에 반영되려면 몇 년은 더 걸린다 — 지금 쓸 수 있는 선택지가 아니다).
* 대신 self-registering factory 패턴을 쓴다: 각 커맨드 클래스가 정적 초기화 시점에 매크로로 자기 자신을 레지스트리(`map<string/ID, factory_fn>`)에 등록하고, 런타임엔 문자열/ID로 그 맵에서 찾아 인스턴스를 만든다.

```cpp
class LoginCommand : public TypedCommand<RequestLogin> {
protected:
    asio::awaitable<CommandOutcome> ExecuteTyped(RequestLogin request) override { /* ... */ }
};
REGISTER_COMMAND("auth.login", LoginCommand);
```

이 패턴은 Unreal 자체의 `UCLASS`/`UPROPERTY` + UHT(코드생성) 방식과 개념적으로 같으니, 팀에 이미 익숙한 사고방식이라고 보면 된다. 새 커맨드를 추가할 때 이 매크로를 빠뜨리지 않는다. `Command`를 직접 상속하지 않고 `TypedCommand<TRequest>`를 상속하는 이유는 JSON 절 참고 — Request 타입들이 서로 상속 관계가 아니라서, "타입별로 파싱해서 호출"을 공유하는 역할을 상속 대신 템플릿이 맡는다.

## 코드를 작성할 때 판단 순서

새로운 기능을 추가하거나 버그를 고칠 때는 이 순서로 판단한다:

1. 이 로직이 OS마다 달라야 하는가? 아니라면 무조건 공통 코어. 3개 OS 중 하나에서만 먼저 급하게 고치고 나머지를 나중에 맞추는 식의 임시방편을 만들지 않는다.
2. OS별로 정말 달라야 한다면, 공통 코어가 필요로 하는 최소 인터페이스를 Adapter에 정의하고, 각 OS Adapter 구현체에서 채운다. Adapter 인터페이스는 "필요해서 추가하는 것"이지 "있으면 좋을 것 같아서 미리 만드는 것"이 아니다.
3. Unreal/Unity 브릿지 쪽을 건드릴 필요가 있는가? 대부분은 없어야 정상이다 (공통 코어 내부 구현 변경은 브릿지 인터페이스에 영향 주면 안 됨). 브릿지 인터페이스 자체가 바뀌어야 한다면, 그건 SDK 사용자(게임 개발자) 쪽 API가 바뀐다는 뜻이니 더 신중하게 검토한다.
4. 빌드스크립트가 새 의존성/설정을 추가해야 하는가? 새 OS Adapter나 새 서드파티 라이브러리를 추가했다면 빌드스크립트 갱신이 필요한지 확인한다.

## 마이그레이션 시 주의사항

기존 3개 OS 라이브러리를 유지하면서 기능별로 하나씩 공통 코어로 이관하는 점진적 마이그레이션을 기본 전제로 한다 (한 번에 새로 작성하는 방식이 아님). 특정 기능을 이관할 때는:

* 이관 전/후 동작이 동일한지(특히 에러 처리, 재시도 정책) 확인한다.
* 아직 이관되지 않은 다른 기능들과의 의존 관계(예: 인증 토큰을 빌링 로직이 참조하는 경우)를 깨뜨리지 않는다.
* 세 플랫폼 모두에서 빌드/동작이 확인되기 전까지는 기존 OS별 구현을 남겨둔다.

## 참고: 왜 C++를 골랐는가 (배경)

Kotlin Multiplatform은 Android/iOS엔 성숙하지만 Windows(mingwx64) 지원이 약하다 (Sentry의 공식 KMP SDK조차 Windows를 no-op 스텁으로만 제공). .NET NativeAOT는 Windows/iOS는 성숙하지만 Android 지원이 아직 안정화 중이고, 무엇보다 Unreal(순수 C++ 엔진)과 자연스럽게 엮이지 않는다. Rust+UniFFI도 유효한 대안이었지만, 팀 러닝커브를 감안해 C++로 결정했다. 이 배경은 나중에 다른 선택지를 다시 검토할 일이 있으면 참고할 것.
