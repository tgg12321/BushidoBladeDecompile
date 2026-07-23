/* REJECTED (s1, 2026-07-23) — score 22, worse than the score-10 floor.
 * Two spellings both regress to 22:
 *   (A) c = (s32)*(u16*)arg0;  ...  call ... (s16)c, (s16)d   [symmetric with a,b]
 *   (B) c = *(u16*)arg0 (u32);  ...  call ... ((s32)(c<<16))>>16
 * Both give c,d the target's lhu+sll+sra shape, which needs v0+v1 as sign-extend
 * scratch. That collides with using v1 as the walking pointer, so GCC shifts the
 * whole pointer allocation: walker v1->t0, base t0->t1 — a full register-rename
 * cascade that costs far more than the 4-insn c/d gap it closes.
 *
 * WHY IT CAN'T BE FIXED THIS WAY: the target reuses the DEAD walking pointer v1
 * as the d sign-extend scratch (`lhu v0,0(v1); lhu v1,2(v1)`), so it needs no
 * extra register. Our build won't reuse the dead walker reg. Fixing c/d requires
 * making the walker provably dead-and-reusable at the d-load, NOT changing the
 * c/d value spelling. See candidate.c residual (a) and the frontier. */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base;
    s32 count;
    s32 entry;
    s32 a, b, c, d;
    s32 off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    arg0 += off;
    count = *(s32 *)arg0;
    if (arg2 >= (u32)count) return -1;
    arg0 += 4;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32)*(u16 *)arg0;
    arg0 += 2;
    b = (s32)*(u16 *)arg0;
    arg0 += 2;
    c = (s32)*(u16 *)arg0;
    d = (s32)*(u16 *)(arg0 + 2);
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}
