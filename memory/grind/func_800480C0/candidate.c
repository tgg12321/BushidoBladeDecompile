/* func_800480C0 - s3 (structural, 2026-09-02) BEST POLICY-LEGAL FORM.
 * sandbox --disable all = 20 (74/74 insns); .frame sp,56 (vars=0, regs=8,
 * args=24) vs target .frame sp,88 (vars=32, regs=8, args=24).
 *
 * TWO s2 LEDGER CORRECTIONS, both measured this session:
 *
 * 1. THE HONEST FLOOR IS 20, NOT 32. engine/volatile_cheats.py's
 *    find_dead_param_assigns strips an UN-ANNOTATED `arg0 = 0;` out of the
 *    scored TU (engine/test_engine.py:1094-1116 pins exactly this: an
 *    annotated dead param assign is bypassed, an un-annotated one is
 *    flagged/stripped). Every s1/s2 body carried the store WITHOUT a
 *    FAKE annotation comment, so the sandbox scored a body with the
 *    store deleted. Adding the annotation - which the dead-store family
 *    REQUIRES anyway (.claude/rules/dead-store-fake-exception.md) - moves the
 *    score 32 -> 20 with no other change (measured s3).
 *
 * 2. THERE IS NO CALLEE-SAVED SEAT ROTATION. s2 recorded base_addr in s6
 *    against the target's s2 and built a whole frontier (ra_solver /
 *    sched_solver) on it. That rotation only exists in the STRIPPED sandbox
 *    build, i.e. it is the downstream effect of correction 1. With the store
 *    honoured, cc1 emits `s2 = s0` (s2 = s0) and seats
 *    sx_arg2..5 in s6/s5/s4/s3 - identical to the target
 *    (tmp/grind/func_800480C0/s3/probe.s, BB2_ALLOC_DEBUG order in
 *    tmp/grind/func_800480C0/s3/alloc.err: pseudo 83 -> hardreg 18).
 *
 * RESIDUAL: purely the frame size. The instruction STREAM is byte-identical
 * to the split target listing for func_800480C0; the 20 differing insns are the 20 sp-relative
 * operands (1 addiu sp, 8 sw, 2 incoming stack-arg lw, 8 lw, 1 addiu sp).
 * The target reserves 32 bytes of `vars` at sp+0x18..0x37 that no instruction
 * touches. args+vars = 0x38 is all the binary fixes, so (args=24, vars=32) and
 * (args=56, vars=0) are indistinguishable layouts - but args=56 needs a
 * 14-word call, i.e. the fabricated-dead-call-site family REFUSED by owner
 * ruling 2026-08-17 (.claude/rules/no-new-park-categories.md:357-366).
 *
 * The pad remains banned here (Judge 2026-09-02 04:28,
 * docs/grind/decisions.md:20349); the bytes-proven pad body is preserved at
 * rejected/pad-judge-banned-2026-09-02.c. */
/* s5 (synthesis, 2026-09-02) - BODY UNCHANGED, floor re-measured 20 (74/74).
 * Two ledger corrections this session, both tree-wide measurements:
 *  1. ONE UNALLOCATED PSEUDO == 8 BYTES OF `vars`, NOT 4. s3 modelled the
 *     ST_REGS compare residue as a 4-byte alter_reg slot rounded up to 8.
 *     Correlating cc1's `# vars=` with the instrumented cc1's BB2_ALLOC_DEBUG
 *     hardreg=-1 count across all 32 TUs gives `vars = 8 * phantoms` exactly
 *     (func_80042874 6->48, func_80041E10 3->24, get_cs 2->16, ~30 at 1->8).
 *     So the target's 32 untouched bytes are FOUR phantoms, and every spelling
 *     in the 30-form ceiling supplies ONE of them.
 *  2. THE PAD IS NOT THE ONLY PRODUCER OF UNTOUCHED `vars`. 54 ordinary-C
 *     COMPLETED-C bodies on main reserve untouched frame bytes with no pad
 *     aggregate and no spill, and eight reach 16 bytes (two phantoms) -
 *     get_cs/get_ce (src/display.c:556), func_8003FECC, func_80038170,
 *     func_80040594, SsSeqCalledTbyT, _SsSeqPlay, SpuSetCommonAttr. Same
 *     ST_REGS residue class (display.lreg: Registers 85 and 99, blocks 1 and 6).
 *     What is missing here is multiplicity, not mechanism.
 *  Bound: among the 45 census candidates with no mult/div, untouched `vars`
 *  takes only {8, 16, 32} and every 32 is a sanctioned-pad function whose
 *  phantom count is ZERO (the 32 bytes are the declared array). Four
 *  folded-compare residues on a mult-free body has no precedent in this tree.
 *  Instruments: tmp/grind/func_800480C0/s5/{census.sh,census2.py,census3.py,
 *  alloc_tu.sh,dump2.sh}. */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base_addr value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
