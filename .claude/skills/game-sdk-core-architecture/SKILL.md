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
* 키 이름은 손으로 문자열을 적지 말고, 아래처럼 컴파일타임 변환 매크로로 멤버 이름에서 자동 계산한다:

```cpp
// utils/json_naming.hpp
template <fixed_string Name>
consteval auto to_snake_case();  // PascalCase -> snake_case, 컴파일타임 계산

#define SDK_FIELD(T, member) to_snake_case<#member>(), &T::member
```

```cpp
template <>
struct glz::meta<AuthTokenDto> {
    using T = AuthTokenDto;
    static constexpr auto value = glz::object(
        SDK_FIELD(T, UserId),       // -> "user_id"
        SDK_FIELD(T, AccessToken),  // -> "access_token"
        SDK_FIELD(T, ExpiresAt)     // -> "expires_at"
    );
};
```

새 DTO를 추가할 땐 이 패턴을 따른다. 키 이름을 직접 문자열로 하드코딩하지 않는다 (오타/불일치 방지).

### 비동기 처리 (async/await 대체)

* C++20 코루틴(`co_await`/`co_return`) 문법을 쓰되, 실행기(executor)는 asio(`asio::awaitable<T>` + `co_spawn`)를 사용한다. 표준 라이브러리엔 실행기가 없어서 asio 없이는 코루틴 문법만으로 실제 비동기 흐름을 돌릴 수 없다.
* Kotlin coroutine이나 Swift async/await에 익숙한 팀원이라면, `co_await`로 순차적으로 읽히는 코드 스타일이 비슷하게 나온다는 걸 참고해도 된다.
* cppcoro는 쓰지 않는다 (유지보수가 활발하지 않음, 실험적 성격).

### 동적 커맨드 생성 (리플렉션 대체)

* C++엔 런타임 리플렉션이 없다 (C++26에 표준 리플렉션(P2996)이 들어갈 예정이지만 아직 실험적 컴파일러 포크에서만 부분 구현돼 있어서, 실제 NDK/Xcode/MSVC 툴체인에 반영되려면 몇 년은 더 걸린다 — 지금 쓸 수 있는 선택지가 아니다).
* 대신 self-registering factory 패턴을 쓴다: 각 커맨드 클래스가 정적 초기화 시점에 매크로로 자기 자신을 레지스트리(`map<string/ID, factory_fn>`)에 등록하고, 런타임엔 문자열/ID로 그 맵에서 찾아 인스턴스를 만든다.

```cpp
class LoginCommand : public Command {
    // ...
};
REGISTER_COMMAND("login", LoginCommand);
```

이 패턴은 Unreal 자체의 `UCLASS`/`UPROPERTY` + UHT(코드생성) 방식과 개념적으로 같으니, 팀에 이미 익숙한 사고방식이라고 보면 된다. 새 커맨드를 추가할 때 이 매크로를 빠뜨리지 않는다.

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
