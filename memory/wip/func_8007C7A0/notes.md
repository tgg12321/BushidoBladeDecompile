# func_8007C7A0 — WIP (current state 2026-08-04, round 18)

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
**one-for-one in opcode, operand shape and order**. The score is 15 only because
masked Levenshtein counts renames; nothing is left but the register assignment.
Verified identically on the twin (both 51/51, score 15). Two levers got there:
**(1) `s16` carrier, not `s32`** — target's insn 1 `move a3,a0` copies the RAW
halfword *before* sign-extension and the join copy `move a3,v0` carries no
extension, so an `s32` carrier forces `sll/sra` at the join and drops the entry
move (frame51's 2-insn error). **(2) a named tail temp**
(`pkt = lo | 0xE3000000; return hi | pkt;`) — written as `return hi | (lo | C)`
GCC reassociates to `(hi|C)|lo` while target computes `(lo|C)|hi`.

Roles ours→target: carrier `a2`→`a3`, tx `a0`→`v0`, limit-save-1 `v1`→`a2`,
limit-save-2 `v1`→`a0`, sxt(arg1) `a0`→`a2`, lo `a0`→`v0`, const `v0`→`a0`
(`hi`=`v1`, `arg1`=`a1`, sxt(arg0)=`a0` already correct).

## The allocation is not reachable in pure C (rounds 17-18)

`tmp/c7a0_a2_scan.py` enumerates all **273 single-atom perturbations** of the RA
model and asks which put the x-clamp limit-save in `$a2` — the atom that gates
the cascade (`83→a2` forces `94→a0`, `92→a2`, `81→a0`, `76→a3`). **Exactly three
work, each needing a hard register this function lacks:**

| atom | requires | why unspellable |
|---|---|---|
| `pref 76→r3` | carrier copy-related to hard `$v1` | no hard `$v1`; also lands `hi` in `$a2` |
| `conf +78~83` | `hi` live across the x-clamp | needs an early def — disproven below |
| `pref 83→r6` | limit-save copy-related to hard `$a2` | `$a2` is arg 3; this takes two |

A 2-param leaf has exactly three hard regs in scope (`$a0`, `$a1`, `$v0`), and
`set_preference` fires only on a hard reg or a local-alloc pseudo renumbered to
one. So `$a2`/`$a3`/`$v1` can never be *preferred*, only reached by exclusion —
whose sole source is a conflict with the `$v1` holder.

**Route 2 is disproven from the bytes.** Liveness starts at a def, so `hi` living
across the x-clamp needs an instruction writing it there. `tmp/c7a0_v1_census.py`
over target's stream: first `$v1` def is insn 39; registers written before it are
`a0, a1, a2, a3, sp, v0` — **no `$v1`**. The limit-save's range is insn 9→13, so
`hi` and the limit-save cannot conflict in the original's own compilation either;
round 16's model 9/9 solution is **not** the mechanism it used.

**Round 18 — widening the signature does not help.** No prototype exists (the
definitions at `src/display.c:563`/`:604` follow the call sites), so it can be
widened alone. 3- and 4-param forms with the extras unread, `s16` and `s32`:
**all four bit-identical to the 2-param baseline**, and `tmp/c7a0_iso_check.py`
proves it at model level — holding the param pseudos fixed and shifting body
pseudos `+2`, the 4-param model is **ISOMORPHIC in every field**. An unread
param's prologue copy is dead and deleted before `global_conflicts`, so hard
`$a2`/`$a3` never go live; a *read* extra param would emit an instruction and
break the stream.

Consequence: target's allocation is unreachable from ANY stream-exact leaf body.
**The reload escape is now MEASURED dead, not merely inferred** (Phase 5,
`tmp/ra/retry_survey.sh` via the `BB2_FINDREG_DEBUG` `retry=` field, candidate
applied): all nine allocnos show `calls=1` with **zero** `retry=1` blocks —
`find_reg` is entered exactly once each, never re-entered. That matches the
source: the losers/retry path needs `best_reg < 0`, i.e. all 32 registers
exhausted, which nine allocnos cannot do. **And the compiler-identity premise is
tested and survives**: `tools/cc1psx_wrapper.sh` (the ORIGINAL PsyQ cc1psx) on
this exact body emits output **byte-identical to the fork**, 0/45 differing
insns after label normalisation. With global alloc searched exhaustively, local
alloc validated, retry never firing and the original compiler agreeing, **no
mechanism remains**. Mechanism of the residual: the `$a0` preference
originates at `76 = a0` (carrier init) and **propagates** to `tx` and `lo` via
`global.c:851`'s REG_DEAD-linked copy merge, stopping only if the allocnos
*conflict*; `find_reg` takes the **lowest** preferred reg, and MIPS has **no
`REG_ALLOC_ORDER`** here so both allocators scan ascending.

## Do NOT re-run (measured negative / inert)

- **Permuter** — ~125k iters, 4 runs (rounds 1, 2, 12, 13); all sub-baseline
  candidates are forbidden families.
- **Clamp shape / arm ordering / decl order / types / precomputes** — rounds 1-15.
- **Round 16 inert:** tail-OR left-assoc, `pkt|hi` swap, `pkt` first, `u32 pkt`,
  `lo` before `hi`, named limit locals, a separate sign-extended compare
  variable, `tx = arg0` (CSEs back to the carrier).
- **Round 16 regressions:** const in both arms (50 insns), separate join variable
  (50), `s32 tx` (49), early const local. An `s32 lim` named local *does* put the
  carrier in `a3` but CSEs `lim-1` and loses the limit-save insn (50).
- **Round 17 variable-reuse family** ([[defeat-licm-hoist-var-reuse]] /
  [[staged-value-reused-variable]]) — all five BREAK the stream: `hi` = x-limit
  save (50), = y-limit (49), = both limits (46), = sxt y (50), `lo` = x-limit
  (50). Naming a limit lets GCC CSE `limit - 1`.
- **Round 18 inert (bit-identical to baseline):** 3-/4-param signatures (`s16`
  and `s32`); join-block temps `pkt = lo; pkt |= C`, `word = hi | pkt`, named
  `c = 0xE3000000`, `pkt = C; pkt |= lo` — GCC coalesces every added copy.
- **TU re-attribution / rodata reorder** — FAILED by cheat-reviewer 2026-06-05.

**Build gate:** applying any candidate breaks the oracle (the 21 regfix substs
are calibrated to HEAD's shape); landing one means rewriting/retiring those
rules. src was reverted after every experiment.

## Resume here

Start from **`candidate_stream51.c`**. The stream is done; only the register
assignment is open, and rounds 17-18 closed the pure-C search space for it,
including the premise-attacking escapes (wider signature, join-block temps).
**Do not spend more rounds on C spellings for the allocation** — the next
legitimate move is a mechanism outside the modeled allocator path, or an owner
disposition decision. Tooling: `tmp/c7a0_apply.py`, `c7a0_batch.sh` +
`c7a0_roles.py` (~12 s/variant), `c7a0_model.sh`, `c7a0_what_if.py`,
`c7a0_a2_scan.py` (exhaustive atom scan), `c7a0_v1_census.py`,
`c7a0_iso_check.py`, `tmp/ra/retry_survey.sh`. Sibling:
`memory/wip/func_8007C86C/` — identical pattern and floor.
