/* func_80045878 (src/text1a_c.c) -- BEST FORM, s10 (2026-08-30).
 * sandbox --disable all = 0, build_insns 108 == target 108, rules_dropped 0.
 * THE FUNCTION IS BYTE-EXACT THIS SESSION.  It is NOT submitted as
 * candidate-ready because ONE construct in it (the fresh carrier local `c`)
 * has no citable sanctioned family -- see memory/grind/func_80045878/self_vet.md
 * and the ruling-request outcome.  Do not weaken the vet to ship it; get the
 * ruling, then submit unchanged.
 *
 * WHAT CHANGED vs the s9 form (floor 4 -> 0), two independent edits:
 *
 * (1) GAP A (idx 29<->32, the else-arm `addiu s3,s2,3` placement) is CLOSED,
 *     and it was never a scheduler problem at all -- it was a REORG.C
 *     DELAY-SLOT DUPLICATION being mis-read as a two-arm recompute.
 *     Evidence: in asm/funcs/func_80045878.s the then-arm `j` at idx 15
 *     targets func+0x84 == idx 33, i.e. it SKIPS idx 32.  reorg.c copied the
 *     first insn of the join block into the `beqz` delay slot (idx 13) and
 *     advanced the jump label past the original, so the ONE source statement
 *     `s3 = a0 + 3;` materialises TWICE in the emitted stream.  Writing it as
 *     a single statement in the JOIN block (after the if/else, before
 *     `if (func_8004574C(s3))`) reproduces both insns and the exact order.
 *     Nine sessions had spelled it as an assignment in BOTH arms, which puts
 *     the else-arm addiu at the WRONG END of its block and cannot be fixed by
 *     scheduling levers (see hypotheses.md: source order in the else block is
 *     byte-neutral, measured three ways this session).
 *     Hoisting it ABOVE the first if instead gives 107 insns / score 3 (the
 *     delay-slot copy is then the only materialisation) -- the statement must
 *     sit in the JOIN block, not the entry block.
 *
 * (2) GAP B (idx 89<->90, `move v0,s1` vs `addiu v1,s2,3`) is CLOSED by
 *     giving the tail base pointer a SECOND SET, which denies it the
 *     birthing_insn_p LAUNCH_PRIORITY bump.  Mechanism, read out of the
 *     cc1 -dS trace this session (tmp/grind/func_80045878/s10/, block 13):
 *       ";; ready list at T-8: 219 (7f000001) 222 (1), now 219 222"
 *     insn 219 = the base copy, insn 222 = `c = a0 + 3`.  sched.c:2543
 *     adjust_priority sets a ready predecessor to max_priority (== 0x7f000001,
 *     sched.c:187/4049) iff sched.c:2570 birthing_insn_p holds, i.e. the dest
 *     is live AND reg_n_sets[dest] == 1.  With a single-set base the bump
 *     lands on 219, it is picked at T-8 and therefore EMITTED FIRST at idx 89.
 *     Reusing the EXISTING local `v0` (the func_8004574C result pointer, dead
 *     after the first if) as the tail base makes reg_n_sets == 2, the bump is
 *     denied, both insns sit at priority 1, rank_for_schedule falls through to
 *     INSN_LUID (sched.c, "sort by INSN_LUID ... to make the sort stable"),
 *     222 has the higher LUID, schedule_select finds no hazard difference
 *     between two ALU insns so best_insn stays 0 -- and 222 is picked at T-8
 *     and emitted at idx 90.  Target order, zero extra instructions.
 *     Every OTHER way of giving the base a second set costs an instruction
 *     (four spellings measured this session, all 109-110 insns; see
 *     rejected/).  `v0` is the only existing local whose value is dead at the
 *     tail AND whose merged allocno does not cross a call.
 *
 * WHY THE CARRIER `c` IS STILL REQUIRED (unchanged from s9, re-measured on
 * THIS chassis): the two tail scratch values must live in a pseudo that is
 * (a) mentioned in >= 2 basic blocks, so local_alloc (local-alloc.c:472 only
 * claims reg_basic_block[i] >= 0) does not hand one of them $v0 ahead of the
 * base, and (b) never live across a call, or global.c seats it callee-save.
 * Dropping `c` entirely: 110 insns / score 14 (rejected/s10-no-carrier-*.c).
 * Borrowing the existing `s0` instead: 108 insns / score 6 -- callee-save seat
 * (`addiu s0,s2,3`).  Every existing local and parameter in this function
 * (s0, s1, s3, a1, a2) crosses a call; the ONLY call-free multi-block anchors
 * are the two third-if condition reads, and target seats the s1[4] temp in
 * $v0, so the carrier must be the s1[3] temp -- which is what `c` is.
 *
 * s10b (2026-08-30, rederive) NOTE: callee names in this file were updated to
 * the names src/text1a_c.c actually declares today (func_80045600 /
 * func_80045230 / func_80045694); the earlier spelling in this file
 * (saSeMain_*/saTan5TakeGetPos_*) no longer compiles.  Re-measured verbatim
 * this session: score 0, 108/108 insns, rules_dropped 0.
 *
 * s10b also found a SECOND byte-exact form, memory/grind/func_80045878/
 * alt_anchor_e1.c, which anchors the carrier on the first-if else arm's
 * `0x1A88 + (s32)s1` call argument and leaves the third `if` condition as
 * ordinary C.  It carries the same (Judge-FAILed) multi-write fresh local, so
 * neither form is submittable until that family question is ruled.
 *
 * Apply verbatim over the INCLUDE_ASM line in src/text1a_c.c.
 */
void func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s16 *v0;
    s16 *s1;
    s32 s0;
    s32 c;
    v0 = (s16 *) func_8004574C(a0);
    if (v0 != 0) {
        s1 = (s16 *) ((s32 *) v0)[1];
    } else {
        s1 = (s16 *) func_800455AC(a0);
        func_80045600(a0, 0x1A88 + ((s32) s1));
        func_80045230(0);
        func_80045694(a0, (s32) (&func_80045AA4));
        s1[4] = -1;
        s1[3] = 0;
    }
    s3 = a0 + 3;
    if (func_8004574C(s3) != 0) {
        func_800400F8((s32) s1);
    }
    if (((func_8004574C(s3) != 0) && (s1[4] == a1)) && ((c = s1[3]) != (-2))) {
        s1[3] = 0;
    } else {
        *((s32 *) (((s32) s1) + 0x20)) = a2;
        s0 = (s32) func_800455AC(s3);
        *((s32 *) (((s32) s1) + 0x1C)) = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32) ((s32 *) s0)[*((s32 *) s0)]) >> 2) << 2);
            func_80045230(s0);
        }
        func_80045600(s3, s0);
        func_80045694(s3, (s32) (&func_80045AA4));
        s1[3] = 1;
        *((s32 *) (((s32) s1) + 0x24)) = 0;
        *((s32 *) s1) = 0;
    }
    v0 = s1;
    c = a0 + 3;
    v0[11] = c;
    v0[2] = a0;
    v0[4] = a1;
    v0[10] = a0;
    v0[8] = a0;
    c = 0x8000;
    *((s32 *) (((s32) v0) + 0x18)) = c;
}
