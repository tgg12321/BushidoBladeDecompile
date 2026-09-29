#!/bin/bash
# usage: dump.sh <func> <body.c> <outdir> [cc1 dump flags...]
# Substitutes <body.c> for <func> in a copy of src/text1b.c, preprocesses with the
# build's cpp recipe and runs cc1 with the build's CC_FLAGS plus the dump flags.
# Dumps are cut down to <func> only (file.<pass> -> <outdir>/<pass>.txt).
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
FUNC="$1"; BODY="$2"; OUT="$3"; shift 3
CC1="${CC1:-tools/gcc-2.7.2/cc1}"
mkdir -p "$OUT"
python3 - "$FUNC" "$BODY" "$OUT/t.c" <<'PY'
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm
src = Path('src/text1b.c').read_text(encoding='utf-8')
src = inlineasm.substitute_body(src, sys.argv[1], Path(sys.argv[2]).read_text(encoding='utf-8'))
Path(sys.argv[3]).write_text(src, encoding='utf-8', newline='\n')
PY
CPP_DEFS=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CPP_DEFS if isinstance(b.CPP_DEFS,str) else ' '.join(b.CPP_DEFS))")
CC_FLAGS=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CC_FLAGS)")
mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin $CPP_DEFS "$OUT/t.c" > "$OUT/t.i" 2>/dev/null
( cd "$OUT" && "$OLDPWD/$CC1" $CC_FLAGS "$@" t.i -o t.s ) > "$OUT/cc1.log" 2>&1 || { echo "cc1 failed"; tail "$OUT/cc1.log"; exit 1; }
echo "cmd: $CC1 $CC_FLAGS $* t.i -o t.s" > "$OUT/cmdline.txt"
for f in "$OUT"/t.i.* "$OUT"/t.s; do
  [ -f "$f" ] || continue
  python3 - "$FUNC" "$f" <<'PY'
import sys, re
func, path = sys.argv[1], sys.argv[2]
txt = open(path, encoding='latin-1').read()
if path.endswith('.s'):
    i = txt.find('\n' + func + ':')
    j = txt.find('.end\t' + func, i)
    part = txt[i:j+40] if i >= 0 else ''
else:
    i = txt.find(';; Function ' + func)
    j = txt.find(';; Function ', i + 10)
    part = txt[i:j if j > 0 else len(txt)] if i >= 0 else ''
open(path + '.' + func, 'w').write(part)
PY
  rm -f "$f"
done
ls "$OUT"
