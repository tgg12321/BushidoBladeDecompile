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

## s6d ground truth: check2's fill trace matches the model; r5d-on-real
= 45 (condition-read cascades); record_jump_equiv folds v0/check.

## REGION-1 ground truth (s6h/j): t0 chain IN-PLACE in a0; fmt-la LAST
(no staging needed); 11DC via $at macro; 11D5 on v0's 2nd segment.
mh5 basin structurally wrong (masked mirage). Local-qty equation
(measured): t0 2.0 vs arg5 1.33 ⇒ t0→v1 — SOLVED BY THE M2C TAIL (s6l,
below), which changes the pseudo landscape wholesale. S6I archive: global-hosting cascades (src=22, i=22-32);
suggestions = hard↔pseudo copies only; no qty rebirth; regno first-fit.
SECOND IMPOSSIBILITY THEOREM (in the OLD tail): the target allocation
cannot arise from two plain local qtys — RESOLVED by s6l: the original's
TAIL STRUCTURE (m2c) changes the landscape so the theorem's premises
don't apply. In-place = `<<=`/`+=` (NOT `*=`); fmt staging cse-inert.
Region-3 (Window
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
preference order. S6L BREAKTHROUGH — THE M2C TAIL FIXES THE EXCHANGE: full m2c read
(tmp/m2c output; save it!) gave the original's control shape; the TAIL
transplant (tmp/mar_m2c_tail.py: arm-1 `goto done` INTO arm-2's if-body
(done: inside the arm = .L812BC's 2 jumpers ✓); arm-2 order [sb;
dst2=a1; src; i] (move-FIRST = the transposition natively!); after-code
as FALL-THROUGH `v0 = 0; if (a0 != 0) return v0; goto loop;` (no
after_blocks label; v0=0 = beqz-s7's backward-fill ✓)) yields masked 23
with REGION-1's registers RIGHT (t0→a0, lbu a0@51 ✓✓) — the exchange
GONE. Residual = s-web ROTATION (p73's 84-luid 952-tie, the documented
d1/d2 coupling: ALLOCDBG T_mh5: p73(4r/84/952)→s1 ✗, p82(2r/21/952)→s2,
p76/p78(7r/150/933)→s3/s4; TARGET needs p82>p76≥p78>p73 = the OLD
ledger's exact inequality) + the 56/57 pair + region-3. Old vehicles
re-measured here: iq+y1 = 22 (marginal); i5-split = 30 (backfires —
extra move); whole-m2c = 28 (timeout respell also rotates). NEXT: (1)
ALLOCDBG on C_iq_y1 → the new pri numbers → re-derive byte-clean bumps
IN THIS LANDSCAPE (p73 needs <933: len 86 (+2 luids via a late a1 read)
or refs 3; p82/76/78 bumps); (2) then the 56/57 pair in this world; (3)
region-3 in this world (the fill landscape CHANGED — after_blocks label
GONE ⇒ re-run the check2-fill trace — the Window Theorem's premises may
not hold here!!). Permuters: tmp/perm_mar_ip (-j6) + perm_csmd4 (-j4)
running; finds 210-1 (pointer-arith semantics) REJECTED, 260-1/2 ties.

## Region-2 SOLVED. Archive: o1=8 pair-order-perfect; s4=7 (launch
lever); qty_compare = flog2(r)*r*size/span DESC; status/cnt/i GLOBAL.
- MEASURED DEAD (s6e-l): mh5-basin staging+orders; o1 copy-stages;
  q15-18 stale-refs; t0load-late; o3/o6/o8; shift-stages (mask FAKEs
  load-bearing!); u1-u3; fmt staging; ipA/B/C; src/i-hosting (cascades);
  h1-h4 in-call arg5 (sw drags); m1-m5 pure in-call (sequential shape);
  whole-m2c (28 — timeout respell rotates s-web too); i5-split (30 —
  extra move); iq+y1 in the new tail = 22 (marginal only).

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
