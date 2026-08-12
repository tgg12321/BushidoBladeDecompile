/* MoveImage — BEST FORM as of grind session 2 (structural).
 * Honest pure-C floor: 2 (sandbox MoveImage --disable all), down from 7.
 * build_insns 49 == target_insns 49 (the H2 delay-slot steal is intact — this
 * is the standing gate: ANY form at build_insns 47/48 has lost it and is a
 * dead end no matter how good its score looks).
 *
 * Two levers are load-bearing here, both ordinary program logic:
 *
 * 1. (session 1) `D_8009BF28 = packed;` is the FIRST of the three primitive
 *    stores. That makes the packed-value computation (`sll $v0,$s1,16` / andi
 *    / or) the highest-priority work at the top of the post-guard block, so
 *    cc1's delay-branch pass (reorg.c fill_slots_from_thread, eager fill from
 *    the TAKEN thread) steals `sll $v0,$s1,16` into the guard branch's delay
 *    slot. Stealing from the taken thread requires the branch to point AT the
 *    body, which is why the guard is emitted as
 *    `bnez $v0,.L720 / j <epi> / addiu $v0,-1` — target's shape, and the two
 *    instructions that were missing at floor 21.
 *
 * 2. (session 2) The rect is walked with a post-increment pointer, so the
 *    rect[0] read happens BEFORE the D_8009BF28 store and the rect[1] read
 *    after it. Reading rect[0] ahead of the store puts the `lw $a0,0x0($s0)`
 *    where target has it (slot 5, ahead of the store group) and hands the
 *    rect[0] value $a0 — target's register. Session 1's probe G showed that
 *    reading rect[0] ahead of the store via a PLAIN named local
 *    (`src = arg0[0];`) destroys the delay-slot steal (47 insns); the walking
 *    pointer is the spelling that does it without paying that price
 *    (measured: W1/W2/W3 plain-local hoists all 21/47, W10 walking 2/49).
 *
 * Everything here is ordinary program logic. No FAKE constructs, no dead
 * stores, no pins, no volatile, no unused declarations: `packed`, `bf24`,
 * `rect`, `src`, `p` and `fn` are all written and read.
 *
 * REMAINING 2 — one instruction's position. Target emits
 *   ... sw $a0,0x0($a1) / lw $v0,0x4($s0) / addu $a3,$zero,$zero /
 *       lui $at,%hi(BF2C) / sw $v0,%lo(BF2C)($at) / lw $a0,0x18($v1) / ...
 * we emit `lw $a0,0x18($v1)` (the p[6] call argument) four slots earlier,
 * immediately after `sw $a0,0x0($a1)`. Everything else is byte-identical.
 * Diagnosis from the cc1 .sched2 dump is in evidence.md / hypotheses.md H7.
 *
 * SESSION 3 NOTE — read this before grinding from here.  The 4-slot float of
 * the p[6] load is now explained: `p[6]` is an INDIRECT_REF over a PLUS_EXPR,
 * so expr.c:4567-4577 marks it MEM_IN_STRUCT_P, and sched.c:834-839 then makes
 * it alias-EXEMPT from the `%lo(D_8009BF2C)` symbol store — nothing pins it.
 * Re-spelling that read as `q = p + 6; ... fn(*q, ...)` (a plain INDIRECT_REF)
 * restores the dependence and puts the load at TARGET'S EXACT SLOT.  That form
 * is banked as ../candidate_alt_plain_arg.c and scores 4 / 49 — a HIGHER number
 * than this file only because the residual MOVED to the dev-table load, which
 * is now the single remaining diff.  Evaluate both bases; hypotheses.md H8
 * carries the measurements.
 *
 * SESSION 4 NOTE (permuter modality).  This file is STILL the best form: the
 * floor did not move.  Four decomp-permuter campaigns totalling ~94,000
 * iterations were run and stopped in-session — random and directed
 * (PERM_LINESWAP x PERM_GENERAL) chassis, seeded from BOTH this file and
 * ../candidate_alt_plain_arg.c — and not one form beat either base.  The
 * permuter's only sub-base score class stages the dispatch argument through a
 * narrow local; it merely turns the `lw a0,24(v1)` into `lhu`/`lh` at the SAME
 * index (measured, tmp/grind/MoveImage/s4/sweep12_results.md) and is banked in
 * rejected/permuter-narrow-dispatch-argument.c.  Do not re-run a permuter
 * campaign on this function; the remaining question is forensic (hypotheses.md
 * "Frontier after session 4").
 */
extern u8 D_80015F74;
extern s32 D_8009BF24;
extern s32 D_8009BF28;
extern s32 D_8009BF2C;

s32 MoveImage(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;
    s32 *rect;
    s32 src;

    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0 || ((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    rect = arg0;
    src = *rect++;
    D_8009BF28 = packed;
    *bf24 = src;
    D_8009BF2C = *rect;
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    return fn(p[6], (s32)bf24 - 8, 0x14, 0);
}
