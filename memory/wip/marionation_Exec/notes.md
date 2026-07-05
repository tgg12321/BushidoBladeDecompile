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

## THE WINDOW THEOREM (s6d; premises = the OLD tail — re-verify in s6l's!)
Slot=NOP needs one of these at BOTH fill_eager attempts; each closed:
1. a1 live at after_blocks ⇒ $5 pseudo live there ⇒ crosses the vsync
   call ⇒ s-reg. CLOSED.
2. everything-live ⇒ post-flow-minted target label (every flow-era label
   is its own basic_block_head). Minters: do_cross_jump (mid-block ⇒
   b_far, not -1), USE-hoist (wrong side), relax 3992/4002 (FILLED-SEQ-
   only ⇒ exclusive with refusal; filled slots never empty). CLOSED.
3. Walk-blocker in [beqz..move]: referenced labels need a referencer
   alive to FINAL (=bytes); &&-take dies; forced_labels = EXPAND_
   INITIALIZER-only; LABEL_PRESERVE_P nonlocal/eh-only; near-label
   referencers fold pre-flow; far = bytes + cascade; every relax deleter
   fires pass-1 (pass-2 retry steals, measured) or needs a gap-insn =
   bytes; 4031 re-emits inline; USE-markers walk-skipped +
   next_active-invisible post-reload. CLOSED.
4. Filled-then-emptied: 3956 scan stops at labels ⇒ window {lbu; andi};
   move never matches; matching lbu trap-blocked from steals; recompute
   dests flow-deleted; try_merge never empties own slot. CLOSED.
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
ledger's exact inequality) + the 56/57 pair + region-3. S-WEB DATA (ALLOCDBG, C_iq_y1): saved-y1 = 3636→s1 ✓ TARGET;
i1496-iq = 9r/1800→s2 ✗ (overshoots); arg1 = 4r/84/952→s3 ✗; i1494 =
7r/150/933→s4 ✗. NEEDED: p76 > p78 > p73 after saved — arithmetic says
+1 ref each on p76/p78 (→1066 tie, ascending seats p76 first) with p73
at 952. Vehicle attempts: natural head (idx_1495 = idx_1494+1 + F19C0
direct — target bytes DO show la v0,D_80016248 direct + addiu-from-s2!)
= 30 ALL SUBSETS (the folds the FAKEs guard against happen; the FAKEs
are load-bearing even though the byte SHAPE matches the natural form);
i5-split = 30 (p77 jump). NEXT (mechanical, ALLOCDBG-guided): find +1
p76 / +1 p78 vehicles that don't touch p77/p73 — candidates: an extra
*idx_1494/*idx_1496 read folded into an existing insn (duplicated-
statement family), F19C0-FAKE rebased from idx_1496→idx_1494 (+1 p76
-1 p78 — wrong direction alone but pairs with iq's +2), y1 with iq only
(22, marginal — its web: re-measure). Then: 56/57 pair + region-3 in
THIS world (after_blocks label GONE ⇒ re-run the check2 fill trace —
Window Theorem premises may not hold!). Permuters: perm_mar_ip (-j6) +
perm_csmd4 (-j4) running; 210-1 REJECTED (ptr-arith), 260-1/2 ties.

## Region-2 SOLVED. Archive: o1=8 pair-order-perfect; s4=7 (launch
lever); qty_compare = flog2(r)*r*size/span DESC; status/cnt/i GLOBAL.
- MEASURED DEAD (s6e-l): mh5 staging+orders; o1 copy-stages; q15-18;
  t0load-late; o3/o6/o8; shift-stages; u1-u3; fmt staging; ipA/B/C;
  src/i-hosting; h1-h4 in-call; m1-m5 pure in-call; whole-m2c (28);
  i5-split (30); natural-head N-battery (30 all subsets — head FAKEs
  are load-bearing despite matching byte shapes); iq+y1 = 22 marginal.

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
