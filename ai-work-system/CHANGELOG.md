# Changelog

버전은 `VERSION` 파일과 맞춘다. 스킬·에이전트·지침을 바꿀 때마다 한 줄 이상 남긴다.
형식: `## x.y.z — YYYY-MM-DD` 아래 `- [파일] 변경 내용 (이유)`

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
