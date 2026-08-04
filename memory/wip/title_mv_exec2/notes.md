# title_mv_exec2 — WIP (current state 2026-08-04)
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
exactly. The residual 22 is register NAMING plus one placement (below).

## The lever — [[defeat-licm-hoist-var-reuse]]

**This was never a phantom-frame problem.** The census listed it as
`ours-larger delta -8`, but the 8 bytes are not vars: both bodies have
`vars=64` (the real `s32 buf[16]`). We were emitting **5 callee-saves against
target's 3**, and 5 regs round up to 24 bytes vs 3 regs' 16.

The two extra saves were loop-invariant constants that `loop.c` hoisted:
`li s3,24` (the `0x18` field value) and `li s2,1` (the `1 << i` base). Target
rematerialises both inside the loop. Routing both through ONE scratch local
makes the pseudo multi-set, so it is not a `move_movables` candidate:

```c
s32 t;
...
    t = 0x18;
    *(s16 *)((u8 *)&D_800F4E1A + offset) = t;
    ... four more stores ...
    t = 1;
    buf[0] = t << (s16)var_s0;
```

Both constants then stay in caller-saved registers; the frame, save count and
instruction count all fall to target's. This is the same sanctioned rule (and
the same shape) already shipping in `func_8003DBE4` in this tree.

Semantics verified: GCC rotates the `t = 0x18` reload into the loop-back
branch's delay slot (`bne $2,$0,.L202` / `li $5,24`), so every iteration stores
24 — the hoist visible before the loop label is loop rotation, not a stale
value.

## Residual (22)

- Register swap: ours puts `t` in `$a1` and the sign-extended `var_s0` in
  `$v1`; target has `t` in `$v1` and the sign-extend in `$a1`.
- Placement: target emits `li v1,24` inline at its use and `addiu a0,sp,16`
  early; ours emits `li a1,24` at the loop top with the reload in the loop-back
  delay slot, and `addiu a0,sp,16` late.
## cc1psx diagnostic (sanctioned, [[cc1psx-calibration-only]])

Ran both compilers on the candidate source. **cc1psx makes the same choice we
do** — `li $5,24` (a1) with the sign-extend in `$3` (v1), frame 96, regs=3 —
i.e. it is NOT closer to target here. The earlier differential sweep reading
(`ours=31 / psx=15`) was taken on the OLD source, whose hoisting bug our fork
happened to handle worse; once the hoist is fixed both compilers converge on
the identical shape. This is the expected "cc1psx is never closer" result and
removes the compiler-divergence hypothesis for this function.

## Measured negative / inert (do not re-run)

- `t` for the `0x18` only, or for the `1` only — each stays single-set, still
  hoisted: 27 (no change from HEAD).
- Reusing the existing `offset` variable for the `1` (`off_1`, `off_both`):
  frame 96 but regs=4, insns=74, score 20.
- `s16 t` instead of `s32 t`: frame 96, regs=4, insns=74, score 16 — better on
  the metric but structurally worse (one extra save, two extra insns). Not the
  base to resume from.
- Named sign-extend local (`s32 idx` and `s16 idx`) reused as the shift amount:
  22, inert.
- `t` assignment placement — at loop top, immediately before each use, `0x18`
  store moved after the other five stores, shift computed before the stores,
  and the idx+shift-first combination: all 22, inert.
## Build gate

Applying the candidate breaks the oracle (SHA1 `31fe8c21`) because the 18
regfix rules are calibrated to HEAD's emission shape — the same gate the
display twins hit. `src/main.c` was reverted and the full build re-verified at
`62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Round 2 (same day) — both remaining avenues executed, both inert

Per the no-unrun-avenues discipline, the two levers this note previously listed
as untried were run. **Neither moves the `$a1`/`$v1` swap or the `li 24`
placement**; all hold at exactly 22 with frame 96 / 3 saves / 72 insns.

- **Naming the buffer address in a local** (the
  [[hoist-call-arg-local-flips-jal-delay]] LUID bias): `s32 *p = buf;` declared
  first inside a block wrapping the whole loop body (22); declared at loop-body
  top (22); `u8 *p` form with the store cast (22); and the rule's exact shape —
  a block wrapping only the last pre-call store plus the call, arg declared
  first (22). **`s32 *p = buf;` at FUNCTION scope regresses to 35** (frame 96
  but regs=4, 75 insns) — the pointer becomes live across the loop and takes a
  callee-save. Same trap as the `s16 t` variant: check the save count, not just
  the score.
- **Statement order around the call**: shift-expression before the six field
  stores (22); shift first, then offset, then fields (22); offset moved after
  the shift (22); pointer-local plus shift-first (22).
`store-before-jal` does not apply: the pre-call `sw …,16(sp)` is *already* in
the jal delay slot in both builds — the difference there is only which register
holds the value (`v0` target / `v1` ours), i.e. the same rename.

## Resume here — BANKED at 22, do not force

The frame and save-count questions are closed and the instruction stream
matches target exactly. What is left is one consistent register swap plus the
`li 24` placement, and every lever reachable from the technique index is now
measured: the `t`-placement family, the `idx`-local family, the call-argument
LUID family and statement order around the call. Any future attempt needs a
genuinely new idea, not another placement sweep. Start from `candidate.c`.
