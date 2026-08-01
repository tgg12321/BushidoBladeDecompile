/* saEft01Init — best measured form as of grind session 9.   floor: 7 / 91
 * (`sandbox saEft01Init --disable all`; target is 91 instructions, so the
 * instruction COUNT is exact and the whole residual is inside the
 * debug_printf argument block, build idx 46-61).
 *
 * ===========================================================================
 * WHAT CHANGED IN SESSION 9 — THE CHASSIS IS NOW THE ZERO-CONSTANT-LEVER ONE
 * ===========================================================================
 * Sessions 2-5 reached 8 / 91 on a chassis that needed TWO FAKE-family levers
 * (the reused `k` holding both compare constants to defeat loop.c's LICM, plus
 * `cnt = k;` to stage the mask out of an already-live pseudo).  Session 7
 * proved that chassis is a wrong basin: this function is Sony PsyQ LIBCD
 * `CD_datasync`, whose reference source has NO `k` and NO staging.
 *
 * This file has NEITHER lever.  Both compare constants are plain literals
 * exactly as the Sony reference writes them, there is no staging of any kind,
 * and the ONLY match device is a single `do { } while (0)` wrapper around the
 * timeout/printf block.  It scores 7 where the old two-lever form scored 8.
 *
 * WHY THE WRAPPER IS THE WHOLE LEVER (session 9, H37/H38 — read this before
 * touching it, because the reason is a proved arithmetic dead end on both
 * sides of it):
 *
 *   * global.c:allocno_compare sorts on
 *         pri = floor_log2(nrefs) * nrefs / livelen * 10000
 *     (verified to the integer on 14 allocnos across three chassis), and
 *     find_reg then hands out $s0,$s1,$s2,$s3 in that sorted order.
 *   * Target's map is D_800A125C->$s0, D_800A1494->$s1, param->$s2,
 *     D_800A11DC->$s3.  A plain goto-loop body gives the param 2 refs over a
 *     52-insn live range => pri 384, which outranks D_800A1494's 306, so the
 *     param takes $s0 and the whole thing rotates (the inherited 18 / 91).
 *   * The param can NEVER be demoted by lengthening its live range: its live
 *     length is bounded by the function's 59 insns and it can never receive
 *     the REG_EQUIV note that DOUBLES the three pointers' live lengths
 *     (local-alloc.c:1019-1052 attaches REG_EQUIV only to a single-set pseudo
 *     whose source is CONSTANT_P or an unchanging MEM; the param's set is
 *     `(set (reg 72) (reg:SI 4 a0))`).  2*10000/59 = 338 > 306 for every
 *     spelling that exists.  So the pointers must be lifted, not the param.
 *   * A REAL loop lifts them — flow.c adds `loop_depth`, not 1, per reference,
 *     so the two doubly-used pointers go 3 refs -> 5 and outrank the param.
 *     But a real loop also hands loop.c the two compare constants as
 *     movables, and they are hoisted unconditionally here
 *     (loop.c:1631 moves when `threshold * savings * lifetime >= insn_count`;
 *     threshold = 1 + n_non_fixed_regs = 61, savings = lifetime = 1, and the
 *     loop is 50 real insns), costing two extra callee-saves and +5 insns.
 *     Every reference-corpus spelling pays this: sotn 35/91, xeno 27/96,
 *     tomba 27/96, `clean` 32/96.
 *   * `do { ... } while (0)` emits NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END
 *     exactly like a real loop, so flow.c's loop_depth rises inside it — but
 *     the function's real back edge is still a bare `goto`, so loop.c never
 *     sees a loop and never builds a movable for either constant.  It buys
 *     the ref weighting WITHOUT the hoist.  Measured: 18 -> 8 with nothing
 *     else changed, and the resulting .lreg numbers are exactly the predicted
 *     ones (D_800A125C 5/96 = 1041, D_800A1494 5/98 = 1020, param 2/52 = 384,
 *     D_800A11DC 3/100 = 300).
 *
 * The wrapper is therefore a `/ * FAKE * /`-annotated match device under the
 * owner's 2026-07-06 do-while(0) ruling (sanctioned for ANY codegen effect,
 * mandatory inline annotation, single level so no nesting justification is
 * needed).  It is NOT a semantic lie: the block executes exactly once either
 * way and the wrapper changes no program fact.
 *
 * WHAT IS LEFT (7 diffs, all in build idx 46-61, the argument block):
 *   target: lbu a0,0(s1) / lbu v0,1(s1) / lui+lw a1 / sll v0,2 / addu v0,s0 /
 *           sll a0,2 / lw v1,0(v0) / lui+lbu D_800A11D5 / addu a0,s0 /
 *           sll v0,2 / addu v0,s3 / sw v1,16(sp) / lw a2,0(v0) / lw a3,0(a0)
 *   ours:   the same two lbu IN TARGET'S ORDER, but the idx[0] address chain
 *           runs through $v0 and issues `lw a3,0(v0)` EARLY (build idx 54),
 *           where target keeps that chain alive in $a0 and issues
 *           `lw a3,0(a0)` as the very LAST memory reference of the block.
 * Session 9 swept twelve argument spellings against this chassis (t1-t6,
 * u1-u6): both-inline 13, arg5-named-only 13, index-bytes-named 13,
 * named-pointer-with-inline-deref 9-10, arg2-also-named 14 (and 90 insns),
 * arg4-named-first 7, both-named-arg4-first 7, index-staged 7.  7 is the
 * floor of that whole family.
 *
 * ===========================================================================
 * SESSION 10 (synthesis) — THE BODY IS UNCHANGED; THE MODEL OF THE 7 IS NOT
 * ===========================================================================
 * Re-applied and re-measured this session at exactly 7 / 91.  Nine more
 * argument spellings measured (nineteen in total) and 7 is still the floor,
 * but the instrumented cc1's real INSN_PRIORITY tables now say what the 7 IS:
 *
 *   * GCC 2.7.2's scheduler runs BACKWARD — `priority()` is computed over an
 *     insn's LOG_LINKS (predecessors) and `schedule_block` emits each chosen
 *     insn at the block's current TAIL.  So the printed priority is dependence
 *     DEPTH from the top of the block and HIGHER priority = emitted LATER.
 *   * The whole argument block is a three-level lattice with SEVEN insns tied
 *     at level 2, so `rank_for_schedule`'s tie-breaks decide the order (load /
 *     store dependence class against last_scheduled_insn, then INSN_LUID) —
 *     and INSN_LUID at sched2 is SCHED1's output order, not source order,
 *     which is why session 6 measured statement placement here as byte-inert.
 *   * The 7 DECOMPOSES into two independent sub-goals, each already solved by
 *     a DIFFERENT measured spelling:
 *       (A) `lw a3` as the last memory reference (target build idx 61) — the
 *           both-lookups-inline form lands it at exactly 61, but drags its
 *           whole idx[0] chain to the tail with it (13 / 91);
 *       (B) both `lbu` at the block head in target's order (idx 46/47) — THIS
 *           form has it, and issues `lw a3` six insns early at 54 (7 / 91).
 *     Target's idx[0] chain is SPREAD (lbu 46, sll 52, addu 56, lw a3 61);
 *     every spelling reachable from C emits it as one contiguous run.
 *   * F19's mechanism (a local-alloc copy preference for `$a0`) is KILLED: the
 *     register difference is downstream of the schedule — our chain is live
 *     51-54 and does not overlap arg5's value in `$v1`, target's is live 46-61
 *     and does, so the conflict graphs differ because the ORDER differs.
 *
 * ===========================================================================
 * SESSION 11 (structural) — BODY STILL UNCHANGED; THREE MORE AXES CLOSED
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.  Thirty-one further forms on
 * three axes nobody had touched; all three are dead and the model of the 7 is
 * now positional rather than tie-break-theoretical:
 *
 *   * Target's emitted order in the argument block is EXACTLY ascending in
 *     scheduler priority (1,1,1,2,2,2,2,2,2,3,3,3,4,4 over build idx 46-61),
 *     which is what a backward list scheduler produces when it always pops the
 *     highest-priority ready insn.  So target's `lw a3` is a level-FOUR insn,
 *     a whole level ABOVE `sw 16(sp)` — not a level-2 insn winning a tie, as
 *     session 10's F22 assumed.  The only edge that does that is a store->load
 *     memory dependence on the 16(sp) outgoing-arg slot: in target's RTL stream
 *     the `sw` precedes both register-arg loads while both `lbu`s and the arg5
 *     value chain precede the `sw`.
 *   * `calls.c:1615-1665` is the mechanism.  The register-arg precompute loop
 *     runs FORWARD; for an INLINE array-element argument `expand_expr` emits
 *     the address chain there and returns a MEM, so the load is deferred to
 *     `load_register_parameters` (after the stack stores) — target's split.
 *     For a NAMED-local argument the value is already a REG and the load
 *     happened at the statement — ours.  And the loop-nesting half of
 *     `preserve_subexpressions_p` (stmt.c:2435) is dead code here, because
 *     `toplev.c:3387` sets `flag_expensive_optimizations` for every -O2 build:
 *     the do{}while(0) wrapper has NO influence on argument expansion.
 *   * The block has exactly THREE reachable states and they are RIGID:
 *     arg4-as-named-VALUE = 7 (this file), arg4-as-named-ADDRESS = 9 (i2),
 *     arg4-fully-inline = 13 (i1).  Six more bolt-ons (q1-q6) were byte-
 *     identical to whichever attractor they started from.
 *   * DEAD AXES measured this session: do{}while(0) wrapper EXTENT (7 forms —
 *     byte-inert in both directions provided the wrapper contains BOTH the
 *     tslTm2LoadImage_2 call and the arg4 store; dropping the call out costs 1,
 *     lifting the store out costs 9 and an instruction); the arg3
 *     `tbl_11dc[D_800A11D5]` spelling (8 forms, 10-15, all regressions —
 *     fully-inline arg3 is the family optimum); the pointers' DECLARATION order
 *     (byte-inert).  Their INITIALISATION order is live and the order written
 *     below is the unique optimum (the five permutations score 8/11/11/12/12) —
 *     do not "tidy" it.
 *
 * ===========================================================================
 * SESSION 12 (structural) — BODY STILL UNCHANGED; F25 ANSWERED, 4 AXES DEAD
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.  Fifty-two further forms on
 * five axes nobody had touched:
 *
 *   * F25 is ANSWERED by reading calls.c: the register-arg precompute loop
 *     (1618-1665) runs BEFORE store_one_arg for the single stack argument
 *     (1736-1739), which runs before load_register_parameters (~1876).  So an
 *     inline arg4 genuinely does get its address chain emitted before the
 *     `sw 16(sp)` and its load after — target's split is reachable at expand,
 *     and the fully-inline attractor's contiguous 57-61 idx[0] chain is a
 *     SCHEDULING outcome, not an expand-order one.
 *   * The one live lever found is RTX_UNCHANGING_P on arg4's load: `const s32
 *     *tbl_125c`, or equivalently a per-access `((const s32 *)tbl_125c)[...]`
 *     cast, is a FOURTH attractor at 9 / 91 and the FIRST measured form whose
 *     `sw 16(sp)` precedes `lw a3` (target's relation).  It costs 2 — the two
 *     `lbu` swap and the D_800A11D5/arg3 chain moves after `lw a3` — and all
 *     thirteen bolt-ons on that chassis stay at >= 9.  The flag is inert on a
 *     fully inline arg4, i.e. it only moves a STATEMENT-emitted load.
 *   * DEAD AXES: arg1 / the format-string address (3 forms, all 7 — every one
 *     of the 51 prior forms touched only args 2-5); MEM_IN_STRUCT_P, probed by
 *     re-typing the hoisted bases as pointers-to-array so the accesses are real
 *     ARRAY_REFs (6 forms, all 7); pre-loop global-store placement among the
 *     pointer inits (4 forms, 7/7/17/7); and debug_printf's PROTOTYPE — varargs
 *     `(void *, ...)` and K&R `()` are byte-identical to the fixed 5-arg form
 *     against three different bodies.
 *
 * ===========================================================================
 * SESSION 13 (permuter) — BODY STILL UNCHANGED; THE PERMUTER MODALITY IS DEAD
 * ===========================================================================
 * Re-applied and re-measured at exactly 7 / 91.  Two fresh-seed campaigns
 * (candidate chassis, base 435, 16.5k iters; const chassis, base 455, 13.2k
 * iters) produced 9 finds; every one was re-scored in the sandbox and the only
 * permuter-score improvements are sandbox REGRESSIONS (428 -> 18/92,
 * 423 -> 17/92, 450 -> 10/91), while one 455-scored find is sandbox 10 where
 * the 455-scored base is 9.
 *
 * The reason is structural, not a budget question.  decomp-permuter charges
 * PENALTY_REORDERING = 60 and PENALTY_REGALLOC = 5 over a RE-ALIGNED
 * instruction stream (scorer.py:14-18); the engine sandbox is position-locked
 * and charges 1 for either.  This file's 435 decomposes EXACTLY as seven
 * reorderings (420) plus three register differences (15) — i.e. the permuter
 * is charging 60x for the very seven instructions the sandbox charges 7 for.
 * Our whole residual is ordering, so any mutation that trades ordering for
 * renaming reads as progress to the permuter and is a regression to us.  That
 * is all both campaigns found.  Random permutation cannot descend on this
 * basin; do not spend another session's modality on it without first fixing
 * the objective (the penalties are class attributes with no knob, and tools/
 * is outside a grind session's writable surface — an operator decision).
 *
 * One substantive new axis was measured and closed: arg5 spelled through the
 * GLOBAL, `D_800A125C[idx_1494[1]]`, instead of the hoisted base — the first
 * of 65 argument forms to do so.  17 / 92: the second symbol reference costs a
 * fresh lui/addu/lw chain and collapses tbl_125c's allocno priority, rotating
 * the whole callee-save map.  Banked as
 * rejected/permuter-global-arg5-rotates-callee-save-map-17-92.c.
 *
 * NEXT: hypotheses.md F26 — diff the instrumented cc1's RANKDBG trace for THIS
 * form against the const (c1) form.  They differ in exactly one bit
 * (RTX_UNCHANGING_P on one load) yet flip both the `lbu` order and the arg3
 * chain's position, so the trace difference isolates the single tie-break the
 * whole residual now rests on.  Do NOT re-sweep argument spellings; sixty-three
 * forms are banked over four rigid attractors.
 * ===========================================================================
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    /* FAKE: do{}while(0) — loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of. */
    do {
        s32 arg4;
        tslTm2LoadImage_2(&D_800161B8);
        arg4 = tbl_125c[idx_1494[0]];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                     tbl_125c[idx_1494[1]]);
        cdrom_ClearIrq();
    } while (0);
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
