# tmp/b60/inc/include/code6cac.h = include/code6cac.h + the PracticeMenuRec members func_80055B60 reads
# (widths from its target loads/stores, asm/funcs/func_80055B60.s); PadState moves above PracticeMenuRec.
import os, sys
h = open('include/code6cac.h', encoding='utf-8').read()

def sub1(s, old, new):
    assert s.count(old) == 1, old
    return s.replace(old, new)

h = sub1(h, "    s16 unk_40;\n    u8  unk_42[0x5E - 0x42];\n",
         "    s16 unk_40;\n    u8  unk_42[0x50 - 0x42];\n"
         "    u8  *unk_50;\n"
         "    u8  unk_54[0x58 - 0x54];\n"
         "    u8  *unk_58;\n"
         "    u16 unk_5C;\n"
         "    s16 unk_5E;                    /* 0/1, set alongside func_80021A98 */\n")
h = sub1(h, "    s16 unk_5E;                    /* 0/1, set alongside func_80021A98 */\n    u8  unk_60",
         "    u8  unk_60")
h = sub1(h, "    s16 unk_6A;                    /* SEQ state code; CHAR_STRUCT_SCHEMA.md +0x06A */\n    u8  unk_6C[0x72 - 0x6C];\n",
         "    s16 unk_6A;                    /* SEQ state code; CHAR_STRUCT_SCHEMA.md +0x06A */\n    s16 unk_6C;\n    u8  unk_6E[0x72 - 0x6E];\n")
h = sub1(h, "    s16 unk_84;\n    u8  unk_86[0x88 - 0x86];\n", "    s16 unk_84;\n    s16 unk_86;\n")
h = sub1(h, "    u8  unk_A0;\n    u8  unk_A1[0xAD - 0xA1];\n",
         "    u8  unk_A0;\n    u8  unk_A1;\n    u8  unk_A2;\n    u8  unk_A3;\n    u8  unk_A4;\n    u8  unk_A5[0xAD - 0xA5];\n")
h = sub1(h, "    s32 unk_268;\n    u8  unk_26C[0x274 - 0x26C];\n",
         "    s32 unk_268;\n    s16 unk_26C;\n    u8  unk_26E[0x274 - 0x26E];\n")
h = sub1(h, "    u8  unk_352[0x44C - 0x352];\n",
         "    u8  unk_352[0x3B4 - 0x352];\n"
         "    s32 unk_3B4;                   /* != 0: func_80055B60 polls func_80055948, else func_80058580 */\n"
         "    u8  unk_3B8[0x3CC - 0x3B8];\n"
         "    s32 unk_3CC;\n"
         "    PadState unk_3D0;              /* the pad record func_80055B60 builds for func_8001BE20 */\n"
         "    u16 unk_3E8;                   /* frame counter; bit 0 picks the record func_80055B60 aims from */\n"
         "    u8  unk_3EA[0x3F0 - 0x3EA];\n"
         "    s16 unk_3F0;\n"
         "    u8  unk_3F2;\n"
         "    u8  unk_3F3;\n"
         "    u8  unk_3F4;\n"
         "    u8  unk_3F5;\n"
         "    u8  unk_3F6;\n"
         "    u8  unk_3F7;\n"
         "    s16 unk_3F8[3];\n"
         "    s16 unk_3FE[3];                /* written by func_80055138 */\n"
         "    s16 unk_404[3];\n"
         "    u8  unk_40A[0x414 - 0x40A];\n"
         "    u8  unk_414[8][2];             /* func_80055B60: 8 (target id, count) pairs */\n"
         "    u8  unk_424;\n"
         "    u8  unk_425;\n"
         "    u8  unk_426;\n"
         "    u8  unk_427;\n"
         "    s16 unk_428;\n"
         "    s16 unk_42A;\n"
         "    s16 unk_42C;\n"
         "    s16 unk_42E;\n"
         "    s32 unk_430;                   /* func_80055B60 decision flags */\n"
         "    u8  unk_434[0x438 - 0x434];\n"
         "    u16 unk_438;\n"
         "    s16 unk_43A;\n"
         "    s16 unk_43C;\n"
         "    s16 unk_43E;\n"
         "    u8  unk_440;\n"
         "    u8  unk_441;\n"
         "    u8  unk_442;\n"
         "    u8  unk_443;\n"
         "    u8  unk_444[0x44C - 0x444];\n")
# PadState must precede PracticeMenuRec
ps_start = h.index("/* Pad input record (0x18 bytes) at 0x80102788.")
ps_end = h.index("} PadState;")
ps_end = h.index("\n", ps_end) + 1
ps = h[ps_start:ps_end]
h = h[:ps_start] + h[ps_end:]
pm = h.index("/* Per-character / practice-menu record table")
h = h[:pm] + ps + "\n" + h[pm:]
os.makedirs('tmp/b60/inc/include', exist_ok=True)
open('tmp/b60/inc/include/code6cac.h', 'w', encoding='utf-8', newline='\n').write(h)
print('ok')
