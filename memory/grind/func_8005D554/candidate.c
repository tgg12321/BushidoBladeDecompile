/* func_8005D554 -- BEST CLEAN FORM: honest sandbox distance 6 at 176/176 instructions,
 * frame 120 == target, register allocation byte-identical to the target (H12).
 * Measured s3b 2026-09-08 on HEAD main @ 4e7ad872 with `sandbox func_8005D554 --disable all`.
 *
 * This body REPLACES the previous candidate.c, which reached distance 0 but did so with the
 * fresh multi-write carriers `nv`/`nw` -- a construct the Judge FAILed on 2026-09-08
 * (docs/grind/decisions.md, "2026-09-08 22:31 -- func_8005D554 -- ruling ... FAIL").  That body
 * is banked verbatim at rejected/judge-failed-fresh-multiwrite-nv-nw-carrier-scores-0.c and
 * MUST NOT be resubmitted: the driver keys review verdicts by body, so it is rejected without
 * review.  The Judge's binding constraint: no fresh (invented) local may be written more than
 * once to act as a staging carrier for the a2-site base `(s32)r4 - K`, under any name.
 *
 * The whole remaining residual is 6 = two identical 3-insn rotations, one per loop half.
 * Ours emits [addiu a2,s4,-K][addiu a0,sp,16][lw v1,gp][move a1,zero]; the target emits
 * [addiu a0,sp,16][addu a1,zero,zero][lw v1,gp][addiu a2,s4,-K]
 * (asm/funcs/func_8005D554.s:4DEB4-4DEC0).  Everything else in the function matches.
 *
 * s4 (enumerate, 2026-09-08, HEAD main @ f95f6b8a): floor RE-MEASURED at 6/176 on this body,
 * and 1,224 complete spellings were swept around it (tmp/grind/func_8005D554/enum{1,2,3},
 * histograms in tmp/grind/func_8005D554/s4/sweep{1,2,3}.json).  Nothing scores below 6.  The
 * a2-arithmetic spelling space is quantized to {6, 15}; the 800-form cross product of every
 * neutral axis (a0/a2 arithmetic forms x declaration scope x zero1C position x tail-store
 * moves) is 800/800 at exactly 6/176.  Do not re-sweep those axes: they are byte-level no-ops.
 *
 * s5 (synthesis, 2026-09-09, HEAD main @ 4f43e5cf): floor RE-MEASURED at 6/176 on this body.
 * The pass attribution is now DONE and it corrects s3 and s4.  The Judge-FAILed nv/nw body did
 * NOT win by reg_n_sets (its carrier is SINGLE-set after combine) and did NOT win by out-LUIDing
 * the argument loads (its base insn is born BEFORE the third rand call, i.e. with a LOWER LUID,
 * and sched1 SINKS it).  The real chain: a source-multi-set carrier is not a loop.c movable, so
 * the loop-invariant (s32)r4 - K is not hoisted; combine.c then erases the extra set, leaving a
 * single-set single-use pseudo that dies at the add; sched1 sinks its insn to just after the
 * D_800A3418 load; local-alloc seats it in caller-saved $a2.  Sixteen new spellings (s5/enum*)
 * show the direct-store vs accumulate shape is an exact byte-level no-op, that every early-birth
 * form costs +2 (LICM hoist for a fresh single-set carrier, a tenth callee-saved seat for an
 * existing local), and that freeing a pointer local buys the count back but leaves the base in a
 * callee-saved register -- structurally further from the target than this body.  Do not re-derive
 * the LUID framing or the reg_n_sets framing; see hypotheses.md H23-H26.
 *
 * s5b (synthesis, 2026-09-09, HEAD main @ 4f43e5cf): floor RE-MEASURED at 6/176 on this body.
 * 55 further spellings measured (tmp/grind/func_8005D554/s5b/, sweep{1..5}.json) retire all
 * three s5 frontier items: the 4-local per-half offset chassis is a byte-level no-op at the
 * floor (6/176) and strictly worse when early-birthed (47/178); a2-base carrier IDENTITY is
 * invisible in the accumulate shape (shared / fresh-shared / fresh-per-half all 6/176) and only
 * decides 176 vs 178 in the direct-store shape; the loop chassis is measured out (do-while ==
 * guard-duplicated while == for-break, all 6/176; label+goto 37/175); parameter and dead-local
 * carriers for the permuter s zero10-restage trick cost 1-3 insns (25-61); a zero constant
 * holder for the second call argument costs 2-3 insns (16-30). See hypotheses.md H27-H31 and
 * frontier R1-R3.  Do NOT re-sweep loop-chassis, carrier-identity or per-half-local axes.
 *
 * s6 (solver, 2026-09-09, HEAD main @ 07aadcc3): floor RE-MEASURED at 6/176 on this body.  The
 * residual is now TYPED: inverse_compose.py classify reports SCHED (176 vs 176, registers
 * identical, 6 slots reordered) -- there is no PRE-RA or RA component left at all.  A depth-1
 * perturb.py search over ALL 12,558 single atoms of sched1 block 6 finds only LUID moves of the
 * `addiu a2,s4,-K` insn (uid 211) to a slot at or after the func_80073728 call; and all 420
 * dependence-legal permutations of the eight half-1 source statements, replayed through the
 * exact sched1+sched2 funnel (self-checked against the real dump), miss the target order.  So
 * statement ORDER is closed with a predicate (sched.c:2464, the INSN_LUID tie-break); the next
 * lever has to change the dependence graph or the priorities, not the order.  Confirmed against
 * the compiler: the model-predicted best statement placement measures 6/176 like the control.
 * See hypotheses.md s6 and evidence.md s6.  Do NOT re-sweep statement placements of the a2 base.
 */
s32 func_8005D554(s32 arg0, s32 arg1) {
    extern s32 rand(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;
    S46C s;
    s32 v3;
    s32 v0;
    s32 i;
    u32 r5;
    u32 r4;
    s32 ret;
    s32 stride;
    s32 a0_offset;
    s32 a2_offset;
    s32 c100;
    s32 c1;
    s32 *p_b388;
    s32 *p_b390;
    u8 *p_b2e0;
    u8 *p_b2ec;
    s32 *base_offset;

    v3 = D_800A326C;
    v0 = v3;
    if (v3 < 0) v0 = v3 + 3;
    D_800A326C -= (v0 >> 2) * 4;

    ret = arg0;
    if (arg1 > 0) arg1 -= 1;
    i = 0;

    D_800A3418 ^= rand();
    r5 = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= rand();
    r4 = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    if (i < ((D_800A326C + 1) * 2)) {
        c100 = 0x100;
        c1 = 1;
        stride = arg1 * 0x3C;
        p_b2e0 = (u8 *)&D_8009B2E0;
        p_b2ec = p_b2e0 + 0xC;
        base_offset = (s32 *)(p_b2ec + stride);
        p_b388 = &D_8009B388;
        p_b390 = p_b388 + 2;
        do {
            s.byte28 = 0;
            s.p0 = (void *)(stride + (s32)p_b2e0);
            s.p1 = p_b388;
            D_800A3418 ^= rand();
            i += 1;
            s.c24 = c100;
            s.c20 = c100;
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x19;
            a0_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            a2_offset = (s32)r4 - 0xC;
            a2_offset += ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);

            s.byte28 = 0;
            s.c24 = c100;
            s.c20 = c100;
            s.p1 = p_b390;
            s.p0 = (void *)((u8 *)base_offset + (D_800A3418 & 1) * 0xC);
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x32;
            a0_offset += ((u32)(D_800A3418 * 0x64) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            a2_offset = (s32)r4 - 0x19;
            a2_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ret;
}
