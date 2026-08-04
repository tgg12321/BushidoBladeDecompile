# title_mv_exec2 — WIP (current state 2026-08-04, ra_solver Phase 5)
`src/main.c:879`. Builds a 64-byte GPU/primitive buffer on the stack, then for
each of `D_80101BCC` entries writes six s16 fields at `entry * 54` byte stride
into the `D_800F4E1x` block, sets `buf[0] = 1 << i`, and calls
`saTan1MainJump(buf)` + `motutil_GetAngTableNum(1)`.

18 regfix rules; 0 asmfix; no prologue_config / frame_fix / maspsx-gate entries.

## Where it stands

| body | score | frame | saves | build_insns |
|---|---|---|---|---|
| committed HEAD | 27 | 104 | 5 | 76 |
| **`candidate.c`** | **22** | **96** | **3** | **72** |
| target | 0 | 96 | 3 | 72 |

Frame, callee-save count and instruction count all match target. The residual 22
is register naming plus one placement.

## The lever — [[defeat-licm-hoist-var-reuse]]

**Never a phantom-frame problem.** Both have `vars=64`; we emitted **5
callee-saves against target's 3**. The extra saves were loop-invariant constants
`loop.c` hoisted (`li s3,24`, `li s2,1`); target rematerialises both in the loop.
Routing both through ONE scratch local makes the pseudo multi-set, so it is not a
`move_movables` candidate:

```c
s32 t;
    t = 0x18;   *(s16 *)((u8 *)&D_800F4E1A + offset) = t;   /* + 5 more stores */
    t = 1;      buf[0] = t << (s16)var_s0;
```

GCC rotates the `t = 0x18` reload into the loop-back branch's delay slot, so
every iteration stores 24 — loop rotation, not a stale value. Same shape already
shipping in `func_8003DBE4`.

## The swap, and its quantified spec (ra_solver Phase 5)

Ours puts `t` in `$a1` with the sign-extended `var_s0` in `$v1`; target has them
swapped, and emits `addiu a0,sp,16` early where we emit it late. Global model
validated **3/3**; only three global allocnos (75 = `t`, 72 = `var_s0`, 74 =
`ff`), and **`perturb.py` spec `{75: 3}` finds NO vector** — correct, not a
search failure: the `$v1` block on `t` is the sign-extend, a **local-alloc**
quantity. The local-alloc model (`local_extract.py` + `local_alloc.py`) reproduces this
function **exactly — 2/2 blocks on order, 10/10 qtys on assignment.** Block 2 in
allocation order:

| qty | refs | span | pri | got |
|---|---|---|---|---|
| 2 | 18 | [48,60) | 60000 | `$v0` |
| 1 | 30 | [10,34) | 50000 | `$v0` |
| **0** | 18 | **[6,38)** | **22500** | **`$v1`** ← the sign-extend |
| 3 | 4 | [54,58) | 20000 | `$v1` |

`find_free_reg` scans ascending, so the sign-extend takes `$v1` because only
`$v0` is occupied across its range. Target holds it in `$a1`, needing `$v0`,
`$v1` **and** `$a0` occupied there. Replaying the block (`tmp/ra/tmv_local.py`)
gives a **two-part** requirement — neither part alone is enough:

| perturbation | sign-extend lands in |
|---|---|
| one extra qty overlapping [6,38), pri > 22500 (e.g. refs 8, span ≤10) | `$a0` |
| hard `$a0` live across [6,38) alone | `$v1` (unchanged) |
| **both together** | **`$a1` — TARGET** |

That is why the call-argument family was inert at 22: it only addressed part 2.
Census of target shows how it supplies both — **part 2**: `addiu a0,sp,16` at
line 23, *before* the address arithmetic, so hard `$a0` (the
`saTan1MainJump(buf)` argument) is live across the sign-extend. **part 1**:
`li v1,24` at 31 consumed by `sh v1,0(at)` at 34 — the `24` as a **short-lived
local quantity in `$v1`**, three instructions long.

## The real open question

Target keeps the two constants **separate and short-lived**: `24` in `$v1`
(31→34), the `1` in `$v0` (`li v0,1` at 50, `sllv v0,v0,a1` at 51). Neither is
hoisted. Our only anti-hoist lever routes both through ONE multi-set `t`, which
necessarily makes a single long-lived pseudo — exactly what destroys part 1. So
the question is no longer which register the sign-extend gets, but:

> **why does `loop.c`'s `move_movables` not hoist target's single-set loop
> constants, when it hoists ours?**

Answer that and the `24` becomes a short-lived local qty in `$v1` (part 1), the
shared `t` is unnecessary, and part 2 is a placement question the call-argument
family already half-explored.

## Measured negative / inert (do not re-run)

- `t` for the `0x18` only, or the `1` only — single-set, still hoisted: 27.
- Reusing `offset` for the `1`: frame 96, regs=4, 74 insns, score 20. Routing the
  `1` through `offset` after the stores: **74 insns**.
- `s16 t`: frame 96, regs=4, 74 insns, score 16 — better on the metric but
  structurally worse (extra save, extra insns). Not a base to resume from.
- Named sign-extend local (`s32`/`s16 idx`): 22, register map bit-identical.
- `t` at loop-body block scope: bit-identical to the candidate.
- `t` placement sweeps (loop top, at each use, store reordered, shift-first,
  idx+shift-first): all 22; shift-first is worse (42 differing lines vs 40).
  `t = (s16)var_s0; buf[0] = 1 << t;`: **74 insns**, breaks the stream.
- Shift-first at the qty level: adds a 5th qty (refs 4, span [10,16)) but it
  allocates last and lands in `$v0` — does **not** supply part 1.
- Call-arg naming ([[hoist-call-arg-local-flips-jal-delay]]): `s32 *p = buf;`
  block-wrapped, at loop-body top, `u8 *p`, the rule's exact shape — all 22.
  **At FUNCTION scope it regresses to 35.** Statement order around the call: 22.
- `store-before-jal` does not apply: the pre-call `sw …,16(sp)` is already in the
  jal delay slot in both builds.
- **cc1psx** ([[cc1psx-calibration-only]]) makes the same choice we do — compiler
  divergence is dead.

**Build gate:** applying the candidate breaks the oracle (SHA1 `31fe8c21`) — the
18 regfix rules are calibrated to HEAD's shape. `src/main.c` reverted, build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Resume here — BANKED at 22, do not force

Start from `candidate.c`. Work the `move_movables` question above, not another
placement sweep: both parts must land together, and part 1 is unreachable while
`t` is shared.
