# land_p1.py: preparatory landing for func_80055B60 -- the D_80106A78 object records and the D_800A36F2
# per-player byte pair get their real types; every C consumer respelled onto them. Landing lock only.
import subprocess, sys

def rw(path, fn):
    s = open(path, encoding='utf-8', newline='').read()
    assert '\r' not in s
    t = fn(s)
    assert t != s, path
    open(path, 'w', encoding='utf-8', newline='\n').write(t)

def one(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)

# header (hdr.py with the D_800A36F2 array variant)
subprocess.check_call([sys.executable, 'tmp/d6a78/hdr.py', 'include/code6cac.h', 'include/code6cac.h', 'arr'])
# code6cac_b_tu2.c: the respelled TU measured by tmp/d6a78/chk.sh (all 81 functions 0)
new = open('tmp/d6a78/b_tu2_arr.c', encoding='utf-8').read()
cur = open('src/code6cac_b_tu2.c', encoding='utf-8').read()
assert open('tmp/d6a78/b_tu2_base.c', encoding='utf-8').read() == cur, 'src/code6cac_b_tu2.c moved since the conversion'
open('src/code6cac_b_tu2.c', 'w', encoding='utf-8', newline='\n').write(new)
rw('src/code6cac_b.c', lambda s: one(s, "extern u8 D_80106A78;\n", ""))

def syms(s):
    s = one(s, "D_80106A7A = 0x80106A7A;\n", "")
    s = one(s, "D_80106A80 = 0x80106A80;\n", "")
    s = one(s, "D_80106A82 = 0x80106A82;\n", "")
    return s
rw('undefined_syms_auto.txt', syms)

def names(s):
    s = one(s, "g_active_slot_table_12                                  = 0x80106A7A;  /* 12 entries x 0x64 bytes; first s16 = id (-1 = empty); cleared when paired flag is set */\n", "")
    s = one(s, "g_active_slot_flags_12                                  = 0x80106A80;  /* per-slot u8 flag; if non-zero, paired slot in g_active_slot_table_12 is cleared (set to -1) */\n", "")
    s = one(s, "g_active_slot_flags_12_plus_2                           = 0x80106A82;  /* +2 from g_active_slot_flags_12 (0x80106A80) */\n", "")
    s = one(s, "D_80106A78                                     = 0x80106A78;  /* +5 from g_file_flags (0x80106A73) */  /* data-wave 2026-09-29: was g_file_flags_plus_5 */",
            "D_80106A78                                     = 0x80106A78;  /* Obj80106A78[12], the 0x64-byte object records (include/code6cac.h) */  /* data-wave 2026-09-29: was g_file_flags_plus_5 */")
    s = one(s, "loops 12x writing -1 (s16) to D_80106A7A and 0xFF (u8) to D_80106A82 at stride 0x64 */",
            "loops 12x writing -1 (s16) to D_80106A78[i].unk_02 and 0xFF (u8) to D_80106A78[i].unk_0A */")
    s = one(s, "/* scan 12-entry table D_80106A7A (stride0x64) for in-range",
            "/* scan the 12 D_80106A78 records for in-range")
    return s
rw('named_syms.txt', names)
print('p1 edits applied')
