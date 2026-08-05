# title_mv_exec2 — WIP (current state 2026-08-05, sched_solver goal-mapper)
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

Frame, callee-save count and instruction count all match target; the residual 22
is register naming plus one placement.

## The lever — [[defeat-licm-hoist-var-reuse]]

**Never a phantom-frame problem.** Both have `vars=64`; we emitted **5
callee-saves against target's 3**. The extra saves were loop-invariant constants
`loop.c` hoisted (`li s3,24`, `li s2,1`); target rematerialises both in the loop.
Routing both through ONE scratch local (`s32 t; t = 0x18;` before the six stores,
`t = 1;` before `buf[0] = t << (s16)var_s0;`) makes the pseudo multi-set, so it
is not a `move_movables` candidate. GCC rotates the `t = 0x18` reload into the
loop-back branch's delay slot, so every iteration stores 24 — loop rotation, not
a stale value. Same shape already shipping in `func_8003DBE4`.

## The swap, and its quantified spec (ra_solver Phase 5)

Ours puts `t` in `$a1` with the sign-extended `var_s0` in `$v1`; target has them
swapped, and emits `addiu a0,sp,16` early where we emit it late. Global model
3/3; only three global allocnos (75 = `t`, 72 = `var_s0`, 74 = `ff`), and
`ra_solver/perturb.py` spec `{75: 3}` finds NO vector — correct, not a search
failure: the `$v1` block on `t` is the sign-extend, a **local-alloc** quantity.
The local-alloc model reproduces this function **exactly (2/2 order, 10/10
assign)**; block 2 allocates qty2 (pri 60000)→`$v0`, qty1 (50000)→`$v0`,
**qty0 = the sign-extend, span [6,38), pri 22500 →`$v1`**, qty3 (20000)→`$v1`.
`find_free_reg` scans ascending, so the sign-extend takes `$v1` because only
`$v0` is occupied across its range.

Target holds it in `$a1`, needing `$v0`, `$v1` **and** `$a0` occupied there.
Replaying the block (`tmp/ra/tmv_local.py`) gives a **two-part** requirement,
neither part sufficient alone: (1) one extra qty overlapping [6,38) with
pri > 22500 (e.g. refs 8, span ≤10) → moves it only to `$a0`; (2) hard `$a0`
live across [6,38) alone → unchanged `$v1`; **both together → `$a1`, target.**
That is why the call-argument family was inert at 22 — it supplies only part 2.
Target supplies both: `addiu a0,sp,16` at line 23 *before* the address
arithmetic keeps hard `$a0` (the `saTan1MainJump(buf)` arg) live across the
sign-extend (part 2), and `li v1,24` at 31 consumed by `sh v1,0(at)` at 34 makes
the `24` a three-insn local quantity in `$v1` (part 1). See `ra_solver/README`.

## sched_solver goal-mapper round (2026-08-05)

`perturb.py --goal-from-target main` derives target's order per block.
**Blocks 1 and 2 are goal == identity — the whole loop body's instruction ORDER
already matches target;** only block 0 (the prologue) differs, so the banked
"one placement" (`addiu a0,sp,16` early) is not a scheduling question.

Block 0 has two clusters. Emission slots 1–5: ours 172 `sw $16,80($sp)`, 9
`move $16,$0`, 157 `li $2,0x00060000`, 44 `lbu $3,D_80101BCC`, 158
`ori $2,$2,0x0093`; target 157, 44, 158, 172, 9 — constant/load group first, the
`$16` save and init after. Slots 18–20: ours 34, 37, 40; target 37, 40, 34.

**No single-atom vector** (1460 atoms, post-`ready0`-fix, a real negative).
Depth 2 returns one family: **`add_dep 9 <- 158 (true/data)` plus any one of**
`add_dep 34 <- 44`, a luid swap/move of 34 next to 46, or `cost 32 := 2`. The
second half is spellable (move the `sh $2,76($sp)` buffer-init store later, or
change its selection); **the first is not** — it needs the loop counter init
`var_s0 = 0` to consume the `0x00060093` buffer constant, which has no natural C
form. Same shape as `tslGlobalMemFree_800861BC` block 0. Note uid 172 is a
*reload-generated callee-save store*, so a vector moving it is evidence about
reload's save placement (frame/RA), not statement order.

## The real open question

Target keeps the two constants **separate and short-lived**: `24` in `$v1`
(31→34), the `1` in `$v0` (`li v0,1` at 50, `sllv v0,v0,a1` at 51), neither
hoisted. Our only anti-hoist lever routes both through ONE multi-set `t`, which
necessarily makes a single long-lived pseudo — exactly what destroys part 1. So:

> **why does `loop.c`'s `move_movables` not hoist target's single-set loop
> constants, when it hoists ours?**

Answer that and the `24` becomes a short-lived local qty in `$v1` (part 1), the
shared `t` is unnecessary, and part 2 is a placement question the call-argument
family already half-explored.

## Measured negative / inert (do not re-run)

All score 22 unless noted: named sign-extend local (`s32`/`s16 idx`, register
map bit-identical); `t` at loop-body block scope (bit-identical to candidate);
every `t` placement sweep (loop top, at each use, store reordered, shift-first,
idx+shift-first — shift-first is worse, 42 differing lines vs 40); call-arg
naming per [[hoist-call-arg-local-flips-jal-delay]] (`s32 *p = buf;`
block-wrapped, at loop-body top, `u8 *p`, the rule's exact shape) — but **at
FUNCTION scope it regresses to 35**; statement order around the call.

Worse or structurally wrong: `t` for the `0x18` only or the `1` only (still
hoisted, 27); reusing `offset` for the `1` (frame 96, regs 4, 74 insns, 20), and
routing the `1` through `offset` after the stores (74 insns); `s16 t` (74 insns,
score 16 — better on the metric, but an extra save and extra insns, not a base
to resume from); `t = (s16)var_s0; buf[0] = 1 << t;` (74 insns, breaks the
stream). Shift-first at the qty level adds a 5th qty (refs 4, span [10,16)) but
it allocates last into `$v0` — does **not** supply part 1.

`store-before-jal` does not apply (the pre-call `sw …,16(sp)` is already in the
jal delay slot in both builds). **cc1psx** ([[cc1psx-calibration-only]]) makes
the same choice we do — compiler divergence is dead.

**Build gate:** applying the candidate breaks the oracle (SHA1 `31fe8c21`) — the
18 regfix rules are calibrated to HEAD's shape. `src/main.c` reverted, build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Resume here — BANKED at 22, do not force

Start from `candidate.c`. Work the `move_movables` question above, not another
placement sweep: both parts must land together, part 1 is unreachable while `t`
is shared, and the loop's instruction order is now proven already correct.
