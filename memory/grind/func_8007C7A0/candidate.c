/* func_8007C7A0 — MATCHED FORM (s6, 2026-08-10): sandbox --disable all == 0,
 * build_insns 51/51, 21 rules dropped, cheat-asm stripped. THIS IS THE BODY
 * IN src/display.c AS OF THE s6 SESSION (candidate-ready).
 *
 * THE BREAKTHROUGH (forensics session): the entire 5-role residual knot of the
 * 12-form chassis dissolved once the tail was restructured to PER-ARM RETURNS
 * with per-arm block-scoped hi/lo locals and split-init statement style.
 * Root-cause chain, ground-truthed with the instrumented cc1
 * (ALLOCDBG/QTYDBG/greg/lreg dumps, tmp/grind/func_8007C7A0/s6/):
 *
 * 1. In every shared-tail spelling, the 0xE3000000 constant-holder pseudo is a
 *    single-block qty in the join block; local-alloc's ascending scan hands it
 *    $v0 (nothing hard is live there), and global.c's conflict walk then
 *    records hard_conf[lo] ∋ $v0 — the exact hard conflict s3's THEOREM T1
 *    measured (hard_conf[79] ∋ 2). lo can never take $v0 on ANY shared-tail
 *    graph. T1 was correct but its premise is a property of the SHARED-TAIL
 *    chassis class, not of the function.
 * 2. The 12-form additionally fused sxt(y) and the hi-mask into one pseudo
 *    (hi staging, L2), whose target registers differ ($a2 vs $v1) — GCC 2.7.2
 *    never splits live ranges, so the 12-form could NEVER byte-match. Its
 *    floor 12 was a masked-metric local optimum on a dead-end chassis.
 * 3. Per-arm returns make lo/hi/const per-arm block-locals: lo dies in the
 *    return-value insn (dest = hard $2) so the sugg pass pins lo→$v0; hi takes
 *    $v1; const then finds 2,3 busy and lands $a0 — the target trio. jump2
 *    cross-jumps the two identical lui/or/or tails into one (51 insns), and
 *    reorg fills the dispatch delay slot with the narrow arm's first insn
 *    (andi $v1,$a1,0xFFF) — exactly target's stream.
 * 4. With the arm expressions per-arm, the carrier/xlim/sxt knot ALSO
 *    dissolves: the arm-locals occupy $v0/$v1/$a0 inside both arms, so the
 *    carrier (live across the arms) conflicts with hard 2,3,4 and ascending
 *    scan lands it $a3; xlim-save→$a2, sxt(y)→$a2, ylim-save→$a0, sxt(x)→$a0
 *    all fall out naturally (ground truth: s6/c7a0_p3.model.json, sim 10/10).
 * 5. Dispatch sense: `if ((u32)(D_8009BE74-1) < 2U) { narrow } else { wide }`
 *    emits sltiu + bnez→narrow-label with the wide arm inline — target's
 *    layout. (The >=2U/wide-first spelling emits beq with arms swapped: 7.)
 *
 * No FAKE annotations: every statement maps to emitted instructions; no dead
 * stores, no UB, no reuse levers needed (the s4 L1 tx-dispatch-reuse and L2
 * hi-staging levers are GONE — they were metric-trap artifacts of the
 * shared-tail chassis).
 *
 * Twin: func_8007C86C — same body with 0xE4000000; expect the same 0.
 */

/* PsyQ 4.0 LIBGPU SYS: get_cs (static) — verbatim-linked Sony object
 * (census 2026-07-09); C ref: ground-up reconstruction (no published
 * reference matches this library build). */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 x = arg0;
    s16 tx;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            tx = D_8009BE78 - 1;
        } else {
            tx = x;
        }
    } else {
        tx = 0;
    }
    x = tx;

    if (arg1 >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }

    if ((u32)(D_8009BE74 - 1) < 2U) {
        s32 hi = arg1 & 0xFFF;
        s32 lo;
        hi = hi << 12;
        lo = x & 0xFFF;
        lo = lo | 0xE3000000;
        return hi | lo;
    } else {
        s32 hi = arg1 & 0x3FF;
        s32 lo;
        hi = hi << 10;
        lo = x & 0x3FF;
        lo = lo | 0xE3000000;
        return hi | lo;
    }
}
