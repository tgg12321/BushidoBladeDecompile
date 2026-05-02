#!/bin/bash
# agent_log.sh -- Append a structured event to a function's session log
# Usage:
#   bash tools/agent_log.sh <func_name> SESSION_START "tier=sonnet"
#   bash tools/agent_log.sh <func_name> READ "asm/funcs/func_80034200.s"
#   bash tools/agent_log.sh <func_name> EDIT "changed s32 count to u16, reordered v0/v1 decls"
#   bash tools/agent_log.sh <func_name> SESSION_END "score=0 attempts=14 status=MATCHED"
#
# Log format (append-only, one line per event):
#   TIMESTAMP|EVENT|DATA
# Events: SESSION_START, READ, EDIT, BASH, SESSION_END
# Log lives at: permuter/<func_name>/session_log.txt

if [ $# -lt 3 ]; then
    echo "Usage: bash tools/agent_log.sh <func_name> <EVENT> <data>" >&2
    exit 1
fi

FUNC_NAME="$1"
EVENT="$2"
DATA="$3"

DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$DIR"
while [ ! -f "$ROOT/Makefile" ] && [ "$ROOT" != "/" ]; do
    ROOT="$(dirname "$ROOT")"
done

LOG="$ROOT/permuter/$FUNC_NAME/session_log.txt"
TS=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

echo "${TS}|${EVENT}|${DATA}" >> "$LOG"
