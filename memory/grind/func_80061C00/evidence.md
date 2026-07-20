# Evidence bank — func_80061C00

## s1 (recon, 2026-07-20)

- **Baseline:** canonical verdict C (93 insns), honest floor `sandbox --disable all` = **2**. One regfix rule (`reorder 6,3,4,5 @ 3-6`) papers over the whole gap.
- **The entire distance-2 diff is ONE displaced instruction:** `move v1,a1` (the arg1 param copy). Target compiler-body order: `move s2,a0; move v1,a1; move s1,a2; lui; addiu; sw gp; li; beq` (pure param/RTL order). Our build sinks the copy to between `addiu` and `sw gp` (2 alignment edits = distance 2).
- **Sibling** func_80061D74 (COMPLETED-C, similarity 0.83) avoids the copy entirely — it stores `sh a1,0x1A(sp)` early (no arg2 clamp branch, single block). Not transplantable: our function's clamp branch pushes all array stores into block 2, forcing the a1→v1 copy to live across block 0.
- **Root mechanism (from cc1 -dS dump + sched.c source):** GCC 2.7.2 sched.c is a BACKWARD list scheduler. `adjust_priority` (sched.c:2534) boosts "birthing" insns — `birthing_insn_p` = SET of a REG that is live in `bb_live_regs` with `reg_n_sets[] == 1` — to `LAUNCH_PRIORITY` (0x7f000001, sched.c:187). Boosted insns are picked early in backward order = **placed at the block bottom**. Our arg1 copy `(set (reg:HI 73) (reg:HI a1))` is single-set + live-into-block-2 → boosted → sinks. The arg2 copy (pseudo 75) escapes because `arg2 = 0;` in the clamp arm makes it double-set. Dump extract: `tmp/grind/func_80061C00/s1/sched_block0_extract.txt` (jump 23's LOG_LINKS `(anti 8, anti 19, 10, 22)`; T-2 ready shows insns 8/22 at 0x7f000001).
- **Confirmed positively:** BOTH multi-set probes (arg1 reused as pointer holder; arg1 reused as sp20[1] repack temp) moved the copy to EXACT target slot 3. The boost is the whole story for the displacement.
- **But value-reuse cascades:** with arg1 multi-set, (a) the merged pseudo allocates $a3 not $v1; (b) whichever temp arg1 absorbs loses ITS single-set boost, shifting the post-call schedule (pointer `lui v1` hoists into arg1's live range, $v1/$a0/$a1 rotate). Scores: 28 / 21 / 12 vs baseline 2.
- **cse/flow delete no-op self-assigns before sched1** (probe: `arg1 = arg1;` in clamp arm → byte-identical output). A dead store to the param CANNOT create the needed second set — the dead-store-FAKE route is mechanically closed for this diff.
- **K&R-style definition:** byte-identical output. Param-copy RTL unchanged. Dead axis.
- Post-sched1 order is additionally polished by sched2 (moved the copy up one slot, after reload where the boost is disabled) — final build order `lui,addiu,move v1,sw` vs sched1 RTL order `lui/addiu,sw,move`. Any future lever analysis must reason about BOTH passes.
- Caller: single C caller `src/code6cac_b.c:3170` uses an implicit declaration with only 2 args — the definition's param types/spelling are free to vary without caller codegen impact.

- [s1] Honest floor 2 (93/93 insns), verdict C; the single regfix rule 'reorder 6,3,4,5 @ 3-6' covers exactly this displacement

- [s1] Target prologue body is pure RTL/param order (move s2,a0; move v1,a1; move s1,a2; lui; addiu; sw gp; li; beq); build sinks only the a1 copy

- [s1] sched dump: jump 23 LOG_LINKS (anti 8, anti 19, 10, 22); insns 8/22/17 boosted to 0x7f000001; arg2 pseudo 75 double-set (clamp) hence unboosted — explains why only arg1's copy moves

- [s1] Defeating the boost (any multi-set spelling) reproducibly places the copy at target slot 3 — the displacement mechanism is fully understood

- [s1] cse/flow delete no-op self-assigns before sched1: reg_n_sets cannot be inflated by dead stores

- [s1] Sibling func_80061D74 shape (early sh a1) not transplantable: the arg2 clamp branch forces the copy

- [s1] Single C caller (code6cac_b.c:3170) is implicit-decl 2-arg: definition spelling free to vary

- [s1] sched2 (post-reload, boost disabled) additionally lifts the copy one slot; final order shaped by both passes

## s2 (structural, 2026-07-20)

- **NEW distance-2 attractor found (candidate.c updated):** arg1 retyped s32 + double-set via the arm-1 `D_800A3468` reload, PLUS `val` carrying sp20[2]-repack + arm-1 `0x10016`. Block 0 fully matches target (`move v1,a1` at slot 3, $v1); ALL registers match; both arms match. Sole residual: post-call `lw v1,0x24`/`lw a0,0x28` emitted swapped.
- **flow.c:1284**: reg_n_sets is REBUILT from scratch during flow (post-DCE) — only flow-surviving sets count at sched1. Mechanism-level closure of every dead/derivable-second-set idea.
- **sched2 has no independent lever (frontier #3 KILLED):** rank_for_schedule (sched.c:2455) ties resolve by descending INSN_LUID = post-sched1 stream order; sched2 output is fully determined by sched1's order + memory-unit hazard bumps. Simulated: given target's sched1 order, sched2 reproduces target block 0 exactly.
- **Block-0 order is purely the LAUNCH-boost asymmetry:** without insn 8's boost, backward tie-break (luid desc) reproduces expand order = target. Confirmed empirically: any surviving second set of arg1 fixes block 0 completely.
- **Probe-1 (arm1-reload reuse alone) = score 7:** schedule PERFECT, but local_alloc (runs before global) hands the vacated arm-1 $v1 to the 0x10016 const temp; global arg1 -> $a3. Register-rename cluster only.
- **ALLOCDBG (instrumented cc1 at tmp/gccdbg/cc1, BB2_ALLOC_DEBUG=1):** allocno priority = floor_log2(refs)*refs/live_length*10000*size (global.c allocno_compare). Key rows (probe 1): 91 (D+2 ptr) refs 8 len 22 pri 10909 -> v1 first; arg1 refs 5 len 21 pri 4761 (dead gap correctly excluded from live_length).
- **Const rescue that WORKED (candidate):** globalize 0x10016 through the sp20[2] repack carrier (`val`) — the ONLY $a0-targeted load outside conflicts; conflict walk gives every pseudo its target register (verified: score 2, all regs match).
- **The lw-swap wall (candidate's residual):** val's load loses its birthing boost (n_sets=2); sched1's pick between ready {v1lw(LAUNCH), a0lw(prio 1)} always takes v1lw -> emitted v0,a0,v1 vs target v0,v1,a0. Source order/luid is irrelevant while the boost is asymmetric (variant A measured byte-identical). Target order = all-three-boosted luid-desc; val's double set is inherent to the const rescue. Coupled contradiction: block-0 fix requires arg1 2-set -> const leaks v1 -> const rescue requires val 2-set -> lw order breaks. Each link individually proven.
- **cse kill-laws measured:** (F, 31) naming the D+2 pointer into a C var -> cse const-folds every deref to absolute %lo addressing, +4 insns — the anonymous `(D_800F1164+2)[k]` spelling is load-bearing for the base-reg shape. (G, 7) staging cse-DERIVABLE values (D+3 = 91+1) -> use substituted to the derived temp, set deleted by flow, n_sets collapses. Only memory loads / frame addresses / underivable constants survive as second sets.
- **Variant H (second carrier u = (s32)sp10 + 0x10017) = 17:** u's priority (8000) beats val's; sched1 hoists u's addiu into the lw/sh overlap -> v0 blocked -> u steals $a0 -> full rotation. Two-carrier shapes over-perturb the coupled fixpoint.
- **Target $v1-write census (exhaustive):** slot-3 move (set 1), lw v1,0x24 (H5 killed s1), lui/addiu D+2 (H4 s1 + F s2 killed), arm-1 lw gp (s2 family: works for sched, forces the const problem). No unexplored second-set host remains for arg1.
- Analytic kill: an in-window (pre-call) second set of arg1 is impossible — after `sh 0x1A` the only remaining pre-call stores are $zero stores; interleaved $v0-constants can't share arg1's pseudo (one pseudo = one reg).

- [s2] flow.c:1284 rebuilds reg_n_sets from scratch post-DCE: only flow-surviving second sets count at sched1 (mechanism closure of all dead/derivable-set roads)

- [s2] sched.c:2455 rank_for_schedule: ties below priority resolve by descending INSN_LUID; sched2 output deterministic given post-sched1 stream (plus memory-unit hazard bumps only)

- [s2] global.c allocno_compare priority = floor_log2(n_refs)*n_refs/live_length*10000*size; live_length correctly excludes dead gaps (arg1: refs 5 len 21 pri 4761 despite spanning the function)

- [s2] local_alloc runs before global_alloc and always wins: absorbing a block-local temp into a global pseudo vacates its register to the remaining locals (probe-1 $v1 leak)

- [s2] cse survival law: staged second sets survive only for non-derivable values (memory loads, frame addresses, fresh constants); symbol arithmetic and live-temp-derivable values are substituted away and flow-deleted

- [s2] The anonymous (D_800F1164+2)[k] spelling is load-bearing: naming the pointer triggers cse const-fold to absolute addressing (+4 insns)

- [s2] Target $v1-write census exhausted: slot-3 move, lw v1,0x24 (H5/s1), lui/addiu D+2 (H4/s1 + F/s2), arm-1 lw gp (s2) - no unexplored second-set host for arg1 remains

- [s2] Two distance-2 attractors now banked: baseline (block-0 copy sink) and s2 candidate (post-call lw swap); both are birthing-boost asymmetry residuals

- [s2] Instrumented cc1 lives at tmp/gccdbg/cc1 (BB2_ALLOC_DEBUG/BB2_RANK_DEBUG); production tools/gcc-2.7.2/build/cc1 lacks the hooks

## s3 (structural, 2026-07-20)

- **Floor unchanged: 2** (s2 candidate re-verified in src at session end). Three structural probes measured, all KILLED; the const-carrier host space is now measured-exhausted.
- **P1 kill-law (score 7):** frame-address second sets survive cse only at STORE-OPERAND uses (variant H), never at CALL-ARG uses — cse folds the arg copy `a0 = val` into a direct `addiu a0,sp,K` (equally cheap), val's set goes dead, flow deletes it, const collapses to local (probe-1 leak). Kills (s32)sp18/(s32)sp10/(s32)sp30 call-arg hosts as a class.
- **Side-confirmation from P1 dump:** with the repack natural (all three loads single-set), emission is lw v0/v1/a0 = exact target order AND the whole check region matches byte-perfect, including the delay-slot `addiu a0,v1,1` — that addiu is cse's canonical D+3 pseudo (reg 95, born at the (D+2)[1] lbu address), NOT a C-level variable.
- **P4 generalizes H10 (score 7):** the check-region D+2/D+3 values have canonical anonymous pseudos (regs 92/95 in fn.cse); ANY C-level named carrier of them is substituted away (val's set became `val = reg95`, arm-2 use substituted to 95, set flow-deleted) regardless of source placement. Placement upstream of the first materialization = variant F (naming = const-fold, +4). No spelling of a D+2/D+3 carrier can survive.
- **P3 (score 17):** arg1 as sp20[1] repack carrier ON TOP of the s2 shape rotates RA (arg1→a3, val→v1, gp-reload→a0, sh reorder) — H11 family; extra refs shift the conflict walk.
- **sched.c n_deaths lever is DEAD (source-read):** adjust_priority's `n_deaths` switch (defer insns with REG_DEAD notes) never fires — schedule_block's pre-pass unlinks ALL REG_DEAD notes onto dead_notes before scheduling (sched.c:3596-3618); the `???` comment at 2544 is accurate. Both compiles identical here; not an asymmetry source.
- **combine's reg_n_sets decrement loophole (source-read, combine.c:2307-2314):** when combine eliminates an i2 set it decrements reg_n_sets UNLESS `added_sets_2 || newi2pat || i2dest_in_i2src`. A self-referencing second set (`arg1 = arg1 OP k`) merged away by combine would leave reg_n_sets==2 at sched1 with no emitted insn — but no such op can merge here: arg1's only consumer is the `sh` whose operand must be a plain REG, so the op insn always survives (+1 insn). Analytically closed for THIS function; possibly useful elsewhere.
- **Swap-unfixability argument (analytic, from s1 trace mechanics + target pick order):** to byte-match, our backward pick order must equal target's exactly; in that order, when lw-a0(0x28) is picked, sh 0x12 and sh 0x10 are already scheduled, so lw-v1/lw-v0 are READY — an unboosted a0lw (priority 1) can then never be picked over the boosted (LAUNCH) v1lw. Target order therefore REQUIRES a0lw boosted = its dest pseudo single-set = the const cannot ride the sp20[2] repack temp. Combined with the host exhaustion above, every C shape in the "const-carrier" family is dead; the matching form must break some assumption of this model (e.g. a post-call region respelling that changes which pseudo needs $a0 at all).
- Artifacts: tmp/grind/func_80061C00/s3/{dump.sh, fn.rtl, fn.cse, fn.flow, text1b.i.*}.

- [s3] Kill-law: frame-address second sets survive cse only at store-operand uses (variant H); at call-arg uses cse folds the arg copy into a direct addiu and flow deletes the carrier set (P1, score 7)

- [s3] Generalized H10: the check-region D+2/D+3 values have canonical anonymous pseudos (regs 92/95); any C-level named carrier is substituted away at any source placement (P4 fn.cse, score 7)

- [s3] With the repack natural, emission is lw v0/v1/a0 = exact target order and the check region incl. the delay-slot addiu a0,v1,1 is byte-perfect (P1 objdump) - the residual is purely the const-globalization coupling

- [s3] arg1 as sp20[1] carrier on the s2 shape rotates RA: arg1->a3, val->v1, gp-reload->a0, sh reorder (P3, score 17)

- [s3] sched.c:3596 pre-pass strips all REG_DEAD notes before scheduling; adjust_priority's n_deaths switch never fires (source-read)

- [s3] combine.c:2307 decrements reg_n_sets on i2 elimination unless added_sets_2/newi2pat/i2dest_in_i2src - the loophole needs a mergeable consumer, which arg1 lacks (sh operand must be plain REG)

- [s3] Floor unchanged at 2; s2 candidate re-verified in src at session end (sandbox score 2, 93/93)
