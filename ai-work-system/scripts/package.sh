#!/usr/bin/env bash
# 스킬 폴더마다 claude.ai 업로드용 zip 생성 → dist/<name>.zip
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$ROOT/dist"
cd "$ROOT/skills"
for d in */; do
  n="${d%/}"
  rm -f "$ROOT/dist/$n.zip"
  zip -qr "$ROOT/dist/$n.zip" "$n"
  echo "dist/$n.zip"
done
