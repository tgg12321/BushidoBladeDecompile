# func_8007C7A0 — WIP (current state 2026-08-04, round 16)

GP0 drawing-area packet builder (`0xE3000000`): clamps x to `[0, D_8009BE78-1]`
and y to `[0, D_8009BE7A-1]`, then packs them into a GP0 word at 10/10 or 12/12
bits depending on `D_8009BE74`. Twin of `func_8007C86C` (`0xE4000000`, constant
differs only) — solve one, apply to both. 21 regfix substs at
`regfix.txt:3050-3071`; 0 asmfix; no prologue_config / frame_fix entries.

## Where it stands

| body | score | build_insns | stream vs target (51) |
|---|---|---|---|
| committed HEAD | 20 | 50 | frame wrong (vars=24 vs 16) |
| `candidate.c` (rounds 1-14) | 12 | 50 | park insn MISSING |
| `candidate_frame51.c` (round 15) | 13 | 51 | 2 wrong insns + renames |
| **`candidate_stream51.c` (round 16)** | 15 | **51** | **byte-stream exact except register NAMES** |

`candidate_stream51.c` is the first body whose 51 instructions match target
**one-for-one in opcode, operand shape and order** — no missing, extra or
reordered insn anywhere. The score is 15 (not lower) only because the masked-
Levenshtein metric counts each rename; structurally there is nothing left but
the register assignment. Verified identically on the twin (both 51/51, score 15).

Two levers got there from `candidate_frame51.c`:

1. **`s16` carrier, not `s32`.** Target's insn 1 `move a3,a0` copies the RAW
   incoming halfword *before* the sign-extension, and its join copy `move a3,v0`
   carries no extension. An `s32` carrier forces `sll/sra` at the join and drops
   the entry move — that was frame51's 2-insn structural error.
2. **A named tail temp** (`pkt = lo | 0xE3000000; return hi | pkt;`). Written as
   `return hi | (lo | C)` GCC reassociates to `(hi|C)|lo`; target computes
   `(lo|C)|hi`. The named temp pins the association.

## The residual — exact mechanism (round 16, ra_solver + global.c source)

The model reproduces our dispositions 11/11. Roles ours→target: carrier
`a2`→`a3`, tx `a0`→`v0`, limit-save-1 `v1`→`a2`, limit-save-2 `v1`→`a0`,
sxt(arg1) `a0`→`a2`, lo `a0`→`v0`, tail const `v0`→`a0` (`hi`=`v1`,
`arg1`=`a1`, sxt(arg0)=`a0` already correct).

`tmp/c7a0_what_if.py` finds an **exact 9/9 target allocation** from three model
deltas, and `global.c` says what each would require:

| delta | what it needs in C |
|---|---|
| `hi` conflicts with both limit-saves + both sign-extends | `hi` (the `$v1` value) live from *before* the x-clamp |
| tx and lo lose their `$a0` preference | the carrier's `$a0` pref must stop propagating to them |
| lo loses its `$v0` hard conflict | the `0xE3000000` local must not be local-allocated to `$v0` |

Mechanism detail worth keeping:
- `$a0` preference originates at `76 = a0` (the carrier init) and **propagates**
  to `tx` and `lo` through `global.c`'s REG_DEAD-linked copy merge
  (`IOR_HARD_REG_SET (hard_reg_preferences[a1], …[a2])`, global.c:851). It stops
  only if the two allocnos *conflict*. `find_reg` then takes the **lowest**
  preferred reg, so `$a0` beats the free `$v0`.
- MIPS has **no `REG_ALLOC_ORDER`** in this tree, so both allocators scan
  ascending — that is why every free value lands in `v0/v1/a0` and never `a2/a3`.
  `a2`/`a3` are reachable ONLY by exclusion (hard conflict or
  `regs_someone_prefers`), never by preference: a leaf function with 2 params has
  no hard `$a2`/`$a3`/`$v1` anywhere, so no pseudo can ever *prefer* them.
- The tail const is local-allocated; local-alloc gives it `$v0` because it is the
  first quantity born in the join block. Target has it in `$a0`, which needs
  `$v0` **and** `$v1` occupied at that point — not reproducible while `lo`/`hi`
  are global allocnos (local-alloc cannot see them).

## Do NOT re-run (measured negative / inert)

- **Permuter** — ~125k cumulative iters, 4 runs (rounds 1, 2, 12, 13). Every
  sub-baseline candidate is a forbidden family.
- **Clamp shape / arm ordering / decl order / types / precomputes** — rounds 1-15.
- **Round 16 (all inert at the 51-insn stream, no role moved):** tail-OR
  left-assoc, `pkt|hi` operand swap, `pkt` declared first, `u32 pkt`, `lo`
  computed before `hi` in the arms, `s16`/`s32` named limit locals (one or both
  clamps), a separate sign-extended compare variable, `tx = arg0` instead of
  `tx = x` (GCC CSEs it back to the carrier).
- **Round 16 structural regressions:** const folded into both arms (50 insns);
  separate join variable (50); `s32 tx` (49); early-declared const local (46
  linediff). A `s32 lim` named local *does* put the carrier in `a3` — but CSEs
  `lim-1` and loses the limit-save insn (50).
- **TU re-attribution / rodata reorder** — FAILED by cheat-reviewer 2026-06-05.

## Build gate

Applying any candidate breaks the oracle (the 21 regfix substs are calibrated to
HEAD's emission shape). Landing one requires rewriting/retiring those rules — a
completion-gate activity. src was reverted after every experiment.

## Resume here

Start from **`candidate_stream51.c`**. The only open question is the register
assignment, and round 16 narrowed it to one concrete blocker: **`$a2`/`$a3` are
unreachable by preference in a 2-param leaf function**, so target's allocation
requires an exclusion source we have not found a pure-C spelling for — most
likely `hi` being live across the clamps. Tooling for the next attempt:
`tmp/c7a0_apply.py` (body swap), `tmp/c7a0_batch.sh` + `tmp/c7a0_roles.py`
(~12 s per variant, prints the role→register map), `tmp/c7a0_model.sh`
(extract + simulate), `tmp/c7a0_what_if.py` (model deltas → allocation).

Sibling: `memory/wip/func_8007C86C/` — identical pattern and floor.
