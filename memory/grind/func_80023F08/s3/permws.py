"""build a permuter mini-TU for func_80023F08 from a body file.
usage: python permws.py <body.c> <workspace_dir>"""
import sys
body, ws = sys.argv[1:3]
tu = open("src/code6cac_tu2.c").read()
head = tu[:tu.index('INCLUDE_RODATA("asm/rodata", D_800100A4);')]
head = head.replace("extern void func_80023F08(s32, s32);", "extern void func_80023F08(s32, PadState *);")
protos = """
void func_800198D0(s32 obj, s32 frame, u32 *out, u16 *work);
void func_8001B690(s32 arg0, s32 arg1);
void func_8001F2E4(u8 *obj, u8 *a, u8 *b);
void func_8001F860(s16 *arg0, s32 arg1);
void func_8001F938(u8 *arg0);
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4);
extern void *func_80021424(u8 *, s32, u8 *);
extern void func_80032854(s32, s32, s32 *, s16 *);
void func_8002304C(u8 *obj, s32 *pos1, s32 *pos2, s32 *arg3);
s32 func_800233AC(u8 *arg0, s32 *arg1);
void func_80023648(u8 *arg0);
void func_800238C4(u8 *arg0);
void math_RotMatrixZYXAngles(s32 arg0, s32 arg1, s32 arg2, s16 *arg3);
void func_80023CB4(s16 *arg0, s16 arg1);
void func_80023D08(s32 arg0);
void func_80023D28(u8 *arg0);
s32 func_80023DB8(u8 *arg0);
void func_80023E40(u8 *arg0);
"""
open(f"{ws}/mini_src.c", "w", newline="\n").write(head + protos + open(body).read())
