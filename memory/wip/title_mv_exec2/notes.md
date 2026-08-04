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

**This was never a phantom-frame problem.** Both bodies have `vars=64` (the real
`s32 buf[16]`); we were emitting **5 callee-saves against target's 3**, and 5
regs round up to 24 bytes vs 3 regs' 16. The two extra saves were loop-invariant
constants that `loop.c` hoisted: `li s3,24` and `li s2,1`. Target rematerialises
both inside the loop. Routing both through ONE scratch local makes the pseudo
multi-set, so it is not a `move_movables` candidate:

```c
s32 t;
    t = 0x18;   *(s16 *)((u8 *)&D_800F4E1A + offset) = t;   /* + 5 more stores */
    t = 1;      buf[0] = t << (s16)var_s0;
```

Semantics verified: GCC rotates the `t = 0x18` reload into the loop-back
branch's delay slot, so every iteration stores 24 — loop rotation, not a stale
value. Same sanctioned shape already shipping in `func_8003DBE4`.

## Residual — and why the RA solver cannot see it (2026-08-04)

Ours puts `t` in `$a1` with the sign-extended `var_s0` in `$v1`; target has `t`
in `$v1` and the sign-extend in `$a1`. Target also emits `li v1,24` inline at its
use and `addiu a0,sp,16` early; ours emits `li a1,24` at the loop top with the
reload in the loop-back delay slot, and `addiu a0,sp,16` late.

Model extracted and **validated 3/3 against the dump** (free regression check).
The function has only **three global allocnos**: 75 = `t` (nrefs 8, livelen 17,
→`$a1`), 72 = `var_s0` (→`$s0`), 74 = `ff` (→`$s1`). Everything else is
local-alloc. `hard_conflicts[75] = {2,3,4}` — `$v0`, `$v1` and `$a0` are all
blocked for `t`, which is why it lands in `$a1`.

**`perturb.py` with spec `{75: 3}`, singles + pairs + greedy: NO sufficient
vector.** That is the correct answer, not a search failure: the `$v1` block on
`t` comes from **pseudo 83 (the sign-extend), a LOCAL-alloc quantity sitting in
`$v1`** — and `ra_solver` models `global.c` only. Local-alloc runs first and is
outside the simulated space, so no global-allocno atom can move `t`.

To reach target, local-alloc must put the sign-extend somewhere other than `$v1`.
It scans ascending, so that needs `$v0`, `$v1` and `$a0` all occupied across the
sign-extend's range. `$v0` (offset arithmetic) and `$a0` (the call argument) are;
`$v1` is not. In target `$v1` holds the `24` — i.e. **target's `24` constant is a
local quantity allocated before the sign-extend, while ours is a global allocno.**
RTL check (`tmp/ra/rtl.py`): all four references to pseudo 75 are inside the loop
body block, so the promotion to global is a local-alloc decision, not a
cross-block one.

## Measured negative / inert (do not re-run)

- `t` for the `0x18` only, or the `1` only — single-set, still hoisted: 27.
- Reusing `offset` for the `1` (`off_1`, `off_both`): frame 96, regs=4, 74 insns,
  score 20. Routing the `1` through `offset` after the stores: **74 insns**.
- `s16 t`: frame 96, regs=4, 74 insns, score 16 — better on the metric but
  structurally worse. Not the base to resume from.
- Named sign-extend local (`s32 idx` / `s16 idx`) as the shift amount: 22, inert,
  and the register mapping is bit-identical to the candidate's.
- `t` declared at loop-body block scope: bit-identical to the candidate.
- `t` placement sweeps (loop top, at each use, `0x18` store reordered, shift
  before the stores, idx+shift-first): all 22. Shift-expression *first* changes
  the shape (42 differing lines vs 40) without fixing the swap.
- `t = (s16)var_s0; buf[0] = 1 << t;`: **74 insns**, breaks the stream.
- Call-arg naming ([[hoist-call-arg-local-flips-jal-delay]]): `s32 *p = buf;` in
  a block wrapping the loop body (22), at loop-body top (22), `u8 *p` (22), and
  the rule's exact shape (22). **At FUNCTION scope it regresses to 35** — the
  pointer takes a callee-save. Check the save count, not just the score.
- Statement order around the call: all 22.
- `store-before-jal` does not apply: the pre-call `sw …,16(sp)` is already in the
  jal delay slot in both builds.
- **cc1psx** ([[cc1psx-calibration-only]]) makes the same choice we do — `li $5,24`
  with the sign-extend in `$3`, frame 96, regs=3. Not closer to target; the
  compiler-divergence hypothesis is dead.

## Build gate

Applying the candidate breaks the oracle (SHA1 `31fe8c21`) because the 18 regfix
rules are calibrated to HEAD's shape. `src/main.c` reverted, full build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Resume here — BANKED at 22, do not force

Frame and save-count are closed and the instruction count matches. What is left
is one register swap whose decision is made by **local-alloc, which the solver
does not model** — so neither another placement sweep nor the perturb search can
reach it. A future attempt needs a lever that changes which quantity local-alloc
allocates first in the loop body, or an extension of `ra_solver` to
`local-alloc.c`. Start from `candidate.c`.
