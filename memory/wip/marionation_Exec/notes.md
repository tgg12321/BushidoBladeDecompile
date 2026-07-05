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

## REGION-1 — the final equation (s6h/s6j ground truth)
TARGET BYTES: t0 chain IN-PLACE in a0 (1070/1088/1098/10b0); fmt-la LAST
(10b4, outside t0's span — no staging needed); 11DC via $at macro; 11D5
macro on v0's second segment. mh5's fresh-temp basin = STRUCTURALLY
WRONG (register-masked mirage); true basin = IN-PLACE (ip1/ip2 masked 9,
shape exact; residual = v1/a0 exchange + 56/57 order + region-3).
EQUATION (inputs MEASURED s6j): t0 6refs/span24 pri 2.0 vs arg5
2refs/span6 pri 1.33 ⇒ t0 first ⇒ v1; target needs arg5 first (v1) so
t0 falls to a0. S6I CLOSURES: global-var hosting cascades ALL flavors (src
w1-w4=22; i-hosted x1-x4=22-32); suggestions = hard↔pseudo COPIES only
(combine_regs 1815-62; arg5=stack ⇒ impossible; t0 meets no hard copy);
NO qty rebirth (reg_is_born: alloc_qty only at reg_qty==-2);
REG_ALLOC_ORDER absent ⇒ regno first-fit confirmed. ⇒ SECOND
IMPOSSIBILITY THEOREM: arg5's pri is PINNED (stack arg) and ANY local
carrier of t0's value across [58,61] outranks it ⇒ takes v1 first ⇒ the
TARGET allocation CANNOT arise from two plain local qtys under
qty_compare. The bytes exist ⇒ a model-hole exists. Sub-facts: in-place
= `<<=`/`+=` (NOT `*=`); fmt staging cse-inert. Oracle: masked 2.
Region-3 unchanged (Window
Theorem). find_free_reg read: model CONFIRMED end-to-end. S6J: qty
inputs MEASURED on ip1 (tmp/mar_ip_qty.py): t0 = 6refs/span24 pri 2.0
→ v1 (ord 2); arg5 = 2refs/span6 pri 1.33 → a0 (ord 3) — theorem's
inputs verified on the machine. m2c EVIDENCE (tools/m2c on the target
asm): the call spelled fully in-call, 2nd arg = DIRECT global (no pp!)
— but m2c is VALUE-faithful not structure-faithful: the pure in-call
form measures masked 16 with the WRONG shape (sequential arg chains;
ours idx1-first) ⇒ original HAD locals that sched interleaved = the
ip/o1 family confirmed from a second direction. In-call arg5 hybrids
(h1-h4 = 13-16): the span-2 pri-4.0 arg5 temp allocates first but the
sw moves to 60 (target 63) — shape broken. LAST HOLE CANDIDATE
(global.c:388): pseudos local-alloc FAILS to allocate fall to GREG
(different pri formula + reg preference) — if the original's block was
dense enough that t0-or-arg5 FAILED local, greg's rules apply. NEXT:
probe what makes a qty fail local (span conflicts) + read find_reg's
preference order. PERMUTER RELAUNCHED on the ip1 base (tmp/perm_mar_ip,
base score 260, running) — finds → perm_finds_verify pattern.

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
