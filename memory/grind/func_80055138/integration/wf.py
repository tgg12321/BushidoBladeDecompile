"""Whole-file scratch build: splice a candidate into a COPY of src/text1b.c with the
D_80099D88 record-table declaration applied (aggregate model), compile with the sandbox
pipeline, and score func_80055138 + func_80055948 (the other C consumer) against build/.
usage (WSL, repo root): python3 tmp/func_80055138/wf.py <candidate.c> [--hdr]
"""
import sys, subprocess, os
sys.path.insert(0, '.')
from engine import score as S

cand = open(sys.argv[1]).read()
src = open('src/text1b.c').read()
MARK = 'INCLUDE_ASM("asm/funcs", func_80055138);'
assert src.count(MARK) == 1
src = src.replace(MARK, cand.rstrip('\n'))
old_decl = 'extern u16 D_80099D88;\n'
assert src.count(old_decl) == 1
src = src.replace(old_decl, '')
old_use = '(*(&D_80099D88 + idx * 12) & 0xBF00)'
assert src.count(old_use) == 1
src = src.replace(old_use, '(D_80099D88[idx].flags & 0xBF00)')
os.makedirs('tmp/func_80055138/wf', exist_ok=True)
open('tmp/func_80055138/wf/text1b.c', 'w').write(src)
cfg = 'tmp/sandbox/func_80055138/cfg'
cmd = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ "
       "-D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "
       "tmp/func_80055138/wf/text1b.c | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 "
       "-mno-abicalls -fno-builtin -w -mel -msoft-float | "
       f"PROLOGUE_CONFIG={cfg}/prologue_config.json DELAY_SLOT_RA_FUNCS={cfg}/delay_slot_ra_funcs.txt "
       f"FRAME_FIX_FUNCS={cfg}/frame_fix_funcs.txt python3 tools/prologue_fix.py | "
       "python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt "
       "--sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt "
       "--multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt "
       "--prefill-label-funcs=maspsx_prefill_label_funcs.txt | sed 's/\\.align\\t3/.align\\t2/' | "
       "python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 "
       "-mtune=r3000 -no-pad-sections -O1 -G0 -o tmp/func_80055138/wf/text1b.o")
r = subprocess.run(['bash', '-c', 'set -o pipefail; ' + cmd], capture_output=True, text=True)
if r.returncode:
    print('BUILD FAILED\n', r.stderr[-3000:])
    sys.exit(1)
for f in ('func_80055138', 'func_80055948'):
    print(f, S.score_func('tmp/func_80055138/wf/text1b.o', 'build/src/text1b.o', f))
