# Hypothesis ledger — func_80069AE4

## s1 (recon, 2026-07-20)

- H1 KILLED: "RTL-order/tie-break lever — naming the address pointer first in the loop
  body gives the a0-set an early LUID." Probe: pure form (sandbox) + register-asm $4
  diagnostic (scratch cc1). Result: byte-identical slot-3 output both times; combine
  merges the address set into the call-site arg load whenever the pointer dies there.
- H2 KILLED: "Second real use via routing the first store through the pointer keeps it
  alive past the arg load, defeating the combine merge." Probe: sandbox. Result: score 5;
  cse folds the address use to sp-form (`sw v1,24(sp)`), pointer still dies at arg load,
  RA perturbed. Kills the whole store-routing family (all store uses fold to sp-form).
- H3 (frontier, OPEN): "Loop body was multi-block at sched1/sched2 time; the block
  boundary put the a0-set early (cross-block = no combine merge, no LUID contest with the
  lw), and jump2 — which runs AFTER sched2, before reorg (toplev.c) — deleted the extra
  control flow (no-op-move / branch-to-next / unreferenced-label cleanup)." Next probe:
  enumerate natural spellings whose in-body control flow is real through sched2 but
  jump2-deletable; validate each in sandbox.
- H4 (frontier, OPEN): "The original family-wide idiom is recoverable from siblings."
  func_8005C8A8/D814/E098 show the same addiu-at-block-top signature in STRAIGHT-LINE
  code (simpler than the loop case); matched text1b.c callers of func_8007352C
  (~12986/13086/13172-95/17248/17411) have un-audited emitted-vs-target addiu positions.
  Next probe: objdump build/text1b.o for each matched caller, find one whose TARGET has
  addiu-at-distance, read its C idiom.
- H5 (frontier, OPEN): directed permuter (PERM_* statement-order/spelling macros) seeded
  from the floor-2 HEAD form over the loop-body region — the window is only 2 insns;
  fresh-seed stopping discipline per feedback/permuter-fresh-seed-discipline.

## [s1] Naming the address pointer first in the loop body gives the a0-set an early LUID, flipping the priority-1 tie so `addiu a0,sp,24` lands at the loop top
- mechanism: rank_for_schedule (sched.c) breaks equal-priority ties by INSN_LUID, preserving RTL order; an early source position should mean early LUID
- probe: Pure form: `s32 *dst = &s.sp18;` first in body, member stores kept, arg via (s32)dst (sandbox). Diagnostic: same with register asm("$4") pin, scratch-only cc1 compile
- result: Byte-identical slot-3 output in BOTH probes (sandbox score 2 unchanged). combine merges P=sp+24 into the call-site arg load whenever P dies at the arg use, so the addiu always inherits the call-site LUID regardless of source position
- verdict: KILLED

## [s1] Giving the pointer a second real use (route only the FIRST store through it) prevents it dying at the arg load, defeating the combine merge
- mechanism: combine only merges def into use when the def's register dies at the use insn
- probe: `*dst = v;` for store #1, other stores via members, arg via (s32)dst (sandbox)
- result: Score 5 (floor 2). cse folds the address use 0(dst)->24(sp) (`sw v1,24(sp)` emitted), so dst still dies at the arg load and combine still merges; the longer-lived temp flips lw dest to $v1 and moves the sw next to the jal. Kills the entire store-routing family — every store use folds to sp-form
- verdict: KILLED

## [s1] Both sched1 and sched2 decide the addiu placement identically via a three-way priority-1 LUID tie; target order mathematically requires LUID(addiu a0) < LUID(lw) in post-combine RTL
- mechanism: GCC 2.7.2 reverse list scheduling; rank_for_schedule: priority -> dep-class vs last-scheduled -> INSN_LUID; insns 281(lw)/284(addiu s0)/300(addiu a0) all priority 1 in both -dS and -dR traces
- probe: cc1 -dS -dR instrumented dumps of the exact build flags (tmp/grind/func_80069AE4/s1/text1b.i.sched{,2}); read sched.c rank_for_schedule + loop-note fence code
- result: Confirmed in both traces (T-11 tie picks insn 300 both passes). Also confirmed only LOOP_BEG/LOOP_END notes fence sched (sched.c:2279), and toplev.c shows NO jump pass between reload and sched2, while jump2 (cross-jump/no-op-move/label cleanup) runs AFTER sched2 before reorg
- verdict: CONFIRMED
