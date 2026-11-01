# 사용자 지시사항 기록

이 키트를 수정하거나 확장하는 에이전트는 작업 전에 이 파일을 먼저 읽고 모든 항목을 지킨다.
새로운 지시가 생기면 아래에 번호를 이어서 추가한다. (지시 원문 + 해석 + 반영 위치)

## 1. 영상 기반 AI 업무 시스템 세팅 (2026-11-01)
- 원문: "이거 보고 더 ai 업무를 더 똑똑하게 만들 수 있는 세팅을 만들어줘. 나중에 다른 클로드에도 붙이게 skill.md나 다른 임포트 가능한 형태로"
- 참고 영상: 챗대리의 AI연구소 「클로드는 이렇게 쓰는 겁니다: 나만의 'AI 업무 시스템' 4단계 구축법」 https://www.youtube.com/watch?v=3Me5BJgsG7E
  - 주의: 영상 자막은 확인하지 못했고 제목과 같은 주제의 자료로 4단계를 구성했다. 영상 요약이 들어오면 단계 내용을 맞춰 고친다.
- 반영: `skills/work-system-setup`, `skills/smart-task`, `skills/lessons-learned`, `scripts/package.sh`, `scripts/install.sh`

## 2. SKILL.md + agent.md 형식 모두 제공 (2026-11-01)
- 원문: "skill.md agent.md"
- 해석: 스킬(SKILL.md) 외에 에이전트 지침 형식도 제공. 여러 도구 공통 `AGENTS.md`와 Claude Code 서브에이전트 `agents/*.md` 둘 다 만든다.
- 반영: `AGENTS.md`, `agents/work-reviewer.md`, `agents/context-researcher.md`, `agents/skill-builder.md`, `agents/system-improver.md`

## 3. 지시사항은 md에 저장해서 참고 (2026-11-01)
- 원문: "두번째 지시한 건 md에 저장하여 참고할 것"
- 해석: 사용자 지시를 대화에만 두지 말고 md 파일에 남겨 이후 작업에서 참고한다. 이 키트를 쓰는 에이전트에게도 같은 규칙을 적용한다.
- 반영: 이 파일(`INSTRUCTIONS.md`), `AGENTS.md` 7절, `skills/lessons-learned`

## 4. 자가 업데이트·개선 제안 기능 (2026-11-01)
- 원문: "자가 업데이트 기능이나 개선 제안을 할 수 있을 것"
- 반영: `skills/self-improve`, `agents/system-improver.md`, `scripts/update.sh`, `CHANGELOG.md`, `AGENTS.md` 8절

## 5. 업무 관리 툴 (2026-11-01)
- 원문: "내 업무들을 노션처럼 관리할 수 있도록 필요하다면 툴을 만들어줘. 팀원들과 내 일을 추가하고, 일정을 체크하고 주마다 실에 보고함"
- 요구사항: 나 + 팀원 업무 추가/관리, 일정(마감) 체크, 매주 실(부서) 보고서 작성
- 반영: (진행 중 — 형태 결정 후 기록)
