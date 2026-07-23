# Evidence bank — func_80061658

## Floor
- Honest sandbox `--disable all` = **9** (46 target insns, 46 build insns). 0 regfix/asmfix.
  HEAD byte-matches only via two register pins: `register s32 t asm("$2")`,
  `register s32 mask asm("$3")` (cheat; stripped by sandbox).

## The gap is a PURE v0<->v1 REGISTER SWAP (s1 2026-07-22, RTL-confirmed) — corrects prior WIP
The prior WIP claimed "register-cycle + schedule coupling / interleaving not
reproduced." That is WRONG. Objdump of the honest (pin-stripped) build vs
`asm/funcs/func_80061658.s` shows the tail instruction ORDER and interleaving
are **byte-identical** to target (the mask `lui` is scheduled between load2 and
store2 in BOTH). The ONLY difference is register assignment in the 12-insn tail:
  - TARGET:   load-temp `t` (arg0[0..2]) -> **v0**;  mask 0x10FFFF -> **v1**
  - UNPINNED: load-temp `t` -> **v1**;               mask 0x10FFFF -> **v0**
Every tail insn mentioning v0/v1 differs -> exactly 9 differing insns = the floor.
It is a pure register-allocation choice, NOT scheduling.

## Mechanism (global.c allocno priority) — why unpinned GCC swaps
RTL greg dump (tmp/grind/func_80061658/s1/dumps/np.i.greg, unpinned): the load
temp lands in reg 3 (v1), the mask const in reg 2 (v0). MIPS `reg_alloc_order`
puts $2(v0) before $3(v1), so whichever allocno global.c processes first wins v0.
`t` is ONE reused C variable => one long-lived allocno spanning all three
load/store pairs (~9 insns, 6 refs); `mask` has a short scheduler-fixed live
range (~5 insns, ~3 refs). global.c priority = log2(n_refs)*freq/live_length:
mask's shorter live_length gives it the higher priority, so mask is allocated
FIRST and grabs v0; t falls to v1. The pins override this ordering.

## KILLED this session
- **mask-before-call** (put `mask=0x10FFFF;` before func_80060A68() to cross the
  call and force mask off v0 into v1): floor 9. Disasm shows GCC rematerializes
  the constant AFTER the call (never crosses it), so allocation unchanged.
  => Corollary: ANY source reordering of a *constant* mask is inert (its live
  range is scheduler-pinned to just before its single use). Explains why the 3
  prior WIP mask-move variants were all 9. See rejected/mask-before-call.c.

## Prior WIP rejected forms (banked; do NOT re-run)
- split `t` into t0/t1/t2 single-def temps — floor 11 (WORSE)
- move `mask=` after the 3rd load — floor 9
- compute `mask=` early (after call, before loads) — floor 9
- inline loads directly into global stores (no `t` temp) — floor 11 (WORSE)

## Family
Siblings share the t=$2 / mask=$3 pin pattern: func_80061710, 617C8, 618B4,
611A4, 6133C. A pure-C form that flips 61658 likely flips all. (NOT a park
proposal — no-new-park-categories forbids register-rotation infrastructure.)

- [s1] Honest sandbox --disable all = 9 (46/46 insns); 0 regfix/asmfix; byte-match only via cheat pins t->$2, mask->$3.

- [s1] Pin-stripped build tail is byte-identical in ORDER/interleaving to target; the entire 9-diff residual is the v0<->v1 swap between load-temp t and mask const.

- [s1] Unpinned RTL greg: load-temp -> reg 3 (v1), mask 0x10FFFF -> reg 2 (v0); target wants t->v0, mask->v1.

- [s1] Mechanism: global.c allocno priority — mask's short scheduler-fixed live range outranks t's one long reused-variable allocno, so mask wins v0 first.

- [s1] KILLED: mask-before-call — constant is rematerialized after the call, never crosses it; floor unchanged 9. Reordering a constant mask is inert in general.

- [s1] Prior banked negatives (do not re-run): split t into t0/t1/t2 = 11; move mask after 3rd load = 9; compute mask early = 9; inline loads no-temp = 11.

- [s1] Family sharing t=$2/mask=$3 pins: func_80061710, 617C8, 618B4, 611A4, 6133C — a real pure-C fix should generalize.

## s2 — floor lowered 9 -> 7 (Lever A), root cause = local-vs-global RMW asymmetry
- [s2] RTL root cause (cur.i.lreg/greg, current src): mask (pseudo 76) has two sets
  li(insn105)+ori(insn106) that are RMW-CHAINED -> ONE contiguous quantity -> allocated
  by local_alloc, which runs BEFORE global_alloc and grabs v0. Load-temp t (pseudo 75)
  has THREE disjoint load ranges -> local_alloc can't combine -> t goes to global_alloc
  -> gets leftover v1. local-before-global is why mask seats @v0. This is the mechanism
  behind the s1 "pure v0<->v1 swap".
- [s2] Lever A (register-alloc-pure-c.md, block-local split) LOWERS the floor:
  splitting the MIDDLE load into `{ s32 u = arg0[1]; D_800F1144 = u; }` makes `u` a local
  pseudo born before mask => u@v0, mask correctly @v1. Floor 9 -> 7. Emitted: load1@a0,
  load2(u)@v0, mask@v1(target!), load3@a0, with load2 hoisted (interleave distorted).
- [s2] Split-load3 = 8; split-load2 = 7 (best); split-load1 = 11; split loads1&3 = 11;
  3 scoped block-locals = 11; 3 named temps t0/t1/t2 = 11 (re-confirmed). Only the
  MIDDLE-load split reaches 7; outer/multi splits scatter temps to v0/v1/a0/a1 and break
  the interleave (scheduler hoists all no-anti-dep loads).
- [s2] Measured INERT (all floor 9, grouping-preserving reorders of single-t form):
  mask between load1/load2 (variant A); declaration-order swap (mask decl before t);
  [s1 banked] mask-early, mask-late, mask-before-call. Confirms pure reorder/decl-order
  of the single-temp form cannot flip the tiebreak.
- [s2] THE WALL to 0: single-t gives the TARGET SCHEDULE but wrong RA (mask@v0); every
  split gives target RA (mask@v1) but breaks the SCHEDULE. Reaching 0 needs BOTH: the
  single reused load-temp @v0 (for the interleave) AND mask@v1 — i.e. the disjoint-range
  shared load-temp must win v0 over the RMW-chained LOCAL mask, which local-before-global
  allocation forbids for these value shapes. No grouping-preserving structural transform
  changes the local/global classification. Next lever is the directed permuter (frontier
  #2), a different modality — NOT more structural splitting.

- [s2] Honest sandbox --disable all floor lowered 9 -> 7 this session (clean pure C, 0 rules/pins) via Lever A block-local split of the middle load.

- [s2] RTL root cause: mask (pseudo 76) is RMW-chained (li+ori) -> one contiguous LOCAL quantity -> local_alloc grabs v0 before global_alloc places the disjoint-range load-temp t (pseudo 75) into v1. local-before-global is the swap mechanism.

- [s2] Split-load sweep: load1=11, load2=7(best), load3=8, loads1&3=11, 3-scoped=11, t0/t1/t2=11. Only the middle-load split keeps the shared t=$loads1&3 and improves.

- [s2] Reorder/decl sweep (single-temp form): mask-mid=9, decl-order-swap=9, [banked] mask-early/late/before-call=9 - pure reorder cannot flip the tiebreak.

- [s2] The wall to 0: single-t gives the TARGET SCHEDULE but wrong RA (mask@v0); any split gives target RA (mask@v1) but breaks the SCHEDULE. Reaching 0 needs the disjoint-range shared load-temp to win v0 over the RMW-chained LOCAL mask - which local-before-global forbids for these value shapes; no grouping-preserving structural transform changes that classification.

- [s2] Both floor forms are clean pure C: the floor-9 form is a pure RA swap with the correct target schedule (cleaner permuter base); the floor-7 form has mask@v1 but a distorted interleave.
