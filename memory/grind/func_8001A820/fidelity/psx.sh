#!/bin/bash
# usage (WSL, repo root): bash tmp/func_8001A820/psx.sh <candidate.c> <tag>
# builds full.i and compiles it with BOTH our cc1 and cc1psx; extracts func_8001A820 from each.
set -e
cand="$1"; tag="$2"
out=tmp/func_8001A820/build_$tag
python3 tmp/func_8001A820/mk.py "$cand" "$tag" --s > /dev/null
flags="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
tools/cc1psx_wrapper.sh $flags < "$out/full.i" > "$out/psx.s" 2> "$out/psx.err" || { echo "cc1psx failed"; tail -5 "$out/psx.err"; }
python3 - "$out" <<'EOF'
import sys, re
d = sys.argv[1]
def ext(p):
    t = open(p, errors='replace').read()
    m = re.search(r'\nfunc_8001A820:\n(.*?)\n\s*\.end\s+func_8001A820', t, re.S)
    return m.group(1) if m else ''
for n in ('full.s', 'psx.s'):
    open(f'{d}/{n}.fn', 'w').write(ext(f'{d}/{n}'))
    print(n, len(ext(f'{d}/{n}').splitlines()))
EOF
