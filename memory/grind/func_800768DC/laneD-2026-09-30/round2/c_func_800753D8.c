void func_800753D8(s32 *arg0, s32 arg1) {
    S_753D8 s;
    s32 *tbl;
    s16 i;
    s32 body;
    s32 c;
    /* FAKE: constant-holder — the 0 passed to both func_8006E480 calls is
       kept live in callee-save $s5 (target: `addu $s5,$zero,$zero` in the
       prologue, `addu $a1,$s5,$zero` at both call sites) instead of being
       re-materialized as `li $a1,0`; mechanism: global.c allocates the
       once-set constant pseudo a callee-save because it crosses calls and
       cse.c only folds the constant within the entry extended basic block;
       lever-exhaustion: memory/grind/func_800753D8/hypotheses.md H2
       (inline literal 0 measured 70 vs 61,
       rejected/literal-zero-arg-no-holder-score70.c).
       SOTN ships this exact shape: src/dra/7879C.c:2067 `s32 zero = 0;`. */
    s32 zero;
    SelWork *base;

    zero = 0;
    s.sp28 = 0;
    if (arg1 != 0) {
        s.sp2C = 0x16;
    } else {
        s.sp2C = 0xC;
    }
    tbl = *(s32 **)(arg0[0] + 0x2C);
    s.sp18 = tbl[arg1 + 2];
    s.sp30 = arg1 * 240;
    body = s.sp18 + 0xC;
    s.sp1C = body;
    base = SELWORK;
    s.sp34 = base->f68[arg1] * 90 + base->f40[arg1][1];
    c = ((rsin(((base->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
    s.sp43 = c;
    s.sp42 = c;
    s.sp41 = c;
    s.sp40 = 1;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    s.sp40 = 0;
    tbl = *(s32 **)(arg0[0] + 0x14);
    i = 0;
    s.sp18 = tbl[0];
    s.sp30 = arg1 * 240 + 0x9D;
    s.sp34 = 0x36;
    body = s.sp18 + 0xC;
    s.sp1C = body;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    s.sp34 = 0x90;
    s.sp1C += *(u8 *)(s.sp18 + 2) << 3;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;
    tbl = *(s32 **)(arg0[0] + 0x2C);
    do {
        s.sp18 = tbl[i];
        s.sp30 = arg1 * 240;
        s.sp34 = 0;
        body = s.sp18 + 0xC;
        if (arg1 != 0) {
            s.sp2C = 0x16;
        } else {
            s.sp2C = 0xC;
        }
        s.sp1C = body;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
        AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
        i++;
        arg0[6] += 0xC;
    } while (i < 2);
}
