#!/usr/bin/env bash
# work-system 한 번에 설치 (macOS / Linux / WSL / Git Bash)
#   bash install.sh
# 1) 패키지를 ~/.claude/work-system/package 로 복사
# 2) Claude Code 플러그인으로 설치 (claude CLI가 없으면 ~/.claude/skills, agents 로 직접 복사)
# 3) 작업 시스템 홈(~/.claude/work-system)에 지침·기록 파일 생성 (기존 파일은 유지)
# 4) ~/.claude/CLAUDE.md 에 지침 연결 한 줄 추가 (이미 있으면 건너뜀)
set -euo pipefail
SRC="$(cd "$(dirname "$0")" && pwd)"
CLAUDE_HOME="${CLAUDE_CONFIG_DIR:-$HOME/.claude}"
HOME_DIR="$CLAUDE_HOME/work-system"
PKG="$HOME_DIR/package"
VER="$(cat "$SRC/VERSION" 2>/dev/null || echo unknown)"
MKT="work-system-local"

echo "▶ work-system v$VER 설치"
mkdir -p "$HOME_DIR"

# 1) 패키지 복사 (설치 위치 고정)
if [ "$SRC" != "$PKG" ]; then
  rm -rf "$PKG.tmp" && mkdir -p "$PKG.tmp"
  (cd "$SRC" && tar --exclude='./dist' --exclude='./.git' -cf - .) | (cd "$PKG.tmp" && tar -xf -)
  rm -rf "$PKG" && mv "$PKG.tmp" "$PKG"
fi
echo "  패키지: $PKG"

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
if [ "$MODE" = plugin ]; then
  echo "  플러그인 설치 완료: work-system@$MKT"
  SETUP_CMD="/work-system:work-setup"
else
  echo "  claude CLI 플러그인 설치를 쓸 수 없어 직접 복사합니다."
  mkdir -p "$CLAUDE_HOME/skills" "$CLAUDE_HOME/agents"
  for d in "$PKG"/skills/*/; do n="$(basename "$d")"; rm -rf "$CLAUDE_HOME/skills/$n"; cp -r "$d" "$CLAUDE_HOME/skills/$n"; done
  cp "$PKG"/agents/*.md "$CLAUDE_HOME/agents/"
  echo "  스킬·에이전트 복사 완료: $CLAUDE_HOME/skills, $CLAUDE_HOME/agents"
  SETUP_CMD="/work-setup"
fi

# 3) 작업 시스템 홈 파일 (없을 때만)
for f in AGENTS.md INSTRUCTIONS.md LESSONS.md CONFIG.md; do
  if [ ! -e "$HOME_DIR/$f" ]; then cp "$PKG/skills/work-setup/templates/$f" "$HOME_DIR/$f"; echo "  생성: $HOME_DIR/$f"; else echo "  유지: $HOME_DIR/$f"; fi
done
[ -e "$HOME_DIR/CHANGELOG.md" ] || printf '# 개인 설정 변경 기록\n\n' > "$HOME_DIR/CHANGELOG.md"
printf -- '- %s work-system v%s 설치 (%s)\n' "$(date +%F)" "$VER" "${MODE:-copy}" >> "$HOME_DIR/CHANGELOG.md"

# 4) 모든 세션에 지침 연결
LINE="@~/.claude/work-system/AGENTS.md"
touch "$CLAUDE_HOME/CLAUDE.md"
if grep -qF "$LINE" "$CLAUDE_HOME/CLAUDE.md"; then
  echo "  CLAUDE.md 연결: 이미 있음"
else
  printf '\n# work-system (AI 업무 시스템 지침)\n%s\n' "$LINE" >> "$CLAUDE_HOME/CLAUDE.md"
  echo "  CLAUDE.md 연결: 추가함"
fi

cat <<MSG

✅ 설치 완료. 다음 단계:
  1. 자료가 들어 있는 작업 폴더에서 Claude Code를 새로 시작
  2. 이렇게 입력:  $SETUP_CMD
     → 세션 자료를 읽어 내 지침·팀원 목록을 채우고, 회사 계정에 팀 업무 보드를 만들어 줍니다.
MSG
