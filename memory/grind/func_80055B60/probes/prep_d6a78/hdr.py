# hdr.py <in code6cac.h> <out>: add the D_80106A78 record type (preparatory landing for func_80055B60).
import sys
h = open(sys.argv[1], encoding='utf-8').read()
anchor = "extern Tbl8008E194 D_8008E194[];\n"
assert h.count(anchor) == 1
add = anchor + """/* The twelve 0x64-byte object records at 0x80106A78. func_80030580 spawns one
 * (kind unk_02 indexes D_8008E194 / D_8008EB80), func_80030D7C moves them,
 * func_80031B24 tests them against both fighters, func_80030208 hands them to
 * the effect calls; unk_02 == -1 marks a free record. Field widths are the
 * consumers' loads and stores (asm/funcs/func_80030580.s, func_80030D7C.s). */
typedef struct {
    s16 unk_00;                    /* frames since spawn */
    s16 unk_02;                    /* kind; -1 = free */
    u8  unk_04;
    u8  unk_05;
    u8  unk_06;                    /* owner: PracticeMenuRec index */
    u8  unk_07;
    u8  unk_08;
    u8  unk_09;                    /* index into the owner's matrix table (func_800300B4) */
    u8  unk_0A;                    /* slot number; 0xFF = never used */
    u8  unk_0B;
    u8  unk_0C[0x2C - 0x0C];       /* matrix read by func_800300B4 */
    Vec3i32 unk_2C;                /* position */
    Vec3i32 unk_38;                /* previous position */
    Vec3i32 unk_44;                /* velocity */
    s32 unk_50;                    /* != 0: moving */
    s16 unk_54[3];                 /* rotation angles */
    u8  unk_5A[0x5C - 0x5A];
    s16 unk_5C[3];                 /* angular speeds */
    u8  unk_62[0x64 - 0x62];
} Obj80106A78;                     /* sizeof == 0x64 */
extern Obj80106A78 D_80106A78[12];
"""
h = h.replace(anchor, add)
open(sys.argv[2], 'w', encoding='utf-8', newline='\n').write(h)
print('ok')
# variant: D_800A36F2 as the 2-entry per-player byte table func_8003047C indexes
if len(sys.argv) > 3 and sys.argv[3] == 'arr':
    h = open(sys.argv[2], encoding='utf-8').read()
    o = "extern u8 D_800A36F2;\n"
    assert h.count(o) == 1
    h = h.replace(o, "extern u8 D_800A36F2[2];             /* per-player byte, [unk_04] (func_8003047C) */\n")
    open(sys.argv[2], 'w', encoding='utf-8', newline='\n').write(h)
    print('arr')
