set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
# loop-2 inlined dump
W=tmp/audit-2026-09-29/g8-dump71c4c; v=inlined2; mkdir -p $W/$v
python3 - <<'PY'
import re
from pathlib import Path
t = Path('src/text1b_tu1d.c').read_text()
a = t.index('void func_80071C4C(s32 arg0) {'); b = t.index('\n}\n', a) + 3
body = t[a:b]
pat = re.compile(r'            s32 dst = i \* 10; /\*(?:(?!\*/).)*\*/\n\n(\s*)\*\(u8 \*\)\(D_800A3568 \+ dst \+ 1\)', re.S)
assert len(pat.findall(body)) == 1
body2 = pat.sub(lambda m: m.group(1) + '*(u8 *)(D_800A3568 + i * 10 + 1)', body)
Path('tmp/audit-2026-09-29/g8-dump71c4c/inlined2.c').write_text(t[:a] + body2 + t[b:])
PY
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FL="-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
$CPP $W/$v.c > $W/$v/$v.i
tools/gcc-2.7.2/build/cc1 $FL $W/$v/$v.i -o $W/$v/frozen.s
(cd $W/$v && ../../../../tools/gcc-2.7.2/cc1 $FL -da $v.i -o instr.s)
diff <(grep -v '^ #' $W/$v/frozen.s) <(grep -v '^ #' $W/$v/instr.s) > /dev/null && echo "$v: instrumented == frozen (.s)"
python3 - <<'PY'
import re
from pathlib import Path
W = Path('tmp/audit-2026-09-29/g8-dump71c4c'); v = 'inlined2'
D = Path('memory/grind/_completed/func_80071C4C/g8-dst-2026-09-30')
for p in ('rtl', 'combine', 'greg'):
    t = (W / v / f'{v}.i.{p}').read_text(errors='replace')
    m = re.search(r'\n;; Function func_80071C4C\n', t); e = t.find('\n;; Function ', m.end())
    (D / f'{v}.71c4c.{p}').write_text(t[m.start():e if e > 0 else None])
s = (W / v / 'frozen.s').read_text(); a = s.index('\nfunc_80071C4C:'); e = s.index('.end\tfunc_80071C4C', a)
(D / f'{v}.71c4c.s').write_text(s[a:e])
PY
D=memory/grind/_completed/func_80071C4C/g8-dst-2026-09-30
diff $D/landed.71c4c.s $D/inlined2.71c4c.s || true
grep -n "D_800A3568" $D/inlined2.71c4c.rtl | tail -2
