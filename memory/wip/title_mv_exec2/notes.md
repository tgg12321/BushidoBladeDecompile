# title_mv_exec2 — WIP (current state 2026-08-04, ra_solver round)
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

The candidate matches target's frame, callee-save count and instruction count
exactly. The residual 22 is register NAMING plus one placement.

## The lever — [[defeat-licm-hoist-var-reuse]]

**Never a phantom-frame problem.** Both bodies have `vars=64` (the real
`s32 buf[16]`); we emitted **5 callee-saves against target's 3** (5 regs round to
24 bytes vs 3 regs' 16). The two extra saves were loop-invariant constants
`loop.c` hoisted: `li s3,24` and `li s2,1`. Target rematerialises both inside the
loop. Routing both through ONE scratch local makes the pseudo multi-set, so it is
not a `move_movables` candidate:

```c
s32 t;
    t = 0x18;   *(s16 *)((u8 *)&D_800F4E1A + offset) = t;   /* + 5 more stores */
    t = 1;      buf[0] = t << (s16)var_s0;
```

Semantics verified: GCC rotates the `t = 0x18` reload into the loop-back branch's
delay slot, so every iteration stores 24 — loop rotation, not a stale value. Same
sanctioned shape already shipping in `func_8003DBE4`.

## Residual — the swap, and why global-alloc cannot see it (2026-08-04)

Ours puts `t` in `$a1` with the sign-extended `var_s0` in `$v1`; target has `t`
in `$v1` and the sign-extend in `$a1`. Target also emits `li v1,24` inline at its
use and `addiu a0,sp,16` early; ours emits `li a1,24` at the loop top and
`addiu a0,sp,16` late.

Global model validated **3/3** against the dump. Only **three global allocnos**:
75 = `t` (nrefs 8, livelen 17, →`$a1`), 72 = `var_s0` (→`$s0`), 74 = `ff`
(→`$s1`); everything else is local-alloc. `hard_conflicts[75] = {2,3,4}` blocks
`$v0`/`$v1`/`$a0` for `t`. **`perturb.py` spec `{75: 3}`, singles + pairs +
greedy: NO vector** — correct, not a search failure: the `$v1` block on `t` is
pseudo 83, the sign-extend, a **local-alloc** quantity.

RTL check (`tmp/ra/rtl.py`): all four refs to pseudo 75 are inside the loop
body block, so the promotion to global is a local-alloc decision. (Phase 5 below
supersedes this section's guess about which registers are occupied.)

## Measured negative / inert (do not re-run)

- `t` for the `0x18` only, or the `1` only — single-set, still hoisted: 27.
- Reusing `offset` for the `1` (`off_1`, `off_both`): frame 96, regs=4, 74 insns,
  score 20. Routing the `1` through `offset` after the stores: **74 insns**.
- `s16 t`: frame 96, regs=4, 74 insns, score 16 — better on the metric but
  structurally worse. Not the base to resume from.
- Named sign-extend local (`s32 idx` / `s16 idx`) as the shift amount: 22, inert,
  and the register mapping is bit-identical to the candidate's.
- `t` declared at loop-body block scope: bit-identical to the candidate.
- `t` placement sweeps (loop top, at each use, store reordered, shift-first,
  idx+shift-first): all 22; shift-first is worse (42 differing lines vs 40).
  `t = (s16)var_s0; buf[0] = 1 << t;`: **74 insns**, breaks the stream.
- Call-arg naming ([[hoist-call-arg-local-flips-jal-delay]]): `s32 *p = buf;` in
  a block wrapping the loop body, at loop-body top, `u8 *p`, and the rule's exact
  shape — all 22. **At FUNCTION scope it regresses to 35** (the pointer takes a
  callee-save). Statement order around the call: all 22.
- `store-before-jal` does not apply: the pre-call `sw …,16(sp)` is already in
  the jal delay slot in both builds.
- **cc1psx** ([[cc1psx-calibration-only]]) makes the same choice we do (`li $5,24`,
  sign-extend in `$3`, frame 96, regs=3) — compiler divergence is dead.

**Build gate:** applying the candidate breaks the oracle (SHA1 `31fe8c21`) — the
18 regfix rules are calibrated to HEAD's shape. `src/main.c` reverted, build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## The swap now has a QUANTIFIED spec (ra_solver Phase 5, 2026-08-04)

`ra_solver` now models `local-alloc.c` (`local_extract.py` + `local_alloc.py`),
and it reproduces this function **exactly — 2/2 blocks on order, 10/10 qtys on
assignment.** Block 2 (the loop body), in allocation order:

| qty | refs | span | pri | got |
|---|---|---|---|---|
| 2 | 18 | [48,60) | 60000 | `$v0` |
| 1 | 30 | [10,34) | 50000 | `$v0` |
| **0** | 18 | **[6,38)** | **22500** | **`$v1`** ← the sign-extend |
| 3 | 4 | [54,58) | 20000 | `$v1` |

`find_free_reg` scans ascending, so the sign-extend takes `$v1` because only
`$v0` is occupied across its range. Target holds it in `$a1`, which needs `$v0`,
`$v1` **and** `$a0` all occupied there. Replaying the block (`tmp/ra/tmv_local.py`)
gives a **two-part** requirement — neither part alone is enough:

| perturbation | sign-extend lands in |
|---|---|
| one extra qty overlapping [6,38), pri > 22500 (e.g. refs 8, span ≤10) | `$a0` |
| hard `$a0` live across [6,38) alone | `$v1` (unchanged) |
| **both together** | **`$a1` — TARGET** |

That explains why the call-argument family (`s32 *p = buf;` and friends) was
inert at 22: it only ever addressed part 2. Target supplies part 2 by emitting
`addiu a0,sp,16` **early**, before the address arithmetic, so hard `$a0` is live
across the sign-extend's range; we emit it late.

## Resume here — BANKED at 22, do not force

Frame and save-count are closed and the instruction count matches. The residual
register swap is a local-alloc decision, and it now has the concrete two-part
spec above rather than "local-alloc decides it". A future attempt must supply
**both** parts at once; part 1 needs a short-lived, high-reference local quantity
inside the loop body (note `t` has 8 references but is currently a *global*
allocno spanning the whole body, so it does not qualify as-is). Start from
`candidate.c`.
