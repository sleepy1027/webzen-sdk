#!/usr/bin/env bash
# Claude Code에 설치: 기본은 전역(~/.claude/skills), 인자로 대상 저장소 경로를 주면 <repo>/.claude/skills
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="${1:+$1/.claude/skills}"
DEST="${DEST:-$HOME/.claude/skills}"
mkdir -p "$DEST"
for d in "$ROOT"/skills/*/; do
  n="$(basename "$d")"
  rm -rf "$DEST/$n"
  cp -r "$d" "$DEST/$n"
  echo "installed: $DEST/$n"
done
