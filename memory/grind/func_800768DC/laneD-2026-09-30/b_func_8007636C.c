void func_8007636C(s32 *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    S_80074488 s;
    s32 ot;
    s32 *table;
    /* the sprite sheet's cell array (8-byte SprtEntA cells), which starts
       just past the sheet's 12-byte SprtHdrA headers: one header on the
       single-state sheets (+0xC), three (normal, then one cursor highlight
       per player) on the highlightable ones (+0x24). D_SEL.BIN layout:
       memory/grind/func_8007636C/evidence.md "Ruling 9 re-audit". */
    s32 cells;
    s32 color;
    s16 i;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
     * func_8006E480's second argument at both call sites. Set once and live
     * past the first loop, so cse substitutes its pseudo into that loop's
     * `(s16)i < f65 + 3` entry guard (slt needs a register operand) and reload
     * rematerializes it as the target's `move t0,zero; slt` (0x800764A0); a
     * literal 0 lets combine fold the guard to a beqz (3/348). The case-2
     * sibling func_800759D0 holds this same argument's zero in $fp (asm lines
     * 20/56/334/356). Lever exhaustion: memory/grind/func_8007636C/hypotheses.md. */
    s32 mode;
    u16 idx;

    mode = 0;
    ot = 10;
    s.sp28 = 0;
    if (arg3 != 0) {
        ot = 20;
    }
    table = *(s32 **)(arg0[0] + 0x30);
    if (SELWORK->f14.half[arg3] < 4) {
        s.sp18 = table[12];
        s.sp40 = 0;
        s.sp30 = arg3 * 240;
        cells = s.sp18 + 0xC;
        s.sp1C = cells;
        s.sp34 = SELWORK->f3C.half[arg3] * 34;
        s.sp2C = ot;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, mode), 0);
        AddPrim(g_gpu_ot_ptr + ot * 4, arg0[6]);
        arg0[6] += 0xC;
    }

    s.sp40 = 0;
    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    s.sp41 = s.sp42 = s.sp43 = color;
    table = *(s32 **)(arg0[0] + 0x14);
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        s.sp40 = 0;
        s.sp18 = table[arg2[i] + 1];
        cells = s.sp18 + 0x24;
        if (SELWORK->f3C.half[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp1C = cells;
        s.sp1C += *(u8 *)(s.sp18 + 2) * 16;
        s.sp30 = arg3 * 240;
        s.sp34 = i * 34;
        s.sp2C = ot;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + 0x30);
    for (i = 0; i < SELWORK->f3C.half[arg3] + 1; i++) {
        if (SELWORK->f3C.half[arg3] != i || SELWORK->f14.half[arg3] >= 4) {
            idx = SELWORK->f7E[arg3][i];
            s.sp40 = 0;
        } else {
            idx = SELWORK->f48[arg3][SELWORK->f5C[arg3]];
            s.sp40 = 1;
        }
        s.sp18 = table[(s16)idx * 2];
        cells = s.sp18 + 0x24;
        if (SELWORK->f3C.half[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp30 = arg3 * 240;
        s.sp1C = cells;
        s.sp34 = i * 34;
        s.sp2C = ot;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        s.sp18 = table[(s16)idx * 2 + 1];
        s.sp40 = 0;
        cells = s.sp18 + 0xC;
        s.sp1C = cells;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + SELWORK->f65 * 4 + 0x20);
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        s.sp40 = 0;
        s.sp18 = table[i];
        cells = s.sp18 + 0x24;
        if (SELWORK->f3C.half[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp1C = cells;
        s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
        s.sp30 = arg3 * 240;
        s.sp34 = i * 34;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = table[1];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, mode), 0);
    AddPrim(g_gpu_ot_ptr + ot * 4, arg0[6]);
    arg0[6] += 0xC;
}
