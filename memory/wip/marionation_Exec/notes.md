# marionation_Exec — CANDIDATE MASKED 4 (mh5); region-3: near-total impossibility map ⇒ hunt the model-hole (2026-07-04 s6…s6d)

## TL;DR
Floor unchanged (masked 4 = region-1 pair 2 + region-3 2; candidate.c = mh5).
S6d closes the mechanism space: THE WINDOW THEOREM (below) says the check2
NOP has NO byte-free pure-C construction under the now-nearly-complete model
of reorg — every corridor is source-proven or mini-measured dead. Since the
ORIGINAL COMPILER produced the nop from real C, the model MUST have a hole;
the next session's job is trace-side hole-hunting (per-function DBRDBG
segmentation; the check2 walk trace VALIDATED the model where tested, but
block=0 fallbacks and a mid-block-target fill (insn 352, uid-space ambiguous
— possibly another function) remain unexplored), plus re-seeding permuters
on the r5d spelling. r5d (s6b) remains the confirmed refusal SHAPE: an
arm-head label with a live referencer refuses the steal in both passes and
yields the exact target byte shape; only the referencer costs bytes.

## THE WINDOW THEOREM (s6d — the impossibility map; every clause sourced)
Between check2's beqz and the move, target bytes = [sb] only; slot = NOP.
For the slot to stay empty through dbr's TWO {fill_simple×2, fill_eager,
relax} passes, one of these must hold at BOTH fill_eager attempts, and each
is closed:
1. a1 (reg 5) live at after_blocks ⇒ needs a $5-allocated pseudo live there
   ⇒ crosses the loop's immediate vsync call ⇒ s-reg, never $5. CLOSED (5b +
   allocation argument).
2. everything-live ⇒ find_basic_block==-1 ⇒ post-flow-minted target label
   (flow.c: every flow-era label is its own basic_block_head). All minters:
   do_cross_jump (mid-block ⇒ b_far found, not -1), USE-hoist (wrong side),
   relax 3992/4002 chain (FILLED-SEQ-only per 3946 guard ⇒ mutually
   exclusive with refusal; and a filled slot never empties — see 4). CLOSED.
3. Walk-blocker in [beqz..move]: labels un-own (3397) / jumps stop the walk,
   but: referenced labels need a referencer-insn alive to FINAL (bytes);
   plain &&-take dies (unused: never expands — k-battery; dead-store into
   live var: expands but flow kills the set and jump2 sweeps the label —
   u-battery + trace of label 56); forced_labels registration is
   EXPAND_INITIALIZER-ONLY (expr.c 4137 ⇒ static-data bytes);
   LABEL_PRESERVE_P set only for nonlocal/eh labels (stmt.c 665/3085).
   Referencer-jumps: near-label folds pre-flow at ANY condition (round-6);
   far referencers survive but = bytes; every relax deleter's precondition
   either held in pass-1 (dies too early ⇒ W1 pass-2 retry steals — retries
   measured real) or needs a gap-insn X that is itself bytes: X=sb
   unstealable (memory/trap), X=la ineligible (length 2), X=move guard-live;
   4031's dissolution RE-EMITS slot insns inline (bytes back); USE-markers
   are walk-skipped (3404) and next_active-invisible post-reload (emit-rtl
   1855: USE/CLOBBER skipped only when reload_completed). CLOSED.
4. Filled-then-emptied: 3956 slot-redundancy scan stops at CODE_LABELs
   (redundant_insn 2001) ⇒ window = {lbu v0,-1(s3); andi a2,v0,0xFF} only ⇒
   the move never matches; a matching lbu can never be target-stolen into
   the slot (loads fail may_trap at 3461; backward `simp` strategy has no
   trap check but only reaches pre-branch insns — none match); recompute
   dests are all dead-at-after_blocks ⇒ flow deletes pre-reorg; volatile
   loads are unstealable. try_merge_delay_insns (1815-1964) deletes THREAD
   copies or the OTHER SEQ's slots, never the caller's own slot (annul paths
   dead on MIPS). CLOSED.
⇒ Either one of ~30 derivation links is wrong (compound ~30-50%!), or the
original source shaped an upstream difference not yet modeled. The original
EXISTS ⇒ the hole exists. FIND IT EMPIRICALLY.

## Validated ground truth (s6d trace, tmp/mar_check2_trace.py)
check2 = insn 386 in the candidate compile; its fall-walk EXACTLY matches
the model: trial 390 (sb) LOSE setsopp=1 trap=1; trial 393 (la) LOSE quiet
(eligibility, length 2 — la/lui never stolen, arm-1's bare slot confirms);
trial 399 (move) WINNER. The arm-2 guard (402) tried the move backward
(`simp`) and was setneed-refused (condition reg). Arm-1's guard (327) took
its move (324) backward — matches bytes. UNEXPLORED: `mtlr target=399
block=0` + `thr insn=352 thread=356 opp=399 WINNER` — a fill whose OPPOSITE
thread is mid-block at a move with block-0 fallback; uid spaces of system.c
functions overlap in the log (no function separators) — possibly the twin.
Natural block=0/-1 mtlr calls DO occur in this TU. THE HOLE, IF ANY, IS IN
SUCH REGIONS.

## NEXT SESSION (in order)
1. Segment the DBRDBG log per function (add a function-name banner print to
   the debug cc1, or split by uid discontinuities); enumerate EVERY mtlr
   block=-1 / block=0 call inside marionation proper; for each, identify the
   construction that produced it and test transplanting it to check2.
2. Diff every marionation fill decision (simp/thr lines) against the model;
   any surprise = the hole.
3. Re-seed permuters on the r5d spelling (uninverted `if (check) goto arm;
   goto after_blocks; arm:` + label variants) — tmp/perm_mar6 base swap.
4. If the theorem survives 1-3, surface to the owner: region-3's mechanism
   evidence (this file) + the two honest options — keep hunting vs a
   documented-plateau park (NOT canonical-asm; the original is compiled C).

## Arm-2 transposition (2 masked lines): unchanged from s5 — git history
(d1/d2 tie at 952; ref-bump program CLOSED: exact s-alloc, no byte-clean form).
## Region-2 SOLVED — recipe unchanged (in candidate.c)

## Region-1 pair — permuter verdicts (s6)
- perm_mar6 200-1/2 (arg5 value-staging): masked 8, registers break. REJECTED.
- 200-3 (goto-loop into the arm): masked 5 (3-insn rotation). REJECTED.
- csmd4 40-2: masked 6 vs floor 2. REJECTED. mh5's basin keeps re-confirming;
  frontier stays the g3 v1/a0 qty exchange (untried: multi-set arg5
  VALUE-staging keeping the lw dest split).

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5, i1495 s6,
  arg0 s7; check a2, src a0, i v1, b v0, dst/dst2 a1.
- arm-2: sb -1(s3); move a1,s4; la F19A8; beqz a1→.L812BC slot=li v1,7;
  li a3,-1; loop; .L812BC: j .L812CC; move v0,a2. check2 beqz slot = NOP.
- .L812C4: beqz s7→.L810A4 slot=move v0,0; epilogue .L812CC. Tail has TWO
  identical [j .L812CC; slot move v0,a2] (jump-vs-jump cross-jump needs
  min-2 match; only 1). Arm-1's check-beqz slot = NOP naturally (sb trap/
  mem + la ineligible + guard jump — no steal candidates; VERIFIED in trace).
- Base ALLOCDBG s-order: p82 952 / p76 933 / p78 933 / p73 930.

## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin csmd4
  shares source text — marionation-unique anchors. Declare new_var/new_var3.
- d1/d2 stmt reorders flip s-regs via the 952 tie (do NOT re-derive).
- knob uids shift with any C change — tmp/mar_alllive_full.py re-discovers.

## Tools (local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1 + cc1_alllive (BB2_DBR_DEBUG, BB2_ALLLIVE_LABEL, ALLOC/QTY).
- s6 harness: tmp/mar_trigger_gen{,2,3,4,5,6,7}.py (batteries; r5d = the
  shape win; u-battery = &&-take proofs), tmp/mar_trace_labels.sh,
  tmp/mar_trace56.sh, tmp/mar_pass_trace.py (pass-2 retries),
  tmp/mar_check2_trace.py (check2 decision trace). Minis in tmp/mar_trig/.
- Verify kit: tmp/perm_finds_verify.py, tools/mar_test_candidate.sh,
  tmp/mar_floor_check.sh.
