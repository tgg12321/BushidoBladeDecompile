/* func_80073200 - session 9 (forensics).  FLOOR 2 -> 0.  BYTE-MATCHES.
 *
 * The s8 body sat at score 2 on ONE source-level hunk pair: our
 * `addiu a0,sp,0x18` (the `(s32)&s` argument of the FIRST of the four
 * func_80073728 calls) was emitted at the head of the if/else join block,
 * where the target emits it 4th - after `sb v0,0x42(sp); li v0,0x14;
 * sb v0,0x43(sp)` (asm/funcs/func_80073200.s:59-62).
 *
 * PASS ATTRIBUTION (this session, from the instrumented cc1 -da dumps):
 *   - The decision belongs to the FIRST scheduling pass, not the second.
 *     tmp/grind/func_80073200/s9/bb4_sched.txt (basic block 4, insns
 *     134..267 - all four call groups sit in ONE block) shows the a0 set
 *     (insn 158, INSN_PRIORITY 1) sorted to ready[0] at every step from
 *     T-47 to T-53 and displaced FOUR times by "insn N has a greater
 *     potential hazard" - at T-47 (insn 150), T-48 (147), T-50 (142) and
 *     T-52 (137).  It is therefore the LAST pick of the backward pass,
 *     i.e. the FIRST insn emitted in the block.
 *   - The predicate is tools/gcc-2.7.2/sched.c:2717, inside
 *     schedule_select: within one equal-INSN_PRIORITY group it keeps the
 *     insn with the largest `potential_hazard`.  potential_hazard
 *     (sched.c:1327) returns 0 immediately for an insn that is on no
 *     function unit, and a positive value for one whose unit has
 *     max_blockage > 1.  tools/gcc-2.7.2/insn-attrtab.c:6298 gives the MIPS
 *     "memory" unit max_blockage 3; an `addiu` (attr type "arith") is on no
 *     unit at all.  So at equal priority a ready STORE always displaces a
 *     ready address-arith insn REGARDLESS of their ready-list order (the
 *     `best_insn != 0` guard means position 0 can only be kept, never
 *     promoted).  That is precisely why the s7 spelling_enum sweep (1957
 *     no-swap orderings) and the s8 sweep (all 23 orderings of this call
 *     group) all measured >= 2: reordering the C only moves the LUID
 *     tie-break in rank_for_schedule, and the tie-break is never reached.
 *   - The SECOND pass (tmp/grind/func_80073200/s9/bb4_sched2.txt) is only a
 *     stabilizer here: after reload every one of those insns writes $2, so
 *     register anti/output dependences serialize them, and
 *     rank_for_schedule's INSN_LUID tie-break ("sort by INSN_LUID ... so
 *     that we make the sort stable") simply preserves pass 1's order.
 *
 * THE CLOSER is a pass-INPUT change, not another spelling of the same
 * input: stop putting those two `sb` stores into block 4 at all.  Writing
 * `s.sp42` and `s.sp43` inside BOTH if-arms leaves sched pass 1 a block 4
 * whose priority-1 group holds no store able to displace insn 158, so the
 * a0 set is emitted exactly where the target emits it.  jump2's
 * find_cross_jump then re-merges the two identical arm tails, so the
 * duplication is invisible in the bytes: build_insns 203 == target_insns
 * 203, score 0, `sandbox --diff` reports 0 source-level and 0 operand-only
 * hunks (the 6 remaining hunks are masked branch-target relocation
 * artifacts).  Staging the colour byte through the `var_v0` local is no
 * longer needed, so that local is gone.
 *
 * MINIMAL-FORM ABLATIONS (owner prong 4, "simplest-known-form"), all
 * measured THIS session on this chassis:
 *   - drop `v12` (the 0x12 constant holder introduced at s3), storing the
 *     literal at both sites instead: score 0, build_insns 203.  It is no
 *     longer load-bearing on the s9 chassis, so it is REMOVED.
 *     (tmp/grind/func_80073200/s9/b_no_v12.c)
 *   - drop `cond`, testing D_800A3580 inline: score 7 / 205 insns - KEPT.
 *   - split the reused `s1` into three separate locals: score 15 / 201 -
 *     the single reused table pointer is KEPT.
 *   - duplicate only `s.sp42` and leave `s.sp43 = 0x14;` at the join:
 *     score 2 - one remaining store is enough to keep the potential_hazard
 *     swap firing, so BOTH stores must move into the arms.
 *     (tmp/grind/func_80073200/s9/e_sp43_at_join.c, banked as
 *     rejected/s9-dup-sp42-only-join-sp43.c)
 */
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S73200;
void func_80073200(s32 arg0) {
    S73200 s;
    s32 *ctx;
    s32 base1;
    s32 base2;
    s32 s1;
    s32 tmp;
    s32 v1;
    s32 idx;
    s32 cond;

    s.sp30 = 0;
    s.sp34 = 0;
    s.sp38 = 0x100;
    s.sp3C = 0x100;
    ctx = *(s32 **)(D_800A35A8 + 0x5C);
    base1 = *(s32 *)((s32)ctx + 0xC);
    s.sp18 = base1;
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(base1, 0), 0);
    AddPrim(D_800A374C + 0x70, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    s.sp40 = 1;
    base2 = *(s32 *)((s32)ctx + 0x10);
    s.sp18 = base2;
    s1 = base2 + 0xC;
    if (D_800A3580 < 4) {
        s.sp28 = 1;
        if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
            s.sp41 = 0xBC;
            s.sp42 = 0x78;
            s.sp43 = 0x14;
        } else {
            s.sp41 = 0xA8;
            s.sp42 = 0x6E;
            /* FAKE: `s.sp43 = 0x14;` is written in BOTH arms instead of once
             * at the join.  Byte-neutral - jump2's find_cross_jump re-merges
             * the two identical arm tails, so nothing extra materializes
             * (build_insns 203 == target_insns 203).  mechanism: the FIRST
             * scheduling pass, schedule_select's `potential_hazard` ready-list
             * swap (tools/gcc-2.7.2/sched.c:2717).  Keeping the two `sb` stores
             * out of that pass's basic block 4 removes the only ready insns
             * that could displace the `(s32)&s` argument set at its T-50 step.
             * lever-exhaustion: memory/grind/func_80073200/hypotheses.md s4-s8
             * (two permuter campaigns, the 1957-ordering spelling_enum sweep,
             * all 23 orderings of this call group, the addr-local naming). */
            s.sp43 = 0x14;
        }
        s.sp2C = 0x14;
        s.sp1C = s1;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        s.sp1C = s1 + 8;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        s.sp1C = s1 + 0x10;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 2);
        s.sp1C = s1 + 0x18;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 3);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, 0x60, 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    } else {
        s.sp41 = 0x32;
        s.sp42 = 0x32;
        s.sp43 = 0x5A;
    }
    s.sp34 = 0;
    s.sp30 = 0;
    s.sp2C = 0x12;
    s.sp28 = 0;
    tmp = *(s32 *)((s32)ctx + 0x14);
    s.sp18 = tmp;
    s1 = tmp + 0xC;
    s.sp1C = s1;
    s.sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x60), 0);
    AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
    /* FAKE: `cond` names the D_800A3580 read as a fresh once-written,
     * once-read intermediate declared ahead of the pointer bump, so the read
     * is emitted before the branch rather than after it.  mechanism: LUID
     * order into the first scheduling pass - the named read becomes the
     * delay-slot-fillable insn the target puts between `lw v0,0x18(s0)` and
     * `beqz` (asm/funcs/func_80073200.s:151-157).  lever-exhaustion:
     * memory/grind/func_80073200/hypotheses.md s5/s6 (the complementary
     * in-block form and both declaration-order sweeps measured dead); ablated
     * again THIS session - removing it regresses 0 -> 7
     * (tmp/grind/func_80073200/s9/a_no_cond.c). */
    cond = D_800A3580;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    if (cond < 2) {
        s.sp2C = 0x12;
        s.sp28 = 1;
        v1 = *(s32 *)((s32)D_800A35C4 + 8);
        idx = *(s32 *)((s32)ctx + 0x28 + (v1 % 4) * 4);
        s.sp18 = idx;
        s1 = idx + 0xC;
        s.sp1C = s1;
        s.sp20 = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x20), 0);
        AddPrim(D_800A374C + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    }
}
