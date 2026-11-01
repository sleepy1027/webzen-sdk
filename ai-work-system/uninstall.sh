#!/usr/bin/env bash
# work-system 제거. work-system이 설치한 것만 지운다 (installed.txt 기준). 기존 스킬·에이전트는 건드리지 않는다.
#   bash uninstall.sh          : 플러그인·설치 파일·CLAUDE.md 연결 줄 제거, 개인 설정(~/.claude/work-system)은 보존
#   bash uninstall.sh --purge  : 개인 설정 폴더까지 삭제
set -euo pipefail
CLAUDE_HOME="${CLAUDE_CONFIG_DIR:-$HOME/.claude}"
HOME_DIR="$CLAUDE_HOME/work-system"
MANIFEST="$HOME_DIR/installed.txt"
MKT="work-system-local"
if command -v claude >/dev/null 2>&1; then
  claude plugin uninstall "work-system@$MKT" >/dev/null 2>&1 && echo "  플러그인 제거" || true
  claude plugin marketplace remove "$MKT" >/dev/null 2>&1 || true
fi
if [ -f "$MANIFEST" ]; then
  while IFS= read -r rel; do
    case "$rel" in skills/*|agents/*) rm -rf "${CLAUDE_HOME:?}/$rel" && echo "  제거: $rel";; esac
  done < "$MANIFEST"
  : > "$MANIFEST"
fi
if [ -f "$CLAUDE_HOME/CLAUDE.md" ]; then
  grep -vF -e "@~/.claude/work-system/AGENTS.md" -e "# work-system (AI 업무 시스템 지침" "$CLAUDE_HOME/CLAUDE.md" > "$CLAUDE_HOME/CLAUDE.md.tmp" || true
  mv "$CLAUDE_HOME/CLAUDE.md.tmp" "$CLAUDE_HOME/CLAUDE.md"
  echo "  CLAUDE.md 연결 줄 제거"
fi
if [ "${1:-}" = "--purge" ]; then rm -rf "$HOME_DIR"; echo "  개인 설정 삭제: $HOME_DIR"; else echo "  개인 설정 보존: $HOME_DIR"; fi
echo "✅ 제거 완료"
