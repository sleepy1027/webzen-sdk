#!/usr/bin/env bash
# Claude Code에 설치: 기본은 전역(~/.claude), 인자로 대상 저장소 경로를 주면 <repo>/.claude
# skills/* → .claude/skills/, agents/*.md → .claude/agents/
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BASE="${1:+$1/.claude}"
BASE="${BASE:-$HOME/.claude}"
DEST="$BASE/skills"
mkdir -p "$DEST" "$BASE/agents"
for d in "$ROOT"/skills/*/; do
  n="$(basename "$d")"
  rm -rf "$DEST/$n"
  cp -r "$d" "$DEST/$n"
  echo "installed: $DEST/$n"
done
for f in "$ROOT"/agents/*.md; do
  cp "$f" "$BASE/agents/"
  echo "installed: $BASE/agents/$(basename "$f")"
done
