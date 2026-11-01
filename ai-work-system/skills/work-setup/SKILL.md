---
name: work-setup
description: work-system 패키지를 설치한 뒤 처음 한 번 실행하는 초기 설정. 이 세션에 들어 있는 자료(프로젝트 파일, 문서, 연결된 Notion·Drive·메일·캘린더)를 읽어 내 업무 지침(AGENTS.md)과 팀원 목록을 초안으로 만들고, 회사 계정에 팀 업무 보드를 새로 만들고, 모든 세션에 지침이 적용되도록 연결한다. "work-setup", "업무 시스템 설치", "초기 설정", "세팅 시작", "보드 새로 만들어줘" 같은 요청에 사용. 이미 설정된 상태에서 다시 실행하면 빠진 부분만 채운다.
---

# work-system 초기 설정 (한 번에)

이 스킬의 기본 폴더(로드 시 "Base directory for this skill"로 표시됨)를 `$SKILL`이라 한다.
- 템플릿: `$SKILL/templates/` (AGENTS.md, INSTRUCTIONS.md, LESSONS.md, CONFIG.md)
- 보드 원본: `$SKILL/../team-board/assets/team-board.html`
- 설치 대상(작업 시스템 홈): `~/.claude/work-system/` (Windows: `%USERPROFILE%\.claude\work-system\`)

진행 상황은 단계마다 한 줄로 알린다. 사용자 확인이 필요한 곳은 아래에 **[확인]**으로 표시했다. 확인 질문은 한 번에 모아서 한다.

## 0. 현재 상태 점검
- `~/.claude/work-system/` 존재 여부와 그 안의 파일, `CONFIG.md`의 보드 URL을 확인한다.
- 이미 있는 파일은 **덮어쓰지 않는다**. 빠진 파일만 템플릿에서 만들고, 채워진 항목은 그대로 둔다.
- 결과를 표로 보여준다: 항목 / 상태(있음·없음·비어 있음) / 이번에 할 일.

## 1. 세션 자료에서 정보 모으기
이 세션에서 접근할 수 있는 자료를 훑어서 아래를 찾는다. 찾은 것은 **출처와 함께** 정리하고, 추측으로 채우지 않는다.
- 현재 작업 폴더: README, 문서 폴더, 회의록·보고서·업무 목록처럼 보이는 파일(md, txt, docx, xlsx, csv, pdf). 많으면 파일명 기준으로 관련 있어 보이는 것 위주로 최대 20개만 읽는다.
- 연결된 커넥터가 있으면(Notion, Google Drive, 캘린더, 메일 등) 최근 업무 문서·주간보고·회의를 검색한다. 없으면 건너뛴다.
- 찾을 것:
  - 내 역할·팀명·보고 대상(실), 자주 쓰는 도구, 용어집
  - 팀원 이름과 역할
  - 진행 중인 업무(제목, 담당자, 프로젝트, 마감일, 상태)
  - 기존 주간보고 양식이나 예시 (있으면 보고 양식으로 사용)
- 자료가 거의 없으면 사용자에게 5개 이내로 묻는다: 역할, 팀명, 팀원(이름·역할), 보고 요일, 결과물 선호.

## 2. 작업 시스템 홈 만들기
1. `~/.claude/work-system/` 폴더를 만든다.
2. 템플릿 4개 중 없는 것을 복사한다.
3. `AGENTS.md`의 "1. 나에 대해"를 1단계에서 찾은 내용으로 채운다. 확인되지 않은 칸은 비워 두고 "(확인 필요)"라고 적는다.
4. `CONFIG.md`에 팀명, 보고 대상·요일, 설치 버전(`$SKILL/../../.claude-plugin/plugin.json`의 version, 없으면 비움), 오늘 날짜를 적는다.
5. 기존 주간보고 양식을 찾았으면 `~/.claude/work-system/REPORT_TEMPLATE.md`로 저장한다 (team-board 스킬이 이 양식을 우선 사용).

## 3. 모든 세션에 지침 연결 [확인]
- `~/.claude/CLAUDE.md`에 아래 한 줄이 없으면 추가한다. 파일이 없으면 만든다. 기존 내용은 건드리지 않는다.
  ```
  @~/.claude/work-system/AGENTS.md
  ```
- 추가 전에 "모든 Claude Code 세션에 이 지침이 적용됩니다"라고 알리고 확인받는다.

## 4. 팀 업무 보드 만들기 [확인]
CONFIG.md에 보드 URL이 이미 있으면 이 단계를 건너뛴다(보드를 새로 만들어 달라고 한 경우는 예외).
1. `Artifact` 도구가 있으면 `$SKILL/../team-board/assets/team-board.html`을 그대로 publish한다.
   - `capabilities`: `{"db": {}, "user": {}, "sample": {}, "downloads": true}`
   - `icon`: `kanban`, `description`: "팀원과 내 업무를 보드·목록·일정으로 관리하고 실 주간보고를 자동으로 만드는 공유 업무 보드"
   - 원본 HTML은 수정하지 않는다. 팀명 등은 데이터로 넣는다.
2. 받은 URL을 `CONFIG.md`의 "팀 업무 보드 URL"에 적는다.
3. `Artifact` 도구가 없는 환경이면: 이 HTML을 claude.ai 대화에 첨부해 "이 파일을 capabilities db, user, sample, downloads로 Artifact publish해줘"라고 요청하라고 안내하고, 받은 URL을 알려주면 CONFIG.md에 적겠다고 한다.

## 5. 보드에 초기 데이터 넣기 [확인]
1단계에서 찾은 팀원·업무가 있으면 표로 보여주고, 넣을 항목을 확인받은 뒤 `ArtifactData` `batch` 한 번으로 쓴다.
- `members/<새 id>`: `{name, role, order, createdAt}` (order는 0부터)
- `tasks/<새 id>`: `{title, memberId, project, status, priority, start, due, progress, note, doneAt, createdAt, updatedAt}`
  - 상태를 모르면 `todo`, 우선순위를 모르면 `mid`, 날짜를 모르면 `null`
  - 완료된 업무는 `status: "done"`, `doneAt`은 알 수 있으면 그 날짜, 아니면 넣지 않는다(완료 업무는 굳이 옮기지 않아도 됨)
- 실제 자료에 있는 사람·업무만 넣는다. 예시 데이터를 만들어 넣지 않는다.
- 쓴 뒤 `list`로 한 번 읽어 개수가 맞는지 확인한다.

## 6. 마무리 보고
- 만든/바꾼 파일 목록과 보드 URL
- 비어 있어 사용자가 채워야 할 칸(about-me의 확인 필요 항목 등)
- 팀원 초대 방법: 보드 화면 오른쪽 위 공유 메뉴 → 팀원 초대(편집하려면 Contributor 이상)
- 바로 써볼 문장 3개: "이번 주 마감 업무 보여줘", "OO에게 '△△' 업무 추가, 마감 금요일", "이번 주 실 주간보고 초안 써줘"

## 하지 말 것
- 기존 `~/.claude/CLAUDE.md`나 work-system 파일을 덮어쓰지 않는다.
- 확인 없이 보드에 데이터를 쓰거나 보드를 새로 만들지 않는다.
- 회사 자료의 내용을 보드 외의 외부 서비스로 보내지 않는다.
