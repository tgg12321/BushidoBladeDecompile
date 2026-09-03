/* s50 ALTERNATE EQUAL-FLOOR CHASSIS (score 2, build_insns 83) -- banked here
 * because it does NOT beat candidate.c, but it is a STRICTLY more informative
 * base for future work and its residual is disjoint from candidate.c's.
 *
 * WHAT IT IS: block 0 declares `i = a0` BEFORE `v1 = a0 << 4` (the "i-first"
 * chassis, banked dead at score 11 since s1), PLUS the loop-2 preheader is
 * respelled `v1 = a0 << 4; i = a0;` instead of `i = a0; v1 = i << 4;`.
 * The loop-2 respelling is what makes i-first survive: it restores a0's
 * fourth reg_n_refs (see MECHANISM), which s1..s49 never did.
 *
 * RESIDUAL (2 insns, both `sll` OPERANDS, and nothing else):
 *   idx  9  build `sll $v1,$s0,4`   target `sll $v1,$s2,4`   (block 0)
 *   idx 41  build `sll $v1,$s2,4`   target `sll $v1,$s0,4`   (loop-2 preheader)
 * Everything else is byte-exact: the prologue sw/move interleave, the
 * sw $s0 / move $s0,$s2 / sll ordering that candidate.c gets WRONG, the whole
 * callee-save allocation (a0->$s2, a1->$s3, sum->$s1, i->$s0, s4->$s4,
 * s5->$s5) and every stack slot.  candidate.c has the two operands RIGHT and
 * the block-0 schedule WRONG; this form has the schedule RIGHT and the two
 * operands WRONG.  The two score-2 forms are exactly complementary.
 *
 * MECHANISM (measured this session, dumps in tmp/grind/func_80045294/s50/):
 *  1. sched.c:2461-2463 rank_for_schedule -- in block 0 every insn has
 *     INSN_PRIORITY 1 (priority() at sched.c:1497 is distance from the block
 *     START via `insn_cost - 1`, so a chain of latency-1 insns is flat) and
 *     every candidate is class 3 against last_scheduled_insn (a `sw`), so the
 *     tie at T-9 falls through to `INSN_LUID (tmp) - INSN_LUID (tmp2)`.
 *     sched2 runs BACKWARDS; the ready list is descending LUID and the head is
 *     picked, so the HIGHEST-LUID insn is emitted LAST.  Target emits the sll
 *     last => the sll's RTL insn must follow `i = a0`'s => the SOURCE must
 *     declare i before v1.  (Verified: every i-first form emits the target's
 *     `sw $s0 / move $s0,$sX / sll` order; every v1-first form does not.)
 *  2. cse.c:842-857 make_regs_eqv -- on `i = a0`, i displaces a0 as
 *     qty_first_reg because i outlives the cse EBB (block 2..36) and
 *     uid_cuid[regno_last_uid[i]] > uid_cuid[regno_last_uid[a0]] (i is loop
 *     2's counter, so it always dies last).  canon_reg then rewrites any
 *     LATER `a0 << 4` to `i << 4`, so in an i-first source a0 loses the shift
 *     reference: reg_n_refs 4 -> 3.
 *  3. global.c allocno_compare -- priority is
 *     floor_log2(n_refs)*n_refs/live_length, so a0 collapses from
 *     floor_log2(4)*4 = 8 to floor_log2(3)*3 = 3.  a0 falls from 9th to last
 *     in the allocation order and the callee-saves rotate
 *     (a0 $s2->$s5, s4 $s4->$s2, s5 $s5->$s4) => score 11.
 *  4. THE FIX BANKED HERE: the loop-2 preheader is a fresh cse EBB (block
 *     70..109) in which i has been clobbered by loop 1, so an `a0 << 4`
 *     placed BEFORE that block's `i = a0` is NOT canonicalized to i and pays
 *     a0 its fourth reference back.  n_refs returns to 4, priority returns to
 *     8, and the entire target allocation is restored while block 0 keeps the
 *     i-first LUID order.  Cost: that shift now reads a0 ($s2) where the
 *     target reads i ($s0) -- one insn -- and block 0's shift reads i ($s0)
 *     where the target reads a0 ($s2) -- one insn.  Total 2.
 *
 * WHY IT IS NOT 0 (the wall, stated precisely): block 0's shift must sit
 * after `i = a0` (mechanism 1) and therefore reads i (mechanism 2), but the
 * target reads a0; and a0's compensating fourth reference must be an a0 read
 * placed before the loop-2 preheader's `i = a0` (mechanism 4), while every
 * insn the target emits at that point reads $s0.
 *
 * MEASURED s50: sandbox func_80045294 --disable all -> score=2 insns=83.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 *ptr;
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        v1 = a0 << 4;
        i = a0;
        if (i < D_800A33AC) {
            ptr = (s32 *)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *ptr += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                ptr = (s32 *)((u8 *)ptr + 0x10);
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
