# marionation_Exec — CANDIDATE MASKED 4 (mh5); region-3: near-total impossibility map ⇒ hunt the model-hole (2026-07-04 s6…s6d)

## TL;DR
Floor: masked 4 (mh5 committed candidate). S6F: REGION-1 IS ONE QTY-FLIP
FROM CLOSED — the o1 order (t0 chain last) yields PERFECT pair order at
masked 8 = v1/a0 exchange (6) + region-3 (2); the exchange is an EXACT
qty-pri tie (arithmetic below) broken by birth order; flip it ⇒ masked 2.
Region-3: the Window Theorem (s6d) stands — the nop has no byte-free
construction under the trace-validated model; the hole (which MUST exist —
the original is compiled C) is source-shape-dependent. r5d-on-real = masked
45 (referencer cascades allocation) — the label route is closed on the real
fn. Next: the sched1 lw-before-sll flip (below) closes region-1.

## THE WINDOW THEOREM (s6d — the impossibility map; every clause sourced)
Target window [beqz..move] = [sb] only; slot = NOP. One of these must hold
at BOTH fill_eager attempts (dbr runs fill+relax twice); each is closed:
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
check2 = insn 386; fall-walk EXACTLY matches the model (390 sb LOSE
setsopp+trap; 393 la LOSE len-2; 399 move WINNER; guard 402 setneed-refused
the move backward). The `mtlr target=399 block=0` anomaly is ANOTHER
function (log 584 vs marionation 790-897; uid collision). Marionation's
trace is CLEAN. mtlr's recursion (2810-31) INTERSECTS live sets ⇒ can't
create refusal. ⇒ the hole is SOURCE-SHAPE-DEPENDENT. NEW NEGATIVE
(mar_r5d_real.py): r5d-on-real w/ 1-insn opaque referencer = masked 45
(183/179) — one condition-read cascades the s-allocation (9th s-reg). No
referencer condition avoids the delicate pseudos (v0/check fold via
record_jump_equiv fall-side equalities; s-regs all in the 952/933 web).

## NEXT SESSION (in order)
1. REGION-1 IS ONE FLIP FROM CLOSED (see o1 frontier below): find the form
   whose SCHED1 output orders the arg5-lw BEFORE the t0-sll (qty birth
   flip); sched2 restores byte order automatically. Oracle: masked 2 in the
   o1 world. Levers left: sched1 launch/tiebreak surgery via BB2_SCHED on
   the o1 form (read the actual sll/lw priorities + luids), and permuter
   PERM_ perms over the o1 spelling (BASE = o1, not mh5!).
2. Region-3 unchanged: source-shape hole hunt / permuter with the bare-slot
   beqz oracle. If the Window Theorem survives fresh eyes, surface to owner
   (options: keep hunting vs documented-plateau; NOT canonical-asm — the
   original is compiled C so a matching source EXISTS).

## Arm-2 transposition: unchanged (d1/d2 952-tie; ref-bump CLOSED).
## Region-2 SOLVED — recipe in candidate.c. Region-1 s6e/s6f state:
- S6F BREAKTHROUGH — THE O1 FRONTIER: C order [t0load; pp; v0ld; v0shl; a5;
  t0mul; t0add] (t0 CHAIN LAST) = masked 8 with the PAIR ORDER PERFECT;
  residual = the v1/a0 qty EXCHANGE (t0 in v1, arg5 in a0) + region-3. The
  o1 world's alloc data (QTYDBG, marionation cluster = log lines 116-119 of
  tmp/rtl/marQ.log; segment per function — twin's cluster nearby has the
  ledger's p113/p100!): addr-temp 109 (b16 d20 r2)→v0; t0-sll-temp 111 (b18
  d24 r2)→v1; arg5 104 (b20 d26 r2)→a0. Comparator (local-alloc qty_compare)
  = floor_log2(refs)*refs*size/span, DESC; 111 vs 104 = EXACT TIE → qsort
  index order (birth) → 111 first. TARGET NEEDS 104 FIRST ⇒ pri(104) >
  pri(111): +1 surviving ref on arg5, or shorter 104-span, or longer
  111-span. KEY INSIGHT: qty birth order = SCHED1 output order, but emitted
  bytes = SCHED2 output — sched2 re-sorts by priority (sll's downstream
  path 3 > lw's 2 ⇒ sched2 puts sll first REGARDLESS) ⇒ the search target =
  a form whose SCHED1 stream has lw-BEFORE-sll; bytes self-restore. WIN
  SHOWS AS masked 2 (o1 world: exchange 6 + region-3 2 → flip = 2).
- MEASURED DEAD (s6e/f): mh5-basin arg5-staging (7) + orders (8) all >= 4
  and order-invariant; o1-basin stages (cnt/status/v0/b0/fused) 8-14 or
  fold-identical (cse copy-props single-set sources); add-then-subtract
  stale-refs vehicles (q15-q18) fold with refs properly canceled (iq's
  stale-refs did NOT reproduce); t0load-late (o9-o14) costs lbu@51 (9);
  o3/o6/o8 partial-chain moves collapse to mh5 codegen.
- Permuter verdicts (s6): 200-1/2=8, 200-3=5, csmd4 40-2=6 — REJECTED.

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
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin shares
  text — uid spaces COLLIDE in TU-wide debug logs (segment per function!).
  Declare new_var/new_var3. Knob uids shift with any C change.

## Tools (local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1 + cc1_alllive (BB2_DBR/ALLLIVE/ALLOC/QTY/SCHED knobs).
- s6 harness: tmp/mar_trigger_gen{,2..7}.py, tmp/mar_{trace_labels.sh,
  trace56.sh, pass_trace, check2_trace, ctx399, ctx_mtlr399, logseg,
  hole_hunt, r5d_real, arg5_sweep, arg5_diff, order_sweep, pair_sched}.py.
- Verify kit: tmp/perm_finds_verify.py, tools/mar_test_candidate.sh,
  tmp/mar_floor_check.sh. Minis: tmp/mar_trig/.
