#!/bin/bash
# usage: bash tmp/f759d0/mkperm.sh <candidate.c> <dirname>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
CAND="$1"; D="tools/decomp-permuter/nonmatchings/$2"
rm -rf "$D"; mkdir -p "$D"
cp tools/decomp-permuter/nonmatchings/func_80074E08/compile.sh "$D/compile.sh"
chmod +x "$D/compile.sh"
cat > "$D/settings.toml" <<'EOF'
func_name = "func_800759D0"
compiler_type = "base"
EOF
sed "s/^.set gp=64$//" tools/decomp-permuter/prelude.inc | cat - asm/funcs/func_800759D0.s > "$D/target.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$D/target.o" "$D/target.s"
python3 - "$CAND" "$D" <<'PY'
import sys, subprocess
sys.path.insert(0, 'tools/decomp-permuter')
from strip_other_fns import strip_other_fns
cand, d = sys.argv[1], sys.argv[2]
src = open('src/text1b.c').read()
line = 'INCLUDE_ASM("asm/funcs", func_800759D0);'
tu = src[:src.index(line)] + open(cand).read() + '\n'
open(d + '/tu.c', 'w').write(tu)
cpp = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 "
       "-D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ "
       "-D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -P " + d + "/tu.c")
out = subprocess.run(['bash', '-c', cpp], capture_output=True, text=True).stdout
s = strip_other_fns(out, 'func_800759D0')
# drop top-level __asm__(...) statements (INCLUDE_ASM / canonical bodies of other fns)
res, i = [], 0
while True:
    k = s.find('__asm__', i)
    if k < 0:
        res.append(s[i:]); break
    ls = s.rfind('\n', 0, k) + 1
    if s[ls:k].strip():  # not at line start -> inside a function; keep
        res.append(s[i:k + 7]); i = k + 7; continue
    res.append(s[i:k])
    j = s.index('(', k); depth = 0
    while True:
        c = s[j]
        if c == '"':
            j += 1
            while s[j] != '"':
                j += 2 if s[j] == '\\' else 1
        elif c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                break
        j += 1
    i = s.index(';', j) + 1
open(d + '/base.c', 'w').write(''.join(res))
PY
rm -f "$D/tu.c"
bash "$D/compile.sh" "$D/base.c" -o "$D/base.o"
python3 tools/decomp-permuter/permuter.py "$D" --stack-diffs --help >/dev/null 2>&1 || true
ls "$D"
