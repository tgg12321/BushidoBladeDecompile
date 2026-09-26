#!/bin/bash
# Run the ORIGINAL Psy-Q ASPSX 2.34 (psyq3.5) via dosemu2 on an asm file.
# usage: aspsx.sh <in.s> <out.obj> [-G8]
set -eo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
IN="$1"; OUT="$2"; G="${3:--G8}"
WORK="/dev/shm/aspsx_$$_$(date +%N)"
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT
cp "$HERE/psyq/psyq3.5/ASPSX.EXE" "$WORK/ASPSX.EXE"
# ASPSX requires DOS (CRLF) line endings
python3 "$HERE/crlf.py" "$IN" "$WORK/T.S"
export XDG_RUNTIME_DIR="/tmp/runtime-$(id -u)"
mkdir -p "$XDG_RUNTIME_DIR/dosemu2"; chmod 700 "$XDG_RUNTIME_DIR" 2>/dev/null || true
dosemu -dumb -K "$WORK" -E "ASPSX.EXE $G -o T.OBJ T.S" </dev/null >"$WORK/log.txt" 2>&1 || true
grep -a -A20 "ASPSX version" "$WORK/log.txt" | grep -av "kbd: EOF" >&2 || true
if [ -f "$WORK/T.OBJ" ]; then cp "$WORK/T.OBJ" "$OUT";
elif [ -f "$WORK/t.obj" ]; then cp "$WORK/t.obj" "$OUT";
else echo "aspsx: no object" >&2; exit 1; fi
