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
1. a1 live at after_blocks ⇒ needs a $5 pseudo live there ⇒ crosses the
   loop's vsync call ⇒ s-reg, never $5. CLOSED.
2. everything-live ⇒ find_basic_block==-1 ⇒ post-flow-minted target label
   (every flow-era label is its own basic_block_head). Minters:
   do_cross_jump (mid-block ⇒ b_far, not -1), USE-hoist (wrong side),
   relax 3992/4002 (FILLED-SEQ-only per 3946 ⇒ exclusive with refusal;
   filled slots never empty — see 4). CLOSED.
3. Walk-blocker in [beqz..move]: labels un-own (3397) / jumps stop the
   walk, but referenced labels need a referencer alive to FINAL (=bytes).
   &&-take dies (unused: never expands; dead-store: flow+jump2 sweep);
   forced_labels = EXPAND_INITIALIZER-only (expr.c 4137 ⇒ data bytes);
   LABEL_PRESERVE_P nonlocal/eh-only. Referencer-jumps: near-label folds
   pre-flow at ANY condition; far = bytes + CASCADE (r5d-real 45); every
   relax deleter fires pass-1 (⇒ pass-2 retry steals, measured) or needs
   a gap-insn = bytes (sb mem/trap-unstealable; la len-2; move guard-
   live); 4031 RE-EMITS slots inline; USE-markers walk-skipped (3404),
   next_active-invisible post-reload (emit-rtl 1855). CLOSED.
4. Filled-then-emptied: 3956 scan stops at CODE_LABELs (2001) ⇒ window =
   {lbu; andi}; the move never matches; a matching lbu can't be target-
   stolen (may_trap 3461; backward simp reaches only pre-branch insns);
   recompute dests dead ⇒ flow deletes; volatile unstealable. try_merge
   never empties the caller's own slot (annul dead on MIPS). CLOSED.
⇒ The original EXISTS ⇒ a hole exists — upstream in source shape.

## Validated ground truth (s6d — hole-hunt step 1 DONE)
check2 = insn 386; fall-walk matches the model exactly. mtlr block=0
anomaly = another function (uid collision); marionation's trace CLEAN;
mtlr recursion INTERSECTS ⇒ can't refuse. r5d-on-real = masked 45 (one
condition-read cascades; record_jump_equiv folds v0/check conditions).

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
span5 = 0.4). S6I CLOSURES: global-var hosting cascades ALL flavors (src
w1-w4=22; i-hosted x1-x4=22-32); suggestions = hard↔pseudo COPIES only
(combine_regs 1815-62; arg5=stack ⇒ impossible, t0 meets no hard copy);
NO qty rebirth (reg_is_born 2000-27: alloc_qty only at reg_qty==-2; one
pseudo = one monolithic qty); REG_ALLOC_ORDER absent ⇒ regno first-fit
(v0,v1,a0…) confirmed; 11D5-shift temp (pri 4.0) owns v0 first — matches
target. ⇒ SECOND IMPOSSIBILITY THEOREM: pri(arg5) is PINNED at 1.6
(2refs/span5, stack arg) and ANY local carrier of t0's value across
[58,61] has ≥2refs/span≤4 ⇒ pri ≥ 2.0 > 1.6 ⇒ allocates before arg5 ⇒
takes v1 ⇒ the TARGET allocation (arg5→v1 before t0→a0) CANNOT arise
from two plain local qtys under qty_compare. Same epistemic shape as the
Window Theorem: the bytes exist ⇒ a model-hole exists — in UNREAD
local-alloc corners: qty_min_class (constraint-driven class narrowing!),
the sugg-round fallback, requires_inout/may_save_copy tie paths,
retry_global_alloc, or reload-inheritance. NEXT: read those (bounded);
then re-derive the vehicle. Sub-facts: in-place = `<<=`/`+=` (NOT `*=`);
fmt staging cse-inert. Oracle: masked 2. Region-3 unchanged (Window
Theorem). find_free_reg read (s6i2): arg hard-regs live setup→call;
a0[68,70] vs t0[51,67] disjoint ✓; call-crossing qtys exclude call-used
regs; model CONFIRMED end-to-end ⇒ CONVERGENT CONCLUSION (both regions,
triple-confirmed): local perturbations of THIS candidate cannot close
either gap — the original had a GLOBALLY different variable structure.
Search must move to whole-function shapes: permuter big-jump mode over
variable roles + m2c re-read of the original structure + the twin's
eventual crack (shared arithmetic transfers).

## Arm-2 transposition unchanged. Region-2 SOLVED.
- S6F/G archive: o1 = masked 8 pair-order-perfect; qty_compare =
  flog2(refs)*refs*size/span DESC, tie → birth; s4 (status-staged shift)
  = 7, arg5 side fixed (launch lever). status/cnt/i = GLOBAL pseudos.
- MEASURED DEAD (s6e-h): mh5-basin arg5-staging + orders (>= 4, order-
  invariant); o1-basin copy-stages fold or 8-14; q15-q18 stale-refs fold
  (refs canceled — iq did NOT reproduce); t0load-late costs lbu@51;
  o3/o6/o8 collapse; shift-stages cnt(11)/i(26)/v0(12)/new_var*(36-45 —
  mask FAKEs load-bearing!); fresh 2-set stage u1-u3 (combine merges +
  updates refs ⇒ launch returns); fmt staging cse-inert (const source);
  ip3 (in-place C-early: sll@55); ipA/B/C (in-place basin ALSO order-
  invariant — priority-driven); w1-w4 src-hosted chain = 22 (extending a
  GLOBAL var's range cascades the web — same lesson as r5d; global-var
  hosting is closed as a t0→a0 vehicle). Permuter finds rejected.

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
