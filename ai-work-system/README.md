# AI 업무 시스템 키트

"나만의 AI 업무 시스템 4단계"(컨텍스트 → 업무 분해 → 스킬화 → 검증·자동화)를
어느 Claude에든 가져다 붙일 수 있게 만든 스킬 묶음.

## 구성

| 스킬 | 역할 |
|---|---|
| `work-system-setup` | 4단계로 업무 시스템을 구축하는 가이드 + 템플릿 (about-me, 업무 인벤토리, SKILL 템플릿, 평가표) |
| `smart-task` | 모든 업무에 쓰는 기본 프로토콜: 목표·완료조건 → 맥락 → 계획 → 실행 → 자기검증 → 보고 |
| `lessons-learned` | 피드백·실패를 LESSONS.md / about-me / SKILL.md에 반영해 같은 실수를 반복하지 않게 함 |

```
ai-work-system/
├── skills/
│   ├── work-system-setup/
│   │   ├── SKILL.md
│   │   └── templates/ (about-me.md, task-inventory.md, SKILL.template.md, eval-rubric.md)
│   ├── smart-task/SKILL.md
│   └── lessons-learned/SKILL.md
└── scripts/ (package.sh, install.sh)
```

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
채운 about-me 내용은 `~/.claude/CLAUDE.md` 또는 저장소 `CLAUDE.md`에 넣는다.

### API / Agent SDK / 다른 LLM
SKILL.md는 frontmatter(name, description)가 붙은 마크다운일 뿐이라,
본문을 system prompt에 그대로 넣어도 동작한다. Agent SDK에서는 `.claude/skills/`에 두면 자동 인식된다.

## 처음 쓰는 순서
1. Claude에게 "work-system-setup으로 내 업무 시스템 만들자"라고 요청 → 1단계(about-me)부터 진행
2. 반복 업무 1~3개를 골라 스킬로 만든다 (`templates/SKILL.template.md`)
3. 평가표로 5~10회 테스트 → 실패는 `lessons-learned`로 반영
4. 통과선을 넘은 업무만 위험도에 맞춰 자동화(자동 / 검토 후 발송 / 항상 승인)
