# marionation_Exec — mh5 masked 4; region-1 closing combo identified (s6g); region-3 = Window Theorem (2026-07-04/05)

## TL;DR
Committed floor: masked 4 (mh5) — BUT s6h proves mh5's basin structurally
wrong: TARGET BYTES have the t0 chain IN-PLACE in a0 and the fmt-la LAST
(dumped; see NEXT). True basin = in-place (ip1/ip2 at masked 9, shape
exact); the WHOLE remaining region-1 = the v1/a0 allocation equation in
the NEXT section + the 56/57 order (C-luid of the t0-shl). s4 (status-
staged shift, masked 7) proved the launch lever works (arg5 side fixed)
but its s0-shift is byte-wrong — superseded by the in-place reading.
Launch law (s6g): birthing_insn_p = live dest with reg_n_sets==1
(sched.c 2496/2566); `*=` synthesizes a launching temp, `<<=`/`+=` stay
in-place. The v0-staging FAKEs are byte-inert in o1 (removable).
Region-3 unchanged (Window Theorem; source-shape-dependent hole).

## THE WINDOW THEOREM (s6d — impossibility map; every clause sourced)
Slot=NOP needs one of these at BOTH fill_eager attempts; each closed:
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

## Validated ground truth (s6d — hole-hunt step 1 DONE)
check2 = insn 386; fall-walk matches the model exactly (sb LOSE
setsopp+trap; la LOSE len-2; move WINNER; guard setneed-refused the move
backward). mtlr block=0 anomaly = another function (uid collision);
marionation's trace CLEAN; mtlr recursion (2810-31) INTERSECTS ⇒ can't
refuse. ⇒ hole is SOURCE-SHAPE-DEPENDENT. r5d-on-real = masked 45 (one
condition-read cascades the s-allocation; no condition avoids the
delicate pseudos — record_jump_equiv fall-equalities fold v0/check).

## NEXT SESSION — REGION-1, the final equation (s6h ground truth)
TARGET BYTES (dumped, tmp/mar_final_matrix era): t0 chain IN-PLACE in a0
(1070 lbu a0; 1088 sll a0,a0; 1098 addu a0,a0,s5; 10b0 lw a3,0(a0));
fmt-la LAST (10b4, after t0's death) — hard-a0 set outside t0's span, NO
staging needed; 11DC access via $at MACRO (lui at/addu at/lw a2 = ONE RTL
insn); 11D5 via lui+lbu macro reusing V0's second segment (1090-109c ✓
ours matches). ⇒ mh5's fresh-temp basin is STRUCTURALLY WRONG (masked 4 is
a register-masked mirage); the true basin = IN-PLACE (ip1/ip2: o1 order +
`t0 <<= 2; t0 += (s32)tbl_125c;` = masked 9, shape exact, residual = the
v1/a0 exchange + the 56/57 order + region-3). THE EQUATION: first-fit
gives t0→a0 only if v0 AND v1 are occupied at t0's allocation ⇒ ARG5 MUST
ALLOCATE BEFORE T0's one-qty ⇒ need pri(arg5) > pri(t0): t0 (in-place, 1
qty born@51 dies@67: ~6refs/span16: flog2(6)*6/16 = 0.75) vs arg5 (2refs/
span5 = 0.4). Levers: shrink t0's refs (4refs ⇒ 0.5 — still >) or SPAN
(load later? bytes pin 51…67) or SPLIT t0's qty (the 11D5-style SECOND
SEGMENT — v0 got reused mid-block and its qty segments…: check whether a
multi-set pseudo whose value FULLY DIES mid-block gets a FRESH QTY per
local-alloc (reg_qty rebirth!) — if yes, spelling t0's chain so the value
dies at 58ish and REBIRTHS shortens spans below arg5's pri!), or +1 arg5
ref that survives (window: reg-sourced only). Sub-facts: fmt-la staging
is cse-inert (const source); in-place C spelling = `<<=`/`+=` (NOT `*=` —
the mult path synthesizes the launching temp!); the 56/57 order flip
needs the t0-shl's C-luid < a5-stmt's (try [t0load; pp; v0ld; v0shl;
t0shl; a5; t0add]-family in the in-place basin!). Oracle: masked 2.
Region-3 unchanged (Window Theorem; source-shape hole; surface to owner
if it survives fresh eyes — NOT canonical-asm, the original is compiled C).

## Arm-2 transposition unchanged. Region-2 SOLVED.
- S6F/G archive: o1 order = masked 8 pair-order-perfect; QTYDBG cluster:
  addr-109→v0, sll-111→v1, arg5-104→a0; qty_compare = flog2(refs)*refs*
  size/span DESC, tie → birth. s4 (status-staged shift) = 7, arg5 side
  fixed (launch lever proven). status/cnt/i = GLOBAL pseudos.
- MEASURED DEAD (s6e-h): mh5-basin arg5-staging + orders (>= 4, order-
  invariant); o1-basin copy-stages fold or 8-14; q15-q18 stale-refs fold
  (refs canceled — iq did NOT reproduce); t0load-late costs lbu@51;
  o3/o6/o8 collapse; shift-stages cnt(11)/i(26)/v0(12)/new_var*(36-45 —
  mask FAKEs load-bearing!); fresh 2-set stage u1-u3 (combine merges +
  updates refs ⇒ launch returns); fmt staging cse-inert (const source);
  ip3 (in-place C-early: sll@55); ipA/B/C (in-place basin is ALSO order-
  invariant across shl/add C placements — the 56/57 order is priority-
  driven there; the exchange levers are the sole remaining dials).
  Permuter finds rejected.

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5, i1495
  s6, arg0 s7; check a2, src a0, i v1, b v0, dst/dst2 a1.
- arm-2: sb -1(s3); move a1,s4; la F19A8; beqz a1→.L812BC slot=li v1,7;
  li a3,-1; loop; .L812BC: j .L812CC; move v0,a2. check2 beqz slot = NOP.
- .L812C4: beqz s7→.L810A4 slot=move v0,0; epilogue .L812CC. Tail has TWO
  identical [j .L812CC; slot move v0,a2] (jump-vs-jump cross-jump needs
  min-2 match; only 1). Arm-1's check-beqz slot = NOP naturally (sb trap/
  mem + la ineligible + guard jump — no steal candidates; VERIFIED in trace).
## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin
  shares text — uid spaces COLLIDE in TU logs. Declare new_var/new_var3.

## Tools (local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1 + cc1_alllive (BB2_DBR/ALLLIVE/ALLOC/QTY/SCHED knobs).
- s6 harness: tmp/mar_trigger_gen{,2..7}.py + tmp/mar_*.py (traces, sweeps,
  diffs — see git log s6a-s6g for each tool's purpose; o1-world: mar_o1_*).
- Verify kit: tmp/perm_finds_verify.py, tools/mar_test_candidate.sh,
  tmp/mar_floor_check.sh. Minis: tmp/mar_trig/.
