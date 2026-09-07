/*
 * CANDIDATE -- func_80035280 (src/code6cac_b.c) -- s1 (2026-09-07, recon)
 *
 * MEASUREMENTS THIS SESSION (chassis: HEAD, INCLUDE_ASM baseline = 108 / no C body):
 *   THIS body (vC), dropped over `INCLUDE_ASM("asm/funcs", func_80035280);`
 *   with no other edit anywhere in the tree:
 *       sandbox func_80035280 --disable all = 63   (target 108, build 109)
 *   Sibling banked form vB (rejected/s1-walker-two-IV-floor56.c), same flag
 *   block but the loop-2 walker spelled `base`/`base += 8` instead of
 *   `s = base + i * 8`:
 *       sandbox = 56  (target 108, build 111)   <- the SESSION FLOOR
 *   vB scores lower but is STRUCTURALLY WORSE: it makes GCC create a second
 *   induction variable (one giv for `*base`, one for `*(s32*)(base+4)`), which
 *   is 2 insns the target does not have.  vC is the correct structural base to
 *   resume from; its 63 is register-NAMING distance, not shape distance.
 *
 * WHAT IS LEFT, EXACTLY.  ONE lever.  Proven by the diagnostic form
 * rejected/s1-diagnostic-real-call-in-loop2-adds-jal.c: with a real call placed
 * inside loop 2 (`while (1) { ...; i += 1; if (i >= 3) { func_800344B4(); break; } }`)
 * the ENTIRE loop-2 body becomes instruction-for-instruction identical to the
 * target -- preheader `move a3,zero / lui 0x8888 / ori / lui %hi(D_80106A58) /
 * addiu / move`, then in-loop `lui v0,0x91a2 / lw v1,4(base) / ori v0,0xb3c5 /
 * mult / mfhi / addu / sra 10 / sra 31 / subu / sb 0x21(dst)` -- against the
 * target's asm/funcs/func_80035280.s:25B18-25B54.  That body measures 110
 * insns (108 + the jal + its delay slot) and score 57.  Remove the two call
 * insns and it is 108 == 108.  The call is NOT admissible here (this function
 * has no callee to make in loop 2 -- unlike sibling func_8003C714, whose match
 * was closed exactly this way, decisions.md 2026-09-05 final call PASS).
 *
 * WHY THE CALL MATTERS -- loop.c:1631, read this session, not inherited:
 *   move_movables hoists a loop-invariant const load iff
 *       already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count
 *   loop.c:532  threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)
 *   loop.c:897  savings = 1 for a `move_insn` (constant-load) movable -- HARD
 *               CODED, so savings can never be 0.
 *   Measured for THIS function in tmp/grind/func_80035280/dumps/code6cac_b.loop:
 *       "Loop from 109 to 255: 62 real insns."
 *       "Insn 135: regno 112 (life 1), move-insn savings 1  moved to 273"  <- 0x91A2B3C5
 *       "Insn 153: regno 118 (life 35), move-insn savings 1 moved to 275"  <- 0x88888889
 *   No call  -> threshold 122 -> 122*1*1 = 122 >= 62 -> BOTH hoisted (wrong).
 *   With call-> threshold  61 ->  61*1*1 =  61 <  62 -> 0x91A2B3C5 STAYS in the
 *               loop (right), while 0x88888889 (life 35) still hoists (right).
 *   loop.c:1609 also doubles insn_count when moved_once[regno] is already set,
 *   so insn_count 62 -> 124 > 122 would ALSO leave it in the loop.  That is the
 *   cheapest untried route: it needs only that pseudo 112 be the destination of
 *   an earlier already-moved movable in the same loop.  See hypotheses.md F1.
 *
 * THE FLAG-BLOCK FIX (`src[3]`, not `*f`) IS LOAD-BEARING AND IS A DATA-MODEL
 * FACT, not a codegen trick.  The target emits THREE `lbu 0(a1)` of
 * D_80106A73 and THREE `sw 0x20(t0)`; a plain `*f` deref emits one of each
 * (build 107 vs target 108, flag block 20 insns vs 24).  Mechanism, read this
 * session: expr.c:4567-4577 sets MEM_IN_STRUCT_P on an INDIRECT_REF only when
 * the address tree is a PLUS_EXPR (or is &<aggregate>, or the type is
 * aggregate).  cse.c:7565-7576 marks the `p[8] = v` store `nonscalar` (never
 * `all`, because its address IS a PLUS and its mode is not QImode), and
 * cse.c:1701 invalidate_memory then drops only entries with p->in_struct set
 * or a varying address -- and cse_rtx_addr_varies_p returns 0 for a MEM whose
 * address register has a known constant qty, which `&D_80106A73` is.  So a
 * non-in-struct flag load SURVIVES all three stores and is CSE'd, and the two
 * now-redundant stores die.  Spelling the read so the address tree is a
 * PLUS_EXPR (`src[3]`, src = &D_80106A73 - 3) restores all six instructions.
 * The honest declaration form of the same fact is an aggregate covering
 * 0x80106A70..0x80106A73 (see the DATA MODEL / OBJECT MODEL entry in
 * evidence.md); `src[3]` is the TU-local measurement spelling and should be
 * replaced by the declaration before any submission.
 */
void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    u8 *dst;
    s32 i;
    s32 v;
    s32 a;
    s32 b;
    s32 c;

    p = func_80077D00();
    f = &D_80106A73;
    src = f - 3;
    v = p[8];
    v = (v & ~1) | (src[3] & 1);
    p[8] = v;
    v = (v & ~2) | (src[3] & 2);
    p[8] = v;
    v = (v & ~4) | (src[3] & 4);
    p[8] = v;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[0x17 + i] = *src;
        ((u8 *)p)[0x1D + i] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    for (i = 0; i < 3; i++) {
        u8 *s = base + i * 8;
        dst = (u8 *)p + i * 4;
        a = *(s32 *)(s + 4);
        a = a / 1800;
        dst[0x21] = a;
        b = *(s32 *)(s + 4);
        b = b / 30;
        b = b % 60;
        dst[0x22] = b;
        c = *(s32 *)(s + 4);
        c = c % 30;
        c = c * 100;
        c = c / 30;
        dst[0x23] = c;
        v = *s;
        dst[0x24] = v;
    }
}
