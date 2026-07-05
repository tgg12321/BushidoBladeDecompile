# marionation_Exec — CANDIDATE MASKED 4 (mh5); region-3: near-total impossibility map ⇒ hunt the model-hole (2026-07-04 s6…s6d)

## TL;DR
Floor unchanged (masked 4 = region-1 pair 2 + region-3 2; candidate.c = mh5).
S6d: THE WINDOW THEOREM (below) — the check2 NOP has NO byte-free pure-C
construction under the now-nearly-complete reorg model; every corridor
source-proven or mini-measured dead. The ORIGINAL produced the nop from real
C ⇒ the model has a hole ⇒ hole-hunt step 1 DONE: marionation's own trace is
model-clean; the hole is SOURCE-SHAPE-DEPENDENT (different upstream C ⇒
different fills). r5d (s6b) = the confirmed refusal SHAPE (arm-head label +
live referencer, exact target bytes) but r5d-on-real measures masked 45 —
the referencer's one condition-read cascades the allocation. Permuters own
the search from the mh5 base; region-1 is the softer half to close first.

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
   but referenced labels need a referencer-insn alive to FINAL (= bytes).
   &&-take dies (unused: never expands — k; dead-store in live var: expands,
   flow kills the set, jump2 sweeps the label — u + label-56 trace);
   forced_labels is EXPAND_INITIALIZER-ONLY (expr.c 4137 ⇒ static-data
   bytes); LABEL_PRESERVE_P is nonlocal/eh-only (stmt.c 665/3085).
   Referencer-jumps: near-label folds pre-flow at ANY condition (round-6);
   far = bytes + ALLOCATION CASCADE (r5d-real masked 45!); every relax
   deleter's precondition held already in pass-1 (dies early ⇒ pass-2 retry
   steals — retries measured) or needs a gap-insn X = bytes (X=sb
   unstealable mem/trap; X=la ineligible len-2; X=move guard-live); 4031
   dissolution RE-EMITS slots inline; USE-markers walk-skipped (3404) and
   next_active-invisible post-reload (emit-rtl 1855). CLOSED.
4. Filled-then-emptied: 3956 redundancy scan stops at CODE_LABELs (2001) ⇒
   window = {lbu; andi} only ⇒ the move never matches; a matching lbu can't
   be target-stolen (loads fail may_trap 3461; backward simp reaches only
   pre-branch insns); recompute dests all dead-at-after_blocks ⇒ flow
   deletes; volatile loads unstealable. try_merge (1815-1964) deletes thread
   copies or the OTHER SEQ's slots, never the caller's own (annul dead on
   MIPS). CLOSED.
⇒ Either one of ~30 derivation links is wrong (compound ~30-50%!), or the
original source shaped an upstream difference not yet modeled. The original
EXISTS ⇒ the hole exists. FIND IT EMPIRICALLY.

## Validated ground truth (s6d traces — hole-hunt step 1 DONE)
check2 = insn 386; fall-walk EXACTLY matches the model: 390 sb LOSE
setsopp+trap; 393 la LOSE quiet (len 2 — arm-1's bare slot same cause); 399
move WINNER. Guard 402 setneed-refused the move backward; arm-1's guard took
its move backward — all match bytes. The `mtlr target=399 block=0` anomaly
is ANOTHER function (log line 584 vs marionation's 790-897; uid spaces
repeat per function — mar_logseg.py segments by fingerprints). Marionation's
trace is CLEAN — no hole in THIS compilation. mtlr's recursion (2810-31)
INTERSECTS live sets (AND) ⇒ can't create refusal; check2's target scan
breaks at the conditional beqz-s7 first. ⇒ the hole is SOURCE-SHAPE-
DEPENDENT. NEW NEGATIVE (mar_r5d_real.py): r5d-on-real with the 1-insn
opaque referencer (`if (idx_1494==0) goto arm;`) = masked 45 (183/179): one
condition-read cascades the s-allocation (9th s-reg, prologue reshuffles).
No referencer condition avoids the delicate pseudos (v0/check fold via
record_jump_equiv fall-side equalities; all s-regs in the 952/933 tie web).

## NEXT SESSION (in order)
1. Permuters own the mechanical search: keep base = mh5 (r5d-real is masked
   45 — NOT a seed). Add PERM_ variants of the check2/after_blocks region
   spellings if directed mode is used; verify finds w/ perm_finds_verify.py.
2. Trace-diff harness generalization: for ANY candidate variant batch, grep
   the emitted .s for the bare-slot beqz (the refusal oracle is cheap) —
   tmp/mar_trigger_gen*.py pattern applies to real-function sweeps too.
3. Region-1 (the pair, 2 lines) is now the SOFTER half: g3 v1/a0 qty
   exchange, untried multi-set arg5 VALUE-staging keeping the lw dest split.
   Closing region-1 first would make any region-3 experiment cleanly
   measurable at masked 2.
4. If the Window Theorem also survives fresh eyes: surface to the owner —
   region-3's evidence (this file) + options: keep hunting the source-shape
   hole vs a documented-plateau disposition (NOT canonical-asm; the
   original is compiled C, so a matching source EXISTS — the theorem having
   a hole is CERTAIN, only its location is unknown).

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
- s6 harness: tmp/mar_trigger_gen{,2,3,4,5,6,7}.py (batteries), tmp/mar_
  {trace_labels.sh, trace56.sh, pass_trace.py, check2_trace.py, ctx399.py,
  ctx_mtlr399.py, logseg.py, hole_hunt.py, r5d_real.py}. Minis: tmp/mar_trig/.
- Verify kit: tmp/perm_finds_verify.py, tools/mar_test_candidate.sh,
  tmp/mar_floor_check.sh.
