typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S_753D8;

extern u8 *D_800A36A0;
extern s32 D_800A374C;
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 rsin(s32);

/* Per-player gauge draw: fills the 0x2C-byte sprite descriptor `s` (the same
   descriptor shape func_8006DD94 / func_80069F80 hand to func_8007352C) for the
   player's gauge frame, two fixed sub-elements from the 0x14 table, and two
   layered elements from the 0x2C table, emitting a SetDrawMode prim after each
   group. x position is arg1 * 240 (player 1 sits one screen-half right); the
   frame's y is the per-player 0x68 counter * 90 plus the per-player s16 at
   0x42; the frame colour breathes with rsin of the 5-bit timer at 0x34. */
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
    u8 *base;

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
    base = D_800A36A0;
    s.sp34 = *(u8 *)(base + arg1 + 0x68) * 90 + *(s16 *)(base + (arg1 << 2) + 0x42);
    c = ((rsin(((*(u16 *)(base + 0x34) & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
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
    AddPrim(D_800A374C + s.sp2C * 4, arg0[6]);
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
        AddPrim(D_800A374C + s.sp2C * 4, arg0[6]);
        i++;
        arg0[6] += 0xC;
    } while (i < 2);
}
