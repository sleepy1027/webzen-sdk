#!/usr/bin/env bash
# work-system 한 번에 설치 (macOS / Linux / WSL / Git Bash)
#   bash install.sh
# 기존 스킬·에이전트·지침은 절대 덮어쓰지 않는다.
#   - 플러그인 설치: 이름이 work-system:<이름>으로 분리되어 충돌 없음
#   - 직접 복사(플러그인 불가 시): 같은 이름이 이미 있고 work-system이 설치한 것이 아니면 ws-<이름>으로 설치
#   - 설치한 파일은 ~/.claude/work-system/installed.txt 에 기록 (업데이트·제거 때 이것만 건드림)
#   - CLAUDE.md는 백업 후 한 줄만 추가
set -euo pipefail
SRC="$(cd "$(dirname "$0")" && pwd)"
CLAUDE_HOME="${CLAUDE_CONFIG_DIR:-$HOME/.claude}"
HOME_DIR="$CLAUDE_HOME/work-system"
PKG="$HOME_DIR/package"
MANIFEST="$HOME_DIR/installed.txt"
VER="$(cat "$SRC/VERSION" 2>/dev/null || echo unknown)"
MKT="work-system-local"
TODAY="$(date +%Y%m%d-%H%M%S)"

echo "▶ work-system v$VER 설치"
mkdir -p "$HOME_DIR"
touch "$MANIFEST"

# 0) 기존 환경 요약 (읽기만)
n_sk=0; n_ag=0
[ -d "$CLAUDE_HOME/skills" ] && n_sk=$( (find "$CLAUDE_HOME/skills" -mindepth 2 -maxdepth 2 -name SKILL.md 2>/dev/null || true) | wc -l | tr -d ' ')
[ -d "$CLAUDE_HOME/agents" ] && n_ag=$( (find "$CLAUDE_HOME/agents" -maxdepth 1 -name '*.md' 2>/dev/null || true) | wc -l | tr -d ' ')
echo "  기존 환경: 사용자 스킬 ${n_sk}개, 에이전트 ${n_ag}개 — 그대로 유지합니다."

# 1) 패키지 복사 (설치 위치 고정)
if [ "$SRC" != "$PKG" ]; then
  rm -rf "$PKG.tmp" && mkdir -p "$PKG.tmp"
  (cd "$SRC" && tar --exclude='./dist' --exclude='./.git' -cf - .) | (cd "$PKG.tmp" && tar -xf -)
  rm -rf "$PKG" && mv "$PKG.tmp" "$PKG"
fi

# 2) 플러그인 설치
MODE=""
if command -v claude >/dev/null 2>&1; then
  if claude plugin marketplace list 2>/dev/null | grep -q "$MKT"; then
    claude plugin marketplace update "$MKT" >/dev/null 2>&1 || true
    if claude plugin update "work-system@$MKT" >/dev/null 2>&1 || claude plugin install "work-system@$MKT" >/dev/null 2>&1; then MODE=plugin; fi
  else
    if claude plugin marketplace add "$PKG" >/dev/null 2>&1 && claude plugin install "work-system@$MKT" >/dev/null 2>&1; then MODE=plugin; fi
  fi
fi

owned() { grep -qxF "$1" "$MANIFEST"; }
record() { owned "$1" || echo "$1" >> "$MANIFEST"; }
# frontmatter의 첫 name: 줄만 새 이름으로 바꾼다 (macOS/Linux 공통)
rename_fm() { awk -v nn="$2" '!done && /^name: /{print "name: " nn; done=1; next} {print}' "$1" > "$1.tmp" && mv "$1.tmp" "$1"; }

if [ "$MODE" = plugin ]; then
  echo "  플러그인 설치: work-system@$MKT (스킬 이름은 work-system:<이름>, 기존 스킬과 충돌 없음)"
  SETUP_CMD="/work-system:work-setup"
  # 예전에 직접 복사로 설치했던 work-system 파일만 정리 (중복 방지)
  if [ -s "$MANIFEST" ]; then
    while IFS= read -r rel; do
      case "$rel" in skills/*|agents/*) rm -rf "${CLAUDE_HOME:?}/$rel"; echo "  이전 복사본 정리: $rel";; esac
    done < <(grep -E '^(skills|agents)/' "$MANIFEST" || true)
    grep -vE '^(skills|agents)/' "$MANIFEST" > "$MANIFEST.tmp" || true; mv "$MANIFEST.tmp" "$MANIFEST"
  fi
else
  echo "  플러그인 설치를 쓸 수 없어 직접 복사합니다 (기존 파일은 덮어쓰지 않음)."
  mkdir -p "$CLAUDE_HOME/skills" "$CLAUDE_HOME/agents"
  SETUP_CMD="/work-setup"
  for d in "$PKG"/skills/*/; do
    n="$(basename "$d")"; target="skills/$n"
    if [ -e "$CLAUDE_HOME/$target" ] && ! owned "$target"; then
      target="skills/ws-$n"
      echo "  이름 겹침: 기존 스킬 '$n' 유지 → work-system 것은 'ws-$n'으로 설치"
      [ "$n" = work-setup ] && SETUP_CMD="/ws-work-setup"
    fi
    rm -rf "${CLAUDE_HOME:?}/$target"; cp -r "$d" "$CLAUDE_HOME/$target"
    if [ "$target" != "skills/$n" ]; then
      rename_fm "$CLAUDE_HOME/$target/SKILL.md" "ws-$n"
    fi
    record "$target"
  done
  for f in "$PKG"/agents/*.md; do
    n="$(basename "$f" .md)"; target="agents/$n.md"
    if [ -e "$CLAUDE_HOME/$target" ] && ! owned "$target"; then
      target="agents/ws-$n.md"
      echo "  이름 겹침: 기존 에이전트 '$n' 유지 → work-system 것은 'ws-$n'으로 설치"
      cp "$f" "$CLAUDE_HOME/$target"
      rename_fm "$CLAUDE_HOME/$target" "ws-$n"
    else
      cp "$f" "$CLAUDE_HOME/$target"
    fi
    record "$target"
  done
  echo "  스킬·에이전트 설치 완료 (목록: $MANIFEST)"
fi

# 3) 작업 시스템 홈 파일 (없을 때만)
for f in AGENTS.md INSTRUCTIONS.md LESSONS.md CONFIG.md INTEGRATION.md; do
  if [ ! -e "$HOME_DIR/$f" ]; then cp "$PKG/skills/work-setup/templates/$f" "$HOME_DIR/$f"; echo "  생성: work-system/$f"; fi
done
[ -e "$HOME_DIR/CHANGELOG.md" ] || printf '# 개인 설정 변경 기록\n\n' > "$HOME_DIR/CHANGELOG.md"
printf -- '- %s work-system v%s 설치 (%s)\n' "$(date +%F)" "$VER" "${MODE:-copy}" >> "$HOME_DIR/CHANGELOG.md"

# 4) 모든 세션에 지침 연결 (백업 후 한 줄 추가)
LINE="@~/.claude/work-system/AGENTS.md"
if [ -f "$CLAUDE_HOME/CLAUDE.md" ] && grep -qF "$LINE" "$CLAUDE_HOME/CLAUDE.md"; then
  echo "  CLAUDE.md 연결: 이미 있음"
else
  if [ -s "$CLAUDE_HOME/CLAUDE.md" ]; then
    mkdir -p "$HOME_DIR/backup/$TODAY"; cp "$CLAUDE_HOME/CLAUDE.md" "$HOME_DIR/backup/$TODAY/CLAUDE.md"
    echo "  CLAUDE.md 백업: work-system/backup/$TODAY/CLAUDE.md"
  fi
  printf '\n# work-system (AI 업무 시스템 지침 — 기존 지침이 우선)\n%s\n' "$LINE" >> "$CLAUDE_HOME/CLAUDE.md"
  echo "  CLAUDE.md 연결: 맨 끝에 한 줄 추가 (기존 내용 유지)"
fi

cat <<MSG

✅ 설치 완료. 다음 단계:
  1. 자료가 들어 있는 작업 폴더에서 Claude Code를 새로 시작
  2. 이렇게 입력:  $SETUP_CMD
     → 기존 스킬·지침을 조사해 겹치는 부분의 사용 규칙을 정하고(기존 것 우선),
       세션 자료로 내 지침·팀원을 채운 뒤 회사 계정에 팀 업무 보드를 만듭니다.
  제거:  bash "$PKG/uninstall.sh"
MSG
