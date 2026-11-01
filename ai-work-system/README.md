# AI 업무 시스템 키트

"나만의 AI 업무 시스템 4단계"(컨텍스트 → 업무 분해 → 스킬화 → 검증·자동화)를
어느 Claude에든 가져다 붙일 수 있게 만든 스킬 묶음.

## 구성

| 스킬 | 역할 |
|---|---|
| `work-system-setup` | 4단계로 업무 시스템을 구축하는 가이드 + 템플릿 (about-me, 업무 인벤토리, SKILL 템플릿, 평가표) |
| `smart-task` | 모든 업무에 쓰는 기본 프로토콜: 목표·완료조건 → 맥락 → 계획 → 실행 → 자기검증 → 보고 |
| `lessons-learned` | 피드백·실패를 LESSONS.md / INSTRUCTIONS.md / SKILL.md에 반영해 같은 실수를 반복하지 않게 함 |
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
│   ├── team-board/SKILL.md
│   └── self-improve/SKILL.md
├── tools/team-board.html
├── agents/ (work-reviewer, context-researcher, skill-builder, system-improver)
├── AGENTS.md, INSTRUCTIONS.md, VERSION, CHANGELOG.md
└── scripts/ (package.sh, install.sh, update.sh)
```

## 팀 업무 보드 (웹 툴)

https://claude.ai/artifact/R4DWa9bB3NRAEvD1Ty6pZo · 원본 `tools/team-board.html`

- 보드(드래그로 상태 변경) / 목록(정렬) / 일정(월 달력 + 지연·이번 주·다음 주 마감) / 주간보고 / 팀원 탭
- 데이터는 팀 공유 DB에 실시간 저장. 팀원은 보드 화면의 공유 메뉴에서 초대해야 열 수 있다 (편집하려면 Contributor 이상)
- 주간보고: 주 선택 → 담당자별/프로젝트별 자동 정리 → 추가 이슈·지원 요청 입력 → 복사 / .md 저장 / AI로 다듬기 / 보고 기록 저장
- 다른 Claude에서는 `team-board` 스킬로 "업무 추가해줘", "주간보고 써줘"처럼 쓰면 된다

## 붙이는 방법

### claude.ai / Claude 데스크톱 앱
1. `./scripts/package.sh` → `dist/<스킬명>.zip` 생성
2. 설정 → 기능(Capabilities) → 스킬 → 업로드에서 zip을 하나씩 올린다 (코드 실행 기능이 켜져 있어야 함)
3. `templates/about-me.md`를 채워서 프로필 선호 설정이나 프로젝트 지침/지식에 넣는다

### Claude Code
전역 설치 (모든 프로젝트에서 사용):
```bash
./ai-work-system/scripts/install.sh
```
특정 저장소에만 설치:
```bash
./ai-work-system/scripts/install.sh /path/to/repo
```
스킬은 `.claude/skills/`, 에이전트는 `.claude/agents/`에 복사된다.
`AGENTS.md`(나에 대해 부분을 채워서)와 `INSTRUCTIONS.md`를 프로젝트 루트에 두고, `CLAUDE.md`에 `@AGENTS.md`를 넣는다.

최신 버전으로 업데이트 (키트가 git 저장소 안에 있을 때):
```bash
./ai-work-system/scripts/update.sh
```

### Codex / Cursor / Gemini CLI 등
`AGENTS.md`를 프로젝트 루트에 둔다. 스킬 파일은 필요할 때 읽도록 경로를 AGENTS.md에 적어둔다.

### claude.ai 에이전트 지침
`AGENTS.md`를 프로젝트 지침에 붙여넣고, `INSTRUCTIONS.md`는 프로젝트 지식에 올린다.

### API / Agent SDK / 다른 LLM
SKILL.md는 frontmatter(name, description)가 붙은 마크다운일 뿐이라,
본문을 system prompt에 그대로 넣어도 동작한다. Agent SDK에서는 `.claude/skills/`에 두면 자동 인식된다.

## 처음 쓰는 순서
1. Claude에게 "work-system-setup으로 내 업무 시스템 만들자"라고 요청 → 1단계(about-me)부터 진행
2. 반복 업무 1~3개를 골라 스킬로 만든다 (`templates/SKILL.template.md`)
3. 평가표로 5~10회 테스트 → 실패는 `lessons-learned`로 반영
4. 통과선을 넘은 업무만 위험도에 맞춰 자동화(자동 / 검토 후 발송 / 항상 승인)
5. 가끔 "self-improve로 시스템 점검해줘" → 개선안 승인 → 자동 반영 및 버전 기록
