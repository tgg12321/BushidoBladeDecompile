#!/bin/bash
# psylink.sh <workdir> <cmdline...>: run Sony PSYLINK (psyq3.5) under dosemu2 in <workdir> (files must be
# DOS 8.3 names). Prints the DOS console output.
set -e
W="$1"; shift
cp /mnt/c/Users/Trenton/Desktop/Bushido\ Blade\ 2\ Decompile/tmp/research36140/psyq/psyq3.5/PSYLINK.EXE "$W/PSYLINK.EXE"
export XDG_RUNTIME_DIR="/tmp/runtime-$(id -u)"
mkdir -p "$XDG_RUNTIME_DIR/dosemu2"; chmod 700 "$XDG_RUNTIME_DIR" 2>/dev/null || true
dosemu -dumb -K "$W" -E "PSYLINK.EXE $*" </dev/null 2>&1 | grep -av "kbd: EOF" | tail -40
