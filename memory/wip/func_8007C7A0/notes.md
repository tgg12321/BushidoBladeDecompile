# func_8007C7A0 — WIP (current state 2026-08-04, round 17)

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
| **`candidate_stream51.c` (round 16)** | 15 | **51** | **exact except register NAMES** |

`candidate_stream51.c` is the first body whose 51 instructions match target
**one-for-one in opcode, operand shape and order** — no missing, extra or
reordered insn. The score is 15 only because masked Levenshtein counts renames;
structurally nothing is left but the register assignment. Verified identically on
the twin (both 51/51, score 15). Two levers got there from `candidate_frame51.c`:

1. **`s16` carrier, not `s32`.** Target's insn 1 `move a3,a0` copies the RAW
   halfword *before* sign-extension and the join copy `move a3,v0` carries no
   extension. An `s32` carrier forces `sll/sra` at the join and drops the entry
   move — frame51's 2-insn error.
2. **A named tail temp** (`pkt = lo | 0xE3000000; return hi | pkt;`). Written as
   `return hi | (lo | C)` GCC reassociates to `(hi|C)|lo`; target computes
   `(lo|C)|hi`. The named temp pins the association.

Roles ours→target: carrier `a2`→`a3`, tx `a0`→`v0`, limit-save-1 `v1`→`a2`,
limit-save-2 `v1`→`a0`, sxt(arg1) `a0`→`a2`, lo `a0`→`v0`, const `v0`→`a0`
(`hi`=`v1`, `arg1`=`a1`, sxt(arg0)=`a0` already correct).

## Round 17 — the pure-C route to that allocation is CLOSED

`tmp/c7a0_a2_scan.py` enumerates all **273 single-atom perturbations** of the RA
model (refs, live length, preferences, conflict add/remove, hard-conflict
removal, all 9 allocnos) and asks which put the x-clamp limit-save in `$a2` — the
atom that gates the cascade (`83→a2` forces `94→a0`, `92→a2`, `81→a0`, `76→a3`).
**Exactly three work, and each needs a hard register this function lacks:**

| atom | requires | why unspellable |
|---|---|---|
| `pref 76→r3` | carrier copy-related to hard `$v1` | no hard `$v1`; also lands `hi` in `$a2` (wrong) |
| `conf +78~83` | `hi` live across the x-clamp | needs an early def — disproven below |
| `pref 83→r6` | limit-save copy-related to hard `$a2` | `$a2` is arg 3; this function takes two |

A 2-param leaf function has exactly three hard regs in scope (`$a0`, `$a1`,
`$v0`), and `set_preference` fires only on a hard reg or a local-alloc pseudo
already renumbered to one. So `$a2`/`$a3`/`$v1` can never be *preferred*, only
reached by exclusion — whose sole source is a conflict with the `$v1` holder.

**The `hi`-live-early route is disproven from the bytes, not from search.**
Liveness starts at a def, so `hi` living across the x-clamp needs an instruction
writing it there. `tmp/c7a0_v1_census.py` over target's own stream: the first
`$v1` def is insn 39 (`andi v1,a1,0xfff`); registers written before it are
`a0, a1, a2, a3, sp, v0` — **no `$v1`**. The limit-save's range is insn 9→13. So
in the *original's own compilation* `hi` and the limit-save cannot conflict, and
round 16's model 9/9 solution is **not** the mechanism the original used.

Consequence: within the pre-reload allocator the simulator models, target's
allocation is unreachable from ANY stream-exact two-parameter leaf body. The one
unmodeled path is reload's spill-retry (`ra_solver` README's `saTan4FireDisp`
divergence) — but target's frame is `addiu sp,sp,-16`, vars=16, no spill slots.

Mechanism detail worth keeping:
- The `$a0` preference originates at `76 = a0` (carrier init) and **propagates**
  to `tx` and `lo` via `global.c:851`'s REG_DEAD-linked copy merge; it stops only
  if the allocnos *conflict*. `find_reg` takes the **lowest** preferred reg, so
  `$a0` beats a free `$v0`.
- MIPS has **no `REG_ALLOC_ORDER`** in this tree — both allocators scan ascending,
  which is why free values land in `v0/v1/a0` and never `a2/a3`.
- The tail const is local-allocated to `$v0` as the first quantity born in the
  join block. Target's `$a0` needs `$v0` *and* `$v1` occupied there, which
  local-alloc cannot arrange while `lo`/`hi` are global allocnos.

## Do NOT re-run (measured negative / inert)

- **Permuter** — ~125k cumulative iters, 4 runs (rounds 1, 2, 12, 13); every
  sub-baseline candidate is a forbidden family.
- **Clamp shape / arm ordering / decl order / types / precomputes** — rounds 1-15.
- **Round 16, inert at the 51-insn stream (no role moved):** tail-OR left-assoc,
  `pkt|hi` operand swap, `pkt` declared first, `u32 pkt`, `lo` before `hi` in the
  arms, `s16`/`s32` named limit locals, a separate sign-extended compare
  variable, `tx = arg0` (GCC CSEs it back to the carrier).
- **Round 16 structural regressions:** const folded into both arms (50 insns),
  separate join variable (50), `s32 tx` (49), early-declared const local. An
  `s32 lim` named local *does* put the carrier in `a3` but CSEs `lim-1` and loses
  the limit-save insn (50).
- **Round 17 variable-reuse family** ([[defeat-licm-hoist-var-reuse]] /
  [[staged-value-reused-variable]]) — all five spellings BREAK the stream:
  `hi` = x-limit save (50), = y-limit save (49), = both limit saves (46),
  = sign-extended y (50), `lo` = x-limit save (50). Naming a limit lets GCC CSE
  `limit - 1` and the limit-save insn disappears. The family also cannot reach
  the goal in principle (above).
- **TU re-attribution / rodata reorder** — FAILED by cheat-reviewer 2026-06-05.

## Build gate

Applying any candidate breaks the oracle (the 21 regfix substs are calibrated to
HEAD's emission shape). Landing one requires rewriting/retiring those rules — a
completion-gate activity. src was reverted after every experiment.

## Resume here

Start from **`candidate_stream51.c`**. The stream is done; only the register
assignment is open, and round 17 closed the pure-C search space for it. **Do not
spend more rounds on C spellings for the allocation** — the next legitimate move
is a mechanism outside the modeled allocator path, or an owner disposition
decision. Tooling: `tmp/c7a0_apply.py` (body swap), `tmp/c7a0_batch.sh` +
`tmp/c7a0_roles.py` (~12 s/variant, role→register map), `tmp/c7a0_model.sh`
(extract + simulate), `tmp/c7a0_what_if.py` (model deltas → allocation),
`tmp/c7a0_a2_scan.py` (exhaustive atom scan), `tmp/c7a0_v1_census.py`.

Sibling: `memory/wip/func_8007C86C/` — identical pattern and floor.
