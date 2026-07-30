/* REJECTED (grind s3, structural) — score 8 vs the score-2 base.
 *
 * The constant-holder axis, re-measured on the s2 tail and traced through the
 * RTL. Session 2 rejected this axis with the explanation "cse propagates the
 * constant into both uses and deletes the standalone set". THAT EXPLANATION IS
 * WRONG and the correction matters, because the real mechanism tells you what
 * to attack next:
 *
 *   base.i.combine  : the `set (reg 75) (const_int 640)` for `w = 640;` is the
 *                     FIRST insn after the initPolyG4 call — i.e. exactly where
 *                     target emits `addiu $v1,$zero,0x280`. cse keeps it (it has
 *                     a REG_EQUAL note); its only LOG_LINK is REG_DEP_ANTI on
 *                     the call.
 *   base.i.sched    : sched1 SINKS that set down to sit immediately before its
 *                     first use (the `sh 0x10` store, uid 81).
 *   base.i.lreg/greg: with a 2-insn live range, local-alloc hands pseudo 75
 *                     $v0 — the register the 240 and 128 constants already used
 *                     and released.
 *   base.i.sched2   : the set now carries REG_DEP_OUTPUT against `li $v0,128`,
 *                     so it can never be hoisted above `sb $v0,0xE`. Final
 *                     position: block idx 7, mid-RGB-block. Score 8.
 *
 * So a holder does NOT fail because the source-level set disappears; it fails
 * because sched1 collapses its live range and the resulting hard-register reuse
 * pins it. Any future 640-early attempt has to survive sched1's sink, not cse.
 * (Diagnostic proof that the register is the binding constraint:
 * tmp/grind/func_8003553C/s3/forms3/I_pin_v1_DIAGNOSTIC.c — pinning w to $3
 * puts EVERY store in target's slot, including the sh 0x10 our clean form
 * misplaces. Pins are diagnostic-only and never committable.)
 */
void func_8003553C(void) {
    u8 *p;
    u8 *q;
    u32 *ot;
    s16 w;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    w = 640;
    *(s16 *)(p + 0x1A) = 240;
    *(s16 *)(p + 0x22) = 240;
    p[4] = 0;
    p[5] = 0;
    p[6] = 0x80;
    p[0xC] = 0;
    p[0xD] = 0;
    p[0xE] = 0x80;
    p[0x14] = 0;
    p[0x15] = 0;
    p[0x16] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x1E] = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0x10) = w;
    *(s16 *)(p + 0x12) = 0;
    *(s16 *)(p + 0x18) = 0;
    *(s16 *)(p + 0x20) = w;
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
}
