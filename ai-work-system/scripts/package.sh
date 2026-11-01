#!/usr/bin/env bash
# 배포 파일 생성
#   dist/work-system-<버전>.zip   : 회사 PC 등에 옮겨 install.sh / install.ps1 로 한 번에 설치하는 전체 패키지
#   dist/claude-ai/<스킬>.zip      : claude.ai 설정 → 스킬 업로드용 (스킬 1개당 1개)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VER="$(cat "$ROOT/VERSION")"
OUT="$ROOT/dist"
rm -rf "$OUT" && mkdir -p "$OUT/claude-ai"
STAGE="$(mktemp -d)"
mkdir -p "$STAGE/work-system"
(cd "$ROOT" && tar --exclude='./dist' -cf - .) | (cd "$STAGE/work-system" && tar -xf -)
(cd "$STAGE" && zip -qr "$OUT/work-system-$VER.zip" work-system)
rm -rf "$STAGE"
echo "dist/work-system-$VER.zip"
cd "$ROOT/skills"
for d in */; do n="${d%/}"; zip -qr "$OUT/claude-ai/$n.zip" "$n"; echo "dist/claude-ai/$n.zip"; done
