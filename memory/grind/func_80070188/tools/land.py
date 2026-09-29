"""Apply the func_80070188 landing to the tree. Run from the repo root under WSL.
- src/text1b.c: candidate body over the INCLUDE_ASM line; SelectEntryE534 `pad`
  -> `unk1`; every consumer of the 0x800A3560 records respelled through the
  merged declaration (merge_reps.py); the TU-local byte/scalar externs removed.
- include/game.h: the record declaration.
- sdata_exclude.txt / sdata_syms.txt: the three row edits + D_800A3590.
- undefined_syms_auto.txt: alias suffixes on D_800A3561/2/3/5 (func_80070F78)."""
import sys, os
sys.path.insert(0, '.')
from pathlib import Path
from engine import inlineasm
import importlib.util

os.environ.setdefault('OTHER_FORM', 'o1')
_s = importlib.util.spec_from_file_location('merge_reps', 'tmp/func_80070188/merge_reps.py')
mr = importlib.util.module_from_spec(_s); _s.loader.exec_module(mr)


def sub(t, old, new, n=1):
    c = t.count(old)
    assert c == n, (c, n, old[:90])
    return t.replace(old, new)


# ---- src/text1b.c
p = Path('src/text1b.c')
t = p.read_text(encoding='utf-8')
body = Path('memory/grind/func_80070188/candidate.c').read_text(encoding='utf-8')
t = inlineasm.substitute_body(t, 'func_80070188', body)
t = sub(t, '    u8 value;\n    u8 pad;\n} SelectEntryE534;', '    u8 value;\n    u8 unk1;\n} SelectEntryE534;')
for old, new, n in mr.reps(''):
    t = sub(t, old, new, n)
p.write_bytes(t.encode('utf-8'))

# ---- include/game.h
p = Path('include/game.h')
g = p.read_text(encoding='utf-8')
anchor = 'extern s16 D_800A34F0[2];\n\n'
block = """/* 0x800A3560: two 3-byte records, one per selection slot i (slot i at
 * 0x800A3560 + i * 3; 0x800A3566/7 pad before D_800A3568). Object model evidence
 * from the original binary, independent of the byte-chasing: func_8006F100 reads
 * bytes +2 and +1 through ONE offset register stepped by 3 per slot
 * (asm/funcs/func_8006F100.s:73-75 `lbu %lo(D_800A3562)($at)` and :100-102
 * `lbu %lo(D_800A3561)($at)` with $s3, `addiu $s3,$s3,0x3` at :267);
 * func_80070C70 walks byte +0 the same way ($s2, `addiu $s2,$s2,0x3`,
 * func_80070C70.s:114-116, :166); func_80070188 forms i*3 once and reaches
 * +0, +1 and +2 through it (func_80070188.s:58-76, 386-419); func_8006E534 stores
 * byte +1 of both records (func_8006E534.s:96-97, D_800A3561 / D_800A3564).
 * Replaces the splat per-byte symbols D_800A3560..D_800A3565 in C (per-word splat
 * symbol -> aggregate merge family, owner ruling 2026-08-17); their
 * undefined_syms_auto.txt rows stay for the still-asm func_80070F78. */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
} Unk800A3560Record;

extern Unk800A3560Record D_800A3560[2];

"""
g = sub(g, anchor, anchor + block)
p.write_bytes(g.encode('utf-8'))

# ---- sdata_exclude.txt / sdata_syms.txt
p = Path('sdata_exclude.txt')
x = p.read_text(encoding='utf-8')
x = sub(x, 'func_8006F97C: D_800A3560, D_800A3588, D_800A358C, g_gpu_ot_ptr\n',
        'func_8006F97C: D_800A3588, D_800A358C, g_gpu_ot_ptr\n')
x = sub(x, 'func_80070188: D_800A3562, D_800A3588, D_800A358C, g_gpu_ot_ptr\n',
        'func_80070188: D_800A3562, g_gpu_ot_ptr\n')
x = sub(x, 'func_80071C4C: D_800A3560, D_800A3562\n', 'func_80071C4C: D_800A3562\n')
p.write_bytes(x.encode('utf-8'))
p = Path('sdata_syms.txt')
x = p.read_text(encoding='utf-8')
x = sub(x, 'D_800A358E\nD_800A3592\n', 'D_800A358E\nD_800A3590\nD_800A3592\n')
p.write_bytes(x.encode('utf-8'))

# ---- undefined_syms_auto.txt
p = Path('undefined_syms_auto.txt')
u = p.read_text(encoding='utf-8')
for sym, off, rec in (('D_800A3561', 1, 0), ('D_800A3562', 2, 0), ('D_800A3563', 3, 1), ('D_800A3565', 5, 1)):
    u = sub(u, f'{sym} = 0x{sym[2:]};\n',
            f'{sym} = 0x{sym[2:]};  /* alias of D_800A3560+{off} (Unk800A3560Record record {rec}); '
            f'retire with func_80070F78 (asm/funcs/func_80070F78.s is its only assembled referrer) */\n')
p.write_bytes(u.encode('utf-8'))
print('landed')
