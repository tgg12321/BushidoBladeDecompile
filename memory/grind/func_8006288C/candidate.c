/* func_8006288C — MATCHING form; found in grind session 2, re-verified
   unchanged in grind session 3 (2026-08-13). Session 2 was discarded by the
   driver on a self_vet.md FORMAT defect (multi-line SCOPE quotes); session 3
   re-applied this exact body to src/text1b.c, re-measured 0, and rewrote
   self_vet.md with single-line verbatim SCOPE quotes + file:line precedents.
   honest sandbox distance (--disable all): 0   (s1 floor was 2, pre-grind 23)
   52/52 instructions, all 9 register assignments, full init-block order.

   Two levers, both structural:
     1. [s1] Spell the slot scan as a real do/while with an early `goto out;`
        out of the taken-slot body (instead of a label + `goto loop_top` with
        the latch in an else arm). This moves the latch block and the loop
        notes, which changes every pseudo's reg_live_length, which permutes
        global.c's allocno_compare sort — 3/9 -> 9/9 target registers.
     2. [s2] Move `D_800F1138 = 1;` to the FRONT of the init block and put
        `one = 1;` immediately after `i = 0;`. s1's H6 could never place the
        constant holder early because the holder always landed BEFORE the
        store, and cse then folded the two constant-1 materialisations (41
        insns instead of 42). With the store first, the holder is still after
        it (no fold) but its RTL LUID is near the top of the init block, and
        sched.c's first pass emits `addiu $t3,$zero,1` at slot 2 — the target
        slot. Result: distance 2 -> 0.

   The `one` holder is FAKE-annotated in src/text1b.c: a literal `1 << i`
   makes the shift base loop-invariant, loop.c hoists its (set reg 1) into the
   TAIL of the preheader, and every one of the 10 literal-1 init orderings /
   loop spellings measured in s2 sweep set 6 parks the constant at slot 6. */
s32 func_8006288C(void) {
    extern s32 D_800A3460;
    extern s32 D_800A347C;
    extern s32 D_800A3478;
    extern s16 D_800F0C04;
    extern s32 D_800F0FB8;
    extern s32 D_800F0FBC;
    extern s32 D_800F0FC0;
    extern s32 D_800F10A0;
    extern s32 D_800F10A2;
    extern s32 D_800F10A4;
    extern s32 D_800F1138;
    /* FAKE: `one` is an opaque holder for the constant 1 rather than a literal
       `1 << i`; mechanism: with a literal the shift base is loop-invariant and
       loop.c hoists its `(set reg 1)` into the preheader TAIL, so sched.c's
       first pass parks `addiu $t3,$zero,1` at init-block slot 6 instead of the
       target's slot 2. Lever-exhaustion: memory/grind/func_8006288C/
       hypotheses.md H6 (s1) + H7/H8 (s2) — every literal-1 init-block ordering
       and every scalar-type permutation measured, all leave the constant at
       slot 6. */
    s32 one;
    s16 *flag_p;
    s32 *src_a;
    u16 *src_b;
    s32 i;
    s32 off_s32;
    s32 off_s16;
    s32 mask;

    D_800F1138 = 1;
    i = 0;
    one = 1;
    flag_p = &D_800F0C04;
    off_s16 = 0;
    off_s32 = 0;
    src_a = (s32 *)D_800A347C;
    src_b = (u16 *)D_800A3478;
    do {
        mask = one << i;
        if (!(D_800A3460 & mask)) {
            *(s32 *)((s32)&D_800F0FB8 + off_s32) = src_a[0];
            *(s32 *)((s32)&D_800F0FBC + off_s32) = src_a[1];
            *(s32 *)((s32)&D_800F0FC0 + off_s32) = src_a[2];
            *(u16 *)((s32)&D_800F10A0 + off_s16) = src_b[0];
            *(u16 *)((s32)&D_800F10A2 + off_s16) = src_b[1];
            D_800A3460 |= mask;
            *(u16 *)((s32)&D_800F10A4 + off_s16) = src_b[2];
            *flag_p = 0;
            goto out;
        }
        flag_p++;
        off_s16 += 8;
        i++;
        off_s32 += 0xC;
    } while (i < 6);
out:
    return 1;
}
