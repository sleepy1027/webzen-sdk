# AI 업무 시스템 키트

"나만의 AI 업무 시스템 4단계"(컨텍스트 → 업무 분해 → 스킬화 → 검증·자동화)를
어느 Claude에든 가져다 붙일 수 있게 만든 스킬 묶음.

## 구성

| 스킬 | 역할 |
|---|---|
| `work-system-setup` | 4단계로 업무 시스템을 구축하는 가이드 + 템플릿 (about-me, 업무 인벤토리, SKILL 템플릿, 평가표) |
| `smart-task` | 모든 업무에 쓰는 기본 프로토콜: 목표·완료조건 → 맥락 → 계획 → 실행 → 자기검증 → 보고 |
| `lessons-learned` | 피드백·실패를 LESSONS.md / INSTRUCTIONS.md / SKILL.md에 반영해 같은 실수를 반복하지 않게 함 |
| `work-setup` | 설치 후 1회: 세션 자료로 지침·팀원 채우기, 보드 생성, 초기 데이터 입력 |
| `team-board` | 팀 업무 보드의 업무·팀원 데이터를 읽고 쓰기, 일정 체크, 실 주간보고 작성 |
| `self-improve` | 시스템 자체를 점검해 개선안 제안 → 승인 항목만 반영 → VERSION·CHANGELOG 갱신, 원본 저장소에서 최신 버전 받기 |

| 에이전트 (`agents/*.md`, Claude Code 서브에이전트) | 역할 |
|---|---|
| `work-reviewer` | 결과물을 넘기기 전 독립 검토 (완료 조건, 사실 대조, 형식) |
| `context-researcher` | 작업 전 맥락 수집, 출처와 함께 요약 |
| `skill-builder` | 반복 업무를 SKILL.md로 만들거나 고침 |
| `system-improver` | 지침·스킬·에이전트를 점검해 개선안(diff) 제안 |

| 파일 | 역할 |
|---|---|
| `AGENTS.md` | 어떤 AI 도구에나 붙는 공통 지침 (Codex·Cursor 등은 그대로, Claude Code는 `CLAUDE.md`에 `@AGENTS.md`) |
| `INSTRUCTIONS.md` | 사용자 지시사항 기록. 작업 전에 항상 읽는다 |
| `VERSION`, `CHANGELOG.md` | 자가 업데이트 이력 |

```
ai-work-system/
├── skills/
│   ├── work-system-setup/
│   │   ├── SKILL.md
│   │   └── templates/ (about-me.md, task-inventory.md, SKILL.template.md, eval-rubric.md)
│   ├── smart-task/SKILL.md
│   ├── lessons-learned/SKILL.md
│   ├── work-setup/ (SKILL.md, templates/)
│   ├── team-board/ (SKILL.md, assets/team-board.html)
│   └── self-improve/SKILL.md
├── agents/ (work-reviewer, context-researcher, skill-builder, system-improver)
├── AGENTS.md, INSTRUCTIONS.md, VERSION, CHANGELOG.md
├── .claude-plugin/ (plugin.json, marketplace.json)
├── install.sh, install.ps1, install.cmd
└── scripts/ (package.sh, update.sh)
```

## 팀 업무 보드 (웹 툴)

원본 `skills/team-board/assets/team-board.html`. 보드는 `work-setup`이 계정마다 새로 만든다 (개인 계정 시험용: https://claude.ai/artifact/R4DWa9bB3NRAEvD1Ty6pZo).

- 보드(드래그로 상태 변경) / 목록(정렬) / 일정(월 달력 + 지연·이번 주·다음 주 마감) / 주간보고 / 팀원 탭
- 데이터는 팀 공유 DB에 실시간 저장. 팀원은 보드 화면의 공유 메뉴에서 초대해야 열 수 있다 (편집하려면 Contributor 이상)
- 주간보고: 주 선택 → 담당자별/프로젝트별 자동 정리 → 추가 이슈·지원 요청 입력 → 복사 / .md 저장 / AI로 다듬기 / 보고 기록 저장
- 다른 Claude에서는 `team-board` 스킬로 "업무 추가해줘", "주간보고 써줘"처럼 쓰면 된다

## 설치 (한 번에)

### 1. 패키지 옮기기
`dist/work-system-<버전>.zip` 하나만 회사 PC로 옮겨서 압축을 푼다. 이 저장소가 있으면 `./scripts/package.sh`로 다시 만들 수 있다.

### 2. 설치 스크립트 실행
- **Windows:** `work-system\install.cmd`를 더블클릭한다. 또는 PowerShell에서 아래를 실행한다.
  ```
  powershell -ExecutionPolicy Bypass -File work-system\install.ps1
  ```
- **macOS / Linux / WSL / Git Bash:**
  ```bash
  bash work-system/install.sh
  ```

스크립트가 하는 일:
1. 패키지를 `~/.claude/work-system/package`로 복사한다.
2. Claude Code 플러그인 `work-system@work-system-local`로 설치한다. `claude` 명령이 없으면 `~/.claude/skills`, `~/.claude/agents`로 직접 복사한다.
3. 작업 시스템 홈 `~/.claude/work-system/`에 AGENTS.md, INSTRUCTIONS.md, LESSONS.md, CONFIG.md를 만든다. 이미 있으면 그대로 둔다.
4. `~/.claude/CLAUDE.md`에 `@~/.claude/work-system/AGENTS.md`를 추가해 모든 세션에 지침을 적용한다.

다시 실행해도 안전하다. 새 버전 zip으로 다시 실행하면 업데이트되고, 개인 설정은 유지된다.

### 3. 초기 설정 (자료가 있는 세션에서)
자료가 들어 있는 작업 폴더에서 Claude Code를 새로 열고 아래를 입력한다.
```
/work-system:work-setup
```
직접 복사 방식으로 설치됐다면 `/work-setup`을 입력한다.

이 단계에서 Claude가 하는 일:
- 세션의 파일과 연결된 Notion·Drive 등을 읽어 내 역할, 팀원, 진행 중 업무, 기존 보고 양식을 찾는다.
- 찾은 내용으로 AGENTS.md를 채운다.
- 회사 계정에 팀 업무 보드를 새로 만든다.
- 팀원과 업무를 보드에 넣는다. 데이터를 쓰기 전에 확인을 받는다.

### claude.ai 웹에서만 쓸 때
`dist/claude-ai/*.zip`을 설정 → 기능 → 스킬에서 하나씩 올린다. AGENTS.md는 프로젝트 지침에 붙여넣는다.

### Codex / Cursor 등
`AGENTS.md`를 프로젝트 루트에 둔다.

## 처음 쓰는 순서
1. Claude에게 "work-system-setup으로 내 업무 시스템 만들자"라고 요청 → 1단계(about-me)부터 진행
2. 반복 업무 1~3개를 골라 스킬로 만든다 (`templates/SKILL.template.md`)
3. 평가표로 5~10회 테스트 → 실패는 `lessons-learned`로 반영
4. 통과선을 넘은 업무만 위험도에 맞춰 자동화(자동 / 검토 후 발송 / 항상 승인)
5. 가끔 "self-improve로 시스템 점검해줘" → 개선안 승인 → 자동 반영 및 버전 기록
