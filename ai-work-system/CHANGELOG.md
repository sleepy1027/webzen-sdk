# Changelog

버전은 `VERSION` 파일과 맞춘다. 스킬·에이전트·지침을 바꿀 때마다 한 줄 이상 남긴다.
형식: `## x.y.z — YYYY-MM-DD` 아래 `- [파일] 변경 내용 (이유)`

## 0.5.1 — 2026-11-01
- [AGENTS.md 1-1, smart-task] 답변 기본값을 요약으로 (사용자 반복 지적)

## 0.5.0 — 2026-11-01
- [설치] 기존 스킬·에이전트를 덮어쓰지 않음: 이름이 겹치면 `ws-<이름>`으로 설치, 설치 파일은 installed.txt에 기록해 그것만 업데이트·제거
- [설치] CLAUDE.md 수정 전 백업, 맨 끝에 한 줄만 추가
- [uninstall.sh / .ps1 / .cmd] work-system이 설치한 것만 제거, 개인 설정은 보존(--purge로 삭제)
- [INTEGRATION.md] 기존 환경과의 우선순위·라우팅·충돌 결정 기록, AGENTS.md가 불러옴
- [work-setup] 0.5단계: 기존 스킬·에이전트·지침 조사 → 겹침·충돌 찾기 → 기존 우선 통합안 → INTEGRATION.md
- [AGENTS.md 0절] 기존 지침 우선, 빈 곳만 채움

## 0.4.0 — 2026-11-01
- [패키지] Claude Code 플러그인(work-system) + 로컬 마켓플레이스로 구성, zip 하나로 배포
- [install.sh / install.ps1 / install.cmd] 한 번에 설치 (플러그인 설치, 실패 시 직접 복사, 작업 시스템 홈 생성, CLAUDE.md 연결)
- [skills/work-setup] 세션 자료를 읽어 지침·팀원·업무를 채우고 계정별 보드를 만드는 초기 설정
- [개인 설정 분리] ~/.claude/work-system/ (AGENTS, INSTRUCTIONS, LESSONS, CONFIG) — 업데이트해도 유지
- [team-board] 보드 URL을 CONFIG.md에서 읽음, HTML을 스킬 assets로 이동

## 0.3.0 — 2026-11-01
- [tools/team-board.html] 팀 업무 보드 웹 툴 추가 (팀원·업무 관리, 일정 체크, 실 주간보고 자동 생성)
- [skills/team-board] 보드 데이터 연동 스킬 추가

## 0.2.0 — 2026-11-01
- [AGENTS.md] 여러 도구 공통 에이전트 지침 추가
- [agents/*] Claude Code 서브에이전트 4종 추가 (work-reviewer, context-researcher, skill-builder, system-improver)
- [INSTRUCTIONS.md] 사용자 지시사항 기록 파일 추가
- [skills/self-improve, scripts/update.sh] 자가 업데이트·개선 제안 기능 추가

## 0.1.0 — 2026-11-01
- 스킬 3종(work-system-setup, smart-task, lessons-learned)과 package/install 스크립트
