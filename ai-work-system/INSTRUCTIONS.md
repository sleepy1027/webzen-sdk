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
- 결정: Notion이 아닌 전용 웹 툴(claude.ai Artifact)로 만든다. 주간보고는 기본 양식(□ 금주 실적 / □ 차주 계획 / □ 이슈 및 리스크 / □ 지원 요청)으로 한다.
- 반영: `skills/team-board/assets/team-board.html` (원래 `tools/`) → https://claude.ai/artifact/R4DWa9bB3NRAEvD1Ty6pZo (보드·목록·일정·주간보고·팀원 탭, 팀 공유 DB, AI로 다듬기, .md 저장, 보고 기록), `skills/team-board`(다른 Claude가 보드 데이터를 읽고 쓰는 방법)

## 6. 회사 PC용 한 번에 설치 패키지 (2026-11-01)
- 원문: "회사 계정으로 만든 다른 PC에서 데이터를 넣어놓은 새 세션에 사용할 예정이야. 그러니까 패키지 형태로 깔 수 있게 한 방에 만들어 주어."
- 해석: 개인 계정에 묶인 것(보드 URL) 없이, zip 하나 + 설치 명령 한 번으로 설치되고, 자료가 있는 새 세션에서 초기 설정 한 번으로 지침·보드·데이터까지 갖춰지게 한다. 회사 PC는 Windows일 가능성이 높아 PowerShell 설치도 지원한다.
- 반영: `.claude-plugin/`(플러그인+로컬 마켓플레이스), `install.sh`/`install.ps1`/`install.cmd`, `skills/work-setup`(세션 자료 기반 초기 설정), 개인 설정을 `~/.claude/work-system/`로 분리(업데이트해도 유지), team-board 스킬은 CONFIG.md에서 보드 URL을 읽음, `scripts/package.sh`가 `dist/work-system-<버전>.zip` 생성

## 7. 기존 회사 환경과 덮어쓰기 없이 섞이기 (2026-11-01)
- 원문: "기존 회사 계정에서도 스킬 앱이나 이런 것 파일들이 이미 있을 텐데 얘랑 덮어쓰기 안 되고 자연스럽게 섞일 수 있게."
- 해석: 설치·업데이트·제거가 기존 스킬·에이전트·지침 파일을 절대 덮어쓰거나 지우지 않는다. 목적이 겹치면 기존 것을 우선하고 work-system은 빈 곳만 채우며, 어떤 걸 쓸지는 규칙 파일로 정한다.
- 반영: install.sh/.ps1(ws- 접두사, installed.txt, CLAUDE.md 백업), uninstall.*, `templates/INTEGRATION.md`, AGENTS.md 0절, work-setup 0.5단계, self-improve·smart-task·team-board 설명

## 8. 답변은 요약이 기본 (2026-11-01)
- 원문: "너는 왜 대답을 요구하면 논문을 쓰냐? 한 번에 보기가 어렵잖아. 이거 반복되니까 스킬 MD에도 넣어두는 거 알지? 내가 자세히 더 달라고 얘기하기 전에는 요약해서 얘기를 해줘."
- 해석: 모든 답변 기본값을 요약(결론 1~2줄 + 핵심 3~5줄)으로. 세부는 "자세히" 요청 시에만. 반복 지적이므로 지침 파일에 고정.
- 반영: AGENTS.md 1-1절(저장소·템플릿), smart-task 6단계, templates/INSTRUCTIONS.md 3번
