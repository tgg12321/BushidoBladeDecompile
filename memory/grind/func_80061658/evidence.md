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
