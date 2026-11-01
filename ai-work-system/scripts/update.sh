#!/usr/bin/env bash
# 키트를 원본 git 저장소에서 최신으로 받고, 변경 요약을 보여준 뒤 재설치/재패키징한다.
# 사용: ./scripts/update.sh [install.sh에 넘길 대상 저장소 경로]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "git 저장소가 아니라서 자동 업데이트를 할 수 없습니다. 새 버전 폴더로 교체하세요." >&2
  exit 1
fi
if [ -n "$(git status --porcelain -- .)" ]; then
  echo "로컬 변경이 있습니다. 커밋하거나 stash한 뒤 다시 실행하세요:" >&2
  git status --short -- . >&2
  exit 1
fi
OLD="$(cat VERSION 2>/dev/null || echo unknown)"
BEFORE="$(git rev-parse HEAD)"
git pull --ff-only
NEW="$(cat VERSION 2>/dev/null || echo unknown)"
if [ "$BEFORE" = "$(git rev-parse HEAD)" ]; then
  echo "이미 최신입니다 (v$NEW)."
else
  echo "업데이트: v$OLD → v$NEW"
  git diff --stat "$BEFORE" HEAD -- .
fi
"$ROOT/scripts/install.sh" "$@"
if command -v zip >/dev/null; then "$ROOT/scripts/package.sh"; fi
