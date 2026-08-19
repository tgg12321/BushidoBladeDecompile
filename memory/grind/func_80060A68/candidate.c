/* [s10 2026-08-19 — forensics.  READ THIS FIRST: THE BODY IN THIS FILE CHANGED.]
 *
 * WHAT CHANGED.  Until s9 this file held the "staged temp_a1" body (temp_a1 written
 * twice: once with the 0x10 pointer, once with the 0x1A/0x1C halfword — the sanctioned
 * staged-value-reused-variable construct).  That body is PRESERVED VERBATIM at
 * memory/grind/func_80060A68/prior-floor2-staged-temp_a1.c together with its full s1-s9
 * header; it is NOT rejected and still measures score 2 / build 66 / target 66.  This
 * file now holds the s10 body, which measures the SAME floor (score 2, build 66 /
 * target 66, re-measured this session) but is strictly better on two axes:
 *
 *   1. It contains NO multiply-assigned local at all.  Every local here is written
 *      exactly once and read for a real job: `p10` holds the 0x10 pointer for the +4
 *      halfword read, `temp2` holds the 0x1A halfword across the D_800A3478 store,
 *      `temp_a1` holds the 0x1C halfword and the call's second argument.  There is no
 *      staged carrier, so there is no FAKE annotation and no sanctioned-family claim to
 *      defend.  (The prior body needed the staged-temp_a1 claim.)
 *   2. Its residual is in a DIFFERENT PLACE.  For the first time on this function the
 *      whole prefix through slot 20 is byte-identical to target, INCLUDING the three
 *      contested loads that every session since s1 has been fighting:
 *          slot 10  lw v0,12(v1)      slot 11  lw a0,12(v1)      slot 12  lw a1,16(v1)
 *      The sched2 rank_for_schedule LUID tie at T-45 (sched.c:2464) that s2-s9 attributed
 *      the residual to is GONE from this body.  The remaining 2-instruction gap is:
 *          target  ... lhu v0,0(a0) / lw a0,16(v1) / sh v0,24(v1) / lhu a0,2(a0) ...
 *          ours    ... lhu v0,0(a1) / nop          / sh v0,24(v1) / lhu a0,2(a0) ...
 *      i.e. the 0x18 halfword read is CSE-folded onto p10's register (a1) instead of
 *      re-loading 16(v1), and the load that target puts in the slot-23 load-delay slot is
 *      therefore missing and shows up as a nop.
 *
 * THE MECHANISM, AND WHY IT IS A CONSERVATION LAW (this is the s10 finding).
 * `p10` is single-set, so its load (insn 39) takes sched1's birthing_insn_p
 * LAUNCH_PRIORITY bump (sched.c:2504-2535) and is chained LATE, which is what finally
 * gives sched2 the LUID order it needs to emit copy 2's address load at slot 11 and the
 * staged load at slot 12.  But a bumped load is chained next to its FIRST consumer, and
 * with three independent reads of *(s32 *)(outer + 0x10) p10's only consumer is the +4
 * read at target slot 29 — sched1 then chains insn 39 between insns 56 and 58, sched2
 * reaches T-30 with insn 39 as the sole ready insn, and the load overshoots to slot 25
 * (measured: rejected/split-stage-single-set-bumped-overshoots-slot22-score5.c and the
 * whole r2/r3/r4/t1/u1/u2/u3 family, all score 5 / 67 insns).  Moving the p10 statement
 * BELOW the copy-3 store removes the memory fence between p10's load and the 0x18 read,
 * so cse folds the 0x18 read onto p10 — which gives insn 39 an EARLY consumer (slot 22),
 * unpins it, and lands it exactly on target's slot 12.  That is the trade this body makes:
 *
 *     3 independent 16(v1) loads  <=>  p10 has no early consumer  <=>  sched2 T-30 pin
 *     p10 has an early consumer   <=>  one of the 3 loads is CSE-folded away  <=>  a nop
 *
 * Nine source positions for the p10 statement were measured this session (r1..r9) and
 * every one of them sits on one side or the other of that equivalence; so does every
 * attempt to re-separate the loads with a store (t1: split copy 3 so its store falls
 * between p10 and the 0x18 read -> 3 loads and the pin returns, score 5; t2: hoist the
 * D_800A3478 gp store above the 0x1A read -> score 8).
 *
 * WHERE THE NEXT SESSION SHOULD PUSH.  The r4-shape pin has a named and now dump-verified
 * proximate cause that is NOT scheduling at all: in the 3-load bodies reload gives insn 56
 * (the 0x1A read's address load) hard register v0, and insn 53 is `sh v0,24(v1)`, so 53
 * carries a write-after-read anti-dependence on 56.  sched2 therefore cannot release 53
 * until 56 is scheduled, T-30 comes up with only insn 39 ready, and 39 is forced into it
 * (n9b/trace_sched2.txt T-30 `;; ready list at T-30: 39 (3)`).  In the target — and in the
 * prior staged body — insn 56 gets a0 instead, 53 IS ready at T-30 (n9/trace_sched2.txt
 * `;; ready list at T-30: 39 (3) 53 (8), now 53`), 53 wins on priority 8 vs 3, and insn 39
 * survives to a much later cycle.  So the 3-load body closes the function IF insn 56's
 * destination can be steered from v0 to a0 by honest C.  Three type-level attempts failed
 * this session (s32 temp2, s32 temp_a1, a fresh single-set `q` local for the 0x1A address
 * — all score 5); the next lever is a register-allocation one, read out of .lreg/.greg
 * rather than guessed.
 *
 * INTEGRATION HAZARD (unchanged, still live and load-bearing).  This body's first body
 * instruction is `lhu $2,0($3)`, while asmfix.txt:109 (a delete_between anchored on
 * `^lhu\t\$4,0\(\$3\)$`) and asmfix.txt:110 (a 43-instruction insert_before splicing the
 * whole target body) are written against the HEAD body.  Applying this body to
 * src/text1b.c WITHOUT retiring both asmfix rules in the same change silently duplicates
 * the function in a full build.  src/text1b.c was reverted to HEAD before this session
 * finished.
 *
 * DISPOSITION.  The function is ACTIVE, not parked.  s8's terminal-disposition claim was
 * voided by the driver (wrong modality) and s9 reverted the decisions.md entry; nothing
 * this session changes that.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    p10 = *(s32 *)(outer + 0x10);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
