#!/usr/bin/env bash
# 호환용: 패키지 루트의 install.sh 를 실행한다.
exec bash "$(cd "$(dirname "$0")/.." && pwd)/install.sh" "$@"
