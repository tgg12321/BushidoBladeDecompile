/* func_800858D0 (title_mv_exec2) — grind candidate, floor 15 (s1, 2026-08-20,
 * post-migration chassis, build_insns 72 == target 72, frame 96, saves 3).
 * This is the old floor-22 form WITH the banked A+B statement moves folded in:
 *   A: buf[1] = 0x60093;  moved BEFORE  var_s0 = 0;
 *   B: *(s16 *)((u8 *)buf + 0x3C) = 0x4000;  moved to the END of the init block.
 * Measured this session: sandbox --disable all = 15 (was 22 without A+B).
 * The multi-set scratch `t` (defeat-licm-hoist-var-reuse, shape shipped in
 * func_8003DBE4) is load-bearing: it is the only known honest spelling that
 * produces ZERO movables in loop.c (dump-verified: t = reg 75 appears only as
 * "possible biv, discarded"), matching target's nothing-hoisted profile.
 * Residual 15 = (a) rotation identity: reorg steals OUR li 24 into the
 * preheader/loop-back delay slot where target steals addiu a0,sp,16;
 * (b) mid-loop cluster {li 1, addiu a0, sllv} early (after store #1) vs
 * target's late (just before jal); (c) register naming: idx a1 vs v1,
 * the 24 in v1 vs a1, the 1/shift-result in v0 vs a1/v1.
 * Apply over `INCLUDE_ASM("asm/funcs", func_800858D0);` in src/main.c
 * (externs at src/main.c:848-855 already present). */
void func_800858D0(void) {
    s32 buf[16];
    s16 var_s0;
    s32 offset;
    s16 ff;
    s32 t;

    buf[1] = 0x60093;
    var_s0 = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    if (D_80101BCC != 0) {
        ff = 0xFF;
        do {
            offset = (s16)var_s0 * 54;
            t = 0x18;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = t;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)D_800F4E28 + offset) = ff;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
            t = 1;
            buf[0] = t << (s16)var_s0;
            func_8008B488(buf);
            D_8010280A = var_s0;
            func_800871D4(1);
            var_s0 = var_s0 + 1;
        } while ((s16)var_s0 < (s32)D_80101BCC);
    }
}
