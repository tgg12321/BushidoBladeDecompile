set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
W=tmp/audit-2026-09-29/g8-dump71c4c; rm -rf $W; mkdir -p $W
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FL="-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
python3 - <<'PY'
import re
from pathlib import Path
t = Path('src/text1b_tu1d.c').read_text()
Path('tmp/audit-2026-09-29/g8-dump71c4c/landed.c').write_text(t)
a = t.index('void func_80071C4C(s32 arg0) {'); b = t.index('\n}\n', a) + 3
body = t[a:b]
pat = re.compile(r'            s32 dst = i \* 10; /\*.*?\*/\n\n(\s*)\*\(u8 \*\)\(D_800A3568 \+ dst\)', re.S)
assert len(pat.findall(body)) == 1
body2 = pat.sub(lambda m: m.group(1) + '*(u8 *)(D_800A3568 + i * 10)', body)
Path('tmp/audit-2026-09-29/g8-dump71c4c/inlined1.c').write_text(t[:a] + body2 + t[b:])
PY
for v in landed inlined1; do
  mkdir -p $W/$v
  $CPP $W/$v.c > $W/$v/$v.i
  tools/gcc-2.7.2/build/cc1 $FL $W/$v/$v.i -o $W/$v/frozen.s
  (cd $W/$v && ../../../../tools/gcc-2.7.2/cc1 $FL -da $v.i -o instr.s)
  diff <(grep -v '^ #' $W/$v/frozen.s) <(grep -v '^ #' $W/$v/instr.s) > /dev/null && echo "$v: instrumented cc1 == frozen cc1 (.s)" || echo "$v: CC1 OUTPUTS DIFFER"
done
ls $W/landed | head -30
