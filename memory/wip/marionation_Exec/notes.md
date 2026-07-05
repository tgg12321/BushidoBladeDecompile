# marionation_Exec — mh5 masked 4; region-1 closing combo identified (s6g); region-3 = Window Theorem (2026-07-04/05)

## TL;DR
Floor: masked 4 (mh5 committed). S6G HALF-FLIP LANDED: in the o1 world,
staging the t0-SHIFT through a multi-set var kills its sched1 LAUNCH
(birthing_insn_p = live dest with reg_n_sets==1 — sched.c 2496/2566) and
s4 (`status = t0 << 2; t0 = (s32)((u8*)tbl_125c + status);` after the a5
stmt) = masked 7 with THE ARG5 SIDE FULLY FIXED (lw→v1, addu-v0, sw ✓).
Residuals in s4: t0 chain in a3/s0 not in-place-a0 + the 56/57 pair back.
Causes decoded: (a) status/cnt/i are GLOBAL pseudos (fixed regs s0/…) —
the stage var must be BLOCK-LOCAL multi-set (none exists; a fresh 2-set
local `sh = t0; sh <<= 2;` may work IF combine leaves reg_n_sets stale);
(b) a0 blocked for t0 because the CALL's fmt-la schedules BEFORE t0's last
use — the OLD ledger's "fmt-la late; chain4=a0" problem, now THE final
region-1 obstacle. Unstage discovery: the v0-staging FAKEs are INERT in
the o1 world (r1-r4 byte-identical) — removable. Region-3 unchanged
(Window Theorem; source-shape hole). NEXT: s4 + block-local stage +
fmt-la-late sweep = the closing combination for region-1.

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

## NEXT SESSION (in order)
1. REGION-1 CLOSING COMBO (from s6g's s4 at masked 7): o1 order + t0-shift
   staged multi-set (kills its launch; fixes arg5 ✓ measured) + (a) a
   BLOCK-LOCAL 2-set stage (`{ s32 sh; } sh = t0; sh <<= 2;` — verify
   combine leaves reg_n_sets stale; if not, other spellings) so the shift
   colors onto t0's dying reg, + (b) fmt-la LATE (the old x3 hypothesis:
   sweep 11DC/11D5/fmt arg spellings — e.g. fmt via a local, pp-style —
   until la a0 lands after lw a3,0(a0)@67) ⇒ t0 chain in-place a0. Oracle:
   masked 2. Harness: tmp/mar_o1_*.py + mar_s4_diff.py patterns.
2. Region-3 unchanged: source-shape hole hunt / permuter with the bare-slot
   beqz oracle. If the Window Theorem survives fresh eyes, surface to owner
   (options: keep hunting vs documented-plateau; NOT canonical-asm — the
   original is compiled C so a matching source EXISTS).

## Arm-2 transposition unchanged. Region-2 SOLVED. Region-1:
- S6F — THE O1 FRONTIER: C order [t0load; pp; v0ld; v0shl; a5; t0mul;
  t0add] = masked 8, PAIR ORDER PERFECT; residual = v1/a0 qty exchange.
  QTYDBG (marionation cluster; SEGMENT PER FUNCTION — twin collides):
  addr-109 (b16d20r2)→v0; t0-sll-111 (b18d24r2)→v1; arg5-104 (b20d26r2)→a0.
  qty_compare = floor_log2(refs)*refs*size/span DESC; 111v104 EXACT TIE →
  birth order. S6G: LAUNCH = birthing_insn_p (sched.c 2496): live dest AND
  reg_n_sets==1 → INSN_PRIORITY=max (adjust_priority 2566; n_deaths always
  0 — notes stripped). Multi-set dest ⇒ no launch ⇒ scheduled late ⇒ birth
  flip. s4 (status-staged shift) = masked 7, ARG5 SIDE FIXED; t0 side needs
  the block-local stage + fmt-la-late (TL;DR). Sub-facts: status/cnt/i =
  GLOBAL pseudos (fixed s0/…); v0/i stages 11-26 (their fixed regs); the
  v0-staging FAKEs are INERT in o1 (r1-r4 byte-identical — removable).
- MEASURED DEAD (s6e/f/g): mh5-basin arg5-staging + orders all >= 4,
  order-invariant; o1-basin copy-stages fold or 8-14; q15-q18 stale-refs
  vehicles fold (refs canceled — iq did NOT reproduce); t0load-late costs
  lbu@51; o3/o6/o8 collapse to mh5; shift-stages via cnt(11)/i(26)/v0(12)/
  new_var,new_var3(36-45 — NEVER touch the mask FAKEs); FRESH block-local
  2-set stage (u1-u3: sh=t0;sh<<=2 etc.) — combine MERGES the sets and
  properly updates reg_n_sets ⇒ launch returns ⇒ 8 (need a combine-
  resistant 2-set spelling, or lead with fmt-la-late on plain s4).
  Permuter finds all rejected (s6).

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
  shares text — uid spaces COLLIDE in TU-wide debug logs (segment per
  function!). Declare new_var/new_var3.

## Tools (local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1 + cc1_alllive (BB2_DBR/ALLLIVE/ALLOC/QTY/SCHED knobs).
- s6 harness: tmp/mar_trigger_gen{,2..7}.py + tmp/mar_*.py (traces, sweeps,
  diffs — see git log s6a-s6g for each tool's purpose; o1-world: mar_o1_*).
- Verify kit: tmp/perm_finds_verify.py, tools/mar_test_candidate.sh,
  tmp/mar_floor_check.sh. Minis: tmp/mar_trig/.
