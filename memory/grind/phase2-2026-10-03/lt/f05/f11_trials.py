#!/usr/bin/env python3
# F11 (368E4 stream walkers): one bounded round of typed trials per body (team-lead, 2026-10-06).
# Each trial reads the stream through a typed record (u16 members read through (s16) where the code
# sign-extends: lhu + sll / sra), declared inside the body so the candidate compiles against the
# unchanged headers. Writes tmp/w2/f11/<func>.c and tmp/w2/f11/list.txt ("<func> <file>") for
# run_f11.ps1 (engine sandbox <func> --disable all --candidate <file>).
import os

E12 = "    typedef struct { s32 off; u16 x0, y0, x1, y1; } Entry; /* the 12-byte image entry */\n"
TRIALS = {
    "func_800483DC": """void func_800483DC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
""" + E12 + """    s32 *list;
    s32 count;
    Entry *e;
    list = (s32 *)(arg0 + ((s32 *)arg0)[(s16)arg1]);
    count = *list;
    e = (Entry *)(list + 1);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            func_800484A0((u8 *)(arg0 + e->off), (s16)e->x1 + sx_arg2, (s16)e->y1 + sx_arg3);
            e++;
        } while ((count--) != 0);
    }
}
""",
    "func_800484A0": """void func_800484A0(u8 *arg0, s16 arg1, s16 arg2) {
    typedef struct { u8 id; u8 pad1[3]; s32 flags; s32 len; s16 x, y; u16 w, h; } Tim;
    Tim *t = (Tim *)arg0;
    s16 rect[4];
    s16 buf[512];
    if (t->id != 0x10) return;
    if ((t->flags & 8) == 0) return;
    rect[0] = arg1;
    rect[1] = arg2;
    rect[3] = t->h;
    rect[2] = t->w;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((s32)(t + 1), rect[2], (s32)buf);
        LoadImage(rect, (s32)buf);
        return;
    }
    LoadImage(rect, (s32)(t + 1));
}
""",
    "func_80048530": """s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
""" + E12 + """    s32 *list;
    s32 count;
    Entry *e;
    list = (s32 *)(arg0 + ((s32 *)arg0)[arg1]);
    count = *list;
    if (arg2 >= (u32)count) return -1;
    e = (Entry *)(list + 1) + arg2;
    func_800485EC(arg0 + e->off, arg3, (s16)e->x0, (s16)e->y0, (s16)e->x1, (s16)e->y1);
    return count;
}
""",
}
WALK = """void NAME(s32 arg0, s32 arg1ARGS)
{
""" + E12 + """    s32 *list;
    s32 count;
    Entry *e;
    list = (s32 *)(arg0 + ((s32 *)arg0)[(s16)arg1]);
    count = *list;
    e = (Entry *)(list + 1);
    if (count != 0) {
        count--;
        do {
            func_800482C8(arg0 + e->off, CALL);
            e++;
        } while ((count--) != 0);
    }
}
"""
TRIALS["func_80047EE8"] = WALK.replace("NAME", "func_80047EE8").replace("ARGS", "").replace(
    "CALL", "(s16)e->x0, (s16)e->y0, (s16)e->x1, (s16)e->y1")
TRIALS["func_80047FBC"] = WALK.replace("NAME", "func_80047FBC").replace("ARGS", ", s16 arg2, s16 arg3").replace(
    "CALL", "(s16)e->x0 + arg2, (s16)e->y0 + arg3, (s16)e->x1 + arg2, (s16)e->y1 + arg3")
TRIALS["func_800480C0"] = WALK.replace("NAME", "func_800480C0").replace(
    "ARGS", ", s16 arg2, s16 arg3, s16 arg4, s16 arg5").replace(
    "CALL", "(s16)e->x0 + arg2, (s16)e->y0 + arg3, (s16)e->x1 + arg4, (s16)e->y1 + arg5")
TRIALS["func_800481E8"] = WALK.replace("NAME", "func_800481E8").replace("ARGS", "").replace(
    "CALL", "(s16)e->x0, (s16)e->y0, (s16)e->x1, (s16)e->x1 < 0x280 ? (s16)(e->y1 + 1) : (s16)e->y1")
TRIALS["func_800482C8"] = """void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {
    typedef struct { u8 id; u8 pad1[3]; s32 flags; } TimHead;
    typedef struct { u32 len; s16 x, y; u16 w, h; } TimBlock;
    TimHead *t = (TimHead *)arg0;
    TimBlock *clut = (TimBlock *)(t + 1);
    TimBlock *img = clut;
    s16 rect[4];
    s16 buf[512];
    if (t->id != 0x10) return;
    if (t->flags & 8) {
        img = (TimBlock *)((u8 *)clut + ((clut->len >> 2) << 2));
    }
    rect[0] = arg1;
    rect[1] = arg2;
    rect[3] = img->h;
    rect[2] = img->w;
    LoadImage(rect, (s32 *)(img + 1));
    if (!(t->flags & 8)) return;
    rect[0] = arg3;
    rect[1] = arg4;
    rect[3] = clut->h;
    rect[2] = clut->w;
    if (func_800486FC() != 0) {
        math_GrayscaleRgb555((s32)(clut + 1), rect[2], (s32)buf);
        LoadImage(rect, (s32 *)buf);
        DrawSync(0);
        return;
    }
    LoadImage(rect, (s32 *)(clut + 1));
}
"""
os.makedirs("tmp/w2/f11", exist_ok=True)
rows = []
for f, b in TRIALS.items():
    p = "tmp/w2/f11/%s.c" % f
    open(p, "w", encoding="utf-8", newline="\n").write(b)
    rows.append("%s %s" % (f, p))
open("tmp/w2/f11/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
print("\n".join(rows))
