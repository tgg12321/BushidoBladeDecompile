/* REJECTED (s3): routing the walker through a fresh local `w` (w=arg0; w+=off)
 * to flip the operand order via higher pseudo number BREAKS the v1-walker/
 * t0-base routing -> 46 insns (a copy coalesces away) and MORE diffs, not
 * fewer. The correct operand-order flip is `arg0 = off + base` keeping arg0
 * itself as the in-place walker (see candidate.c Lever 3). */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern void func_800485EC(s32, s32, s32, s32, s32, s32);
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base;
    s32 count;
    s32 entry;
    s32 a, b, c, d;
    s32 off;
    s32 w;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    w = arg0;
    w += off;
    count = *(s32 *)w;
    w += 4;
    if (arg2 >= (u32)count) return -1;
    w += arg2 * 0xC;
    entry = *(s32 *)w;
    w += 4;
    a = (s32)*(u16 *)w;
    w += 2;
    b = (s32)*(u16 *)w;
    w += 2;
    c = (s32)*(u16 *)w;
    w += 2;
    d = (s32)*(u16 *)w;
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}
