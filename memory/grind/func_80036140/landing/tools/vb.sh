#!/bin/bash
# Rebuild only code6cac_b5.o of a scratch tree with a func_80036140 body variant and score it.
# usage (WSL, repo root): bash tmp/func_80036140/vb.sh <tree> <variant-file> [G0] [--diff]
set -o pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
T=tmp/func_80036140/$1; V=$2; G=$3
[ -f $T/src/code6cac_b5.c.orig ] || cp $T/src/code6cac_b5.c $T/src/code6cac_b5.c.orig
python3 - "$T/src/code6cac_b5.c.orig" "$V" "$T/src/code6cac_b5.c" <<'EOF'
import sys
sys.path.insert(0, '.')
from engine import inlineasm
from pathlib import Path
src = Path(sys.argv[1]).read_text()
# the body variants carry their own extern prelude: strip the one in the tree copy first
i = src.index('extern void CdMix(CdlATV *);')
j = src.index('void func_80036140(void) {')
src = src[:i] + 'INCLUDE_ASM("asm/funcs", func_80036140);\n' + src[src.index('\n}\n', j) + 3:]
v = Path(sys.argv[2]).read_text()
k = v.index('void func_80036140(void) {')
src = src.replace('INCLUDE_ASM("asm/funcs", func_80036140);\n', v[:k] + 'INCLUDE_ASM("asm/funcs", func_80036140);\n')
Path(sys.argv[3]).write_bytes(inlineasm.substitute_body(src, 'func_80036140', v[k:]).encode())
EOF
rm -f $T/build/src/code6cac_b5.o
if [ "$G" = "G0" ]; then
  make -C $T GP_FILES="text1a_pre text1a_post code6cac_b3 code6cac_b4" build/src/code6cac_b5.o >/dev/null 2>$T/vb.err || { tail -5 $T/vb.err; exit 1; }
else
  make -C $T build/src/code6cac_b5.o >/dev/null 2>$T/vb.err || { tail -5 $T/vb.err; exit 1; }
fi
python3 - $T "$4" <<'EOF'
import sys, json
sys.path.insert(0, '.')
from engine import score
score._symtab()['D_80101E58'] = 0x80101E58
o = sys.argv[1] + '/build/src/code6cac_b5.o'; ref = 'tmp/func_80036140/base/build/src/code6cac_b2_post.o'
print({f: score.score_func(o, ref, f)['score'] for f in ('func_80036140', 'func_80036940')})
if sys.argv[2] == '--diff':
    for h in score.insn_diff(o, ref, 'func_80036140').get('hunks', []):
        if h.get('class') != 'not-scored':
            print(json.dumps(h))
EOF
