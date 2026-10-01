# hdr.py: tmp/func_80055B60/inc/include/code6cac.h = include/code6cac.h + the PracticeMenuRec members
# func_80055B60 reads (u16 unk_5C, s16 unk_6C, PadState unk_3D0, u8 unk_414[8][2], s16 unk_43E).
# PadState moves above PracticeMenuRec. Run from the repo root.
import os, sys

def sub1(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)

def transform(h):
    h = sub1(h, "    u8  unk_5C[0x5E - 0x5C];\n", "    u16 unk_5C;\n")
    h = sub1(h, "    u8  unk_6C[0x72 - 0x6C];\n", "    s16 unk_6C;\n    u8  unk_6E[0x72 - 0x6E];\n")
    h = sub1(h, "    u8  unk_3D0[0x3D8 - 0x3D0];\n    s32 unk_3D8;\n    s32 unk_3DC;\n    s32 unk_3E0;\n    s32 unk_3E4;\n",
             "    PadState unk_3D0;              /* the pad record func_80055B60 builds for func_8001BE20 */\n")
    h = sub1(h, "    u8  unk_414[0x424 - 0x414];\n",
             "    u8  unk_414[8][2];             /* func_80055B60: 8 (target id, count) pairs */\n")
    h = sub1(h, "    u8  unk_43E[0x440 - 0x43E];\n", "    s16 unk_43E;\n")
    ps_start = h.index("/* Pad input record (0x18 bytes) at 0x80102788.")
    ps_end = h.index("} PadState;")
    ps_end = h.index("\n", ps_end) + 1
    ps = h[ps_start:ps_end]
    h = h[:ps_start] + h[ps_end:]
    # drop the blank line left behind
    h = h.replace("\n\n\nextern PracticeMenuRec g_practice_menu_table[];", "\n\nextern PracticeMenuRec g_practice_menu_table[];")
    pm = h.index("/* Per-character / practice-menu record table")
    h = h[:pm] + ps + "\n" + h[pm:]
    return h

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else 'tmp/func_80055B60/inc/include/code6cac.h'
    h = open(sys.argv[2] if len(sys.argv) > 2 else 'tmp/func_80055B60/d6a/include/code6cac.h', encoding='utf-8', newline='').read()
    os.makedirs(os.path.dirname(out), exist_ok=True)
    open(out, 'w', encoding='utf-8', newline='\n').write(transform(h))
    print('hdr ok ->', out)
