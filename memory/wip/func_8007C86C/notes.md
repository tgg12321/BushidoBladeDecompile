# func_8007C86C — WIP (current state 2026-08-04, round 15)

GP0 drawing-area packet builder (`0xE4000000`): clamps x to `[0, D_8009BE78-1]`
and y to `[0, D_8009BE7A-1]`, then packs them into a GP0 word at 10/10 or 12/12
bits depending on `D_8009BE74`. Twin of `func_8007C7A0` (`0xE3000000`, constant
differs only) — solve one, apply to both. 21 regfix substs at
`regfix.txt:3050-3071`; 0 asmfix; no prologue_config / frame_fix entries.

## Where it stands

| body | score | build_insns | stream vs target (51) |
|---|---|---|---|
| committed HEAD | 20 | 50 | frame wrong (vars=24 vs 16) |
| `candidate.c` (rounds 1-14) | **12** | 50 | park insn MISSING |
| `candidate_frame51.c` (round 15) | 13 | **51** | **1:1 in kind and order** |

`candidate_frame51.c` is the structurally closest form in 15 rounds: all 51
instructions match target in kind and order, and **every remaining difference
is a register NAME** — ours→target: carrier `a0`→`a3`, limit-save `v1`→`a2`,
arm temp `a0`→`v0`, tail constant `v0`→`a0`. It costs +1 on the masked-
Levenshtein metric (13 vs 12) only because that metric counts renames.

The lever that closed the long-standing "MISSING park insn" gap is a staged
carrier plus a join copy:

```c
s32 x = arg0;            /* carrier, live across the clamp */
s16 tx;
if (arg0 >= 0) {
    if ((D_8009BE78 - 1) < arg0) tx = D_8009BE78 - 1; else tx = x;
} else tx = 0;
x = tx;                  /* the join copy = target's `move a3,v0` */
```

## Frame is NOT an open problem (correction, 2026-08-04)

The frame-mismatch census flagged this function as reserving 8 bytes too many
(delta −8). That measured the **committed HEAD body**. `candidate.c` already
emits `frame=16 / vars=16` — correct — via its `s16 var_v0_2 / s16 var_a1`
locals. There was never an inverse-phantom problem here; HEAD is just an older,
worse shape. Correction recorded in `memory/wip/_frame_census_2026-08-04.md`.

## The remaining gap — mechanism (round 11, instrumented cc1)

`$a2`-vs-`$a3` X-preserve tiebreaker. ALLOCDBG on the candidate body: pseudo 72
(the X-preserve backup) has priority **1818**, the LOWEST in the 11-pseudo
allocno sort (nrefs=2, livelen=11). Allocation walks ascending and stops at the
first free register, so it lands in `$a2`; target has it in `$a3`. Flipping it
requires some other pseudo to own `$a2` across pseudo 72's live range with
higher priority — the **chain-extension** mechanism, FORBIDDEN per
[[register-alloc-pure-c]] §6. Corroborated empirically by every round since.

## Do NOT re-run (measured negative / inert)

- **Permuter** — ~125k cumulative iters across 4 independent runs (rounds 1, 2,
  12 random; round 13 directed PERM_GENERAL with 5 hand-authored spots). Every
  sub-baseline candidate is a forbidden family: dead-conditional-store,
  semantic change (zeroing Y on the arg0<0 path), UB (uninitialized `var_a1` /
  `var_v1` reads), synthetic shared-zero routing, named-intermediate-for-bool.
- **Clamp shape** — if/else, ternary, assign-first, nested, goto-form, switch
  dispatch, unsigned single-compare, `arg0 >= D` compare: 14-27.
- **Arm ordering** — `<0`-first regresses to 16-28 (rounds 4, 6, 15).
- **Carrier / param forms** — SOTN param-reassignment (16-28), local copy of y
  (+4 over param-in-place), staged-temp sourced from `arg0` instead of the
  carrier (20), s32 signature widening (24).
- **Types** — u16/u32 on the clamp locals and casts at mask sites: inert
  (combine erases the distinction under the 0x3FF/0xFFF masks). s32 widening
  of `var_v0_2`: 17.
- **Named intermediates** — un-decremented limit local (inert), `mode_m1`
  hoist (23), `arg1_neg` sign hoist (28), limits-pointer share (18).
- **Declaration order** — all permutations inert (pseudo numbers for autos do
  not track decl order).
- **Tail OR association** — `hi|(lo|C)`, staged `lo|C`, `(lo|C)|hi`: inert or +1.
- **Precomputes** — narrow-mask X precompute (18), identity precompute (22),
  single-expression return (22-24).
- **TU re-attribution / rodata reorder** — independently FAILED by
  cheat-reviewer 2026-06-05 (commit `2e5098e9`): all four
  [[no-new-park-categories]] evidence criteria unmet; the globals are already
  display.c-exclusive and the 21 rules are a codegen signature, not a layout one.

## Build gate

Applying either candidate breaks the oracle (score-13 form → SHA1 `b4122d17`)
because the 21 regfix substs are calibrated to HEAD's emission shape. Landing
one requires rewriting or retiring those rules — a completion-gate activity,
not a worker one. src was reverted after every experiment and the full build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Round 16 (2026-08-04) — stream-exact form, verified on this twin

`candidate_stream51.c` (banked here, twin-adapted from C7A0) is the first body
whose 51 instructions match target one-for-one in opcode, operand shape and
order. Measured on THIS function: 51/51 insns, `sandbox --disable all` score 15
— identical to C7A0, confirming the twins move in lock-step. The score is higher
than the old 12/13 only because masked Levenshtein counts renames; those forms
carried real structural errors and this one does not.

Two levers: an **`s16` carrier** (target's `move a3,a0` copies the raw halfword
before sign-extension, and the join copy carries no extension) and a **named
tail temp** (`pkt = lo | 0xE4000000; return hi | pkt;`) to pin the OR
association GCC otherwise reassociates.

## Resume here

Start from **`candidate_stream51.c`**. The only open question is the register
assignment. Do not re-run clamp-shape sweeps or another permuter campaign; that
space is covered. Round 16 localized the blocker: with no `REG_ALLOC_ORDER` for
MIPS in this tree both allocators scan ascending, and a 2-param leaf function
has no hard `$a2`/`$a3`/`$v1` for `set_preference` to key on — so those
registers are reachable ONLY by exclusion, never by preference. Full mechanism,
the measured negative list, and the reusable probe tooling are in
`memory/wip/func_8007C7A0/notes.md`; per-round narrative for rounds 1-14 is in
git history for this file.

Sibling: `memory/wip/func_8007C7A0/` — identical pattern and floor.
