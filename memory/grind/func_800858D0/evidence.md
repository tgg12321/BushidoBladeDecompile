# Evidence bank — func_800858D0

- WIP rejected_form: {'form': 't for the 0x18 only', 'score': 27, 'reason': 'stays single-set, still hoisted'}

- WIP rejected_form: {'form': 't for the 1 only', 'score': 27, 'reason': 'stays single-set, still hoisted'}

- WIP rejected_form: {'form': 'reuse the existing `offset` variable for the 1', 'score': 20, 'reason': 'frame 96 but regs=4, insns=74'}

- WIP rejected_form: {'form': 'offset reuse for 1 + t for 0x18', 'score': 20, 'reason': 'regs=4, insns=74'}

- WIP rejected_form: {'form': 's16 t instead of s32 t', 'score': 16, 'reason': 'better metric but structurally worse: regs=4, insns=74. Not the resume base.'}

- WIP rejected_form: {'form': 'named sign-extend local (s32 idx / s16 idx) reused as the shift amount', 'score': 22, 'reason': 'inert'}

- WIP rejected_form: {'form': 't placement: loop top / immediately before each use / 0x18 store last / shift before stores / idx+shift-first', 'score': 22, 'reason': 'all inert'}

- == imported from memory/wip notes.md ==
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

Frame, saves and insn count all match target; the 22 is register naming.

## The lever — [[defeat-licm-hoist-var-reuse]]

**Never a phantom-frame problem.** Both have `vars=64`; we emitted **5
callee-saves against target's 3**. The extra saves were loop-invariant constants
`loop.c` hoisted (`li s3,24`, `li s2,1`); target rematerialises both in the loop.
Routing both through ONE scratch local (`s32 t; t = 0x18;` before the six stores,
`t = 1;` before `buf[0] = t << (s16)var_s0;`) makes the pseudo multi-set, so it
is not a `move_movables` candidate. GCC rotates the `t = 0x18` reload into the
loop-back branch's delay slot — loop rotation, not a stale value. Same shape
already shipping in `func_8003DBE4`.

## The swap, and its quantified spec (ra_solver Phase 5)

Ours puts `t` in `$a1` with the sign-extended `var_s0` in `$v1`; target has them
swapped, and emits `addiu a0,sp,16` early where we emit it late. Global model
3/3 (allocnos 75 = `t`, 72 = `var_s0`, 74 = `ff`); `ra_solver/perturb.py` spec
`{75: 3}` finds NO vector — correct, not a search failure: the `$v1` block on
`t` is the sign-extend, a **local-alloc** quantity. The local-alloc model is
exact here (2/2 order, 10/10 assign); block 2 allocates qty2 (pri 60000)→`$v0`,
qty1 (50000)→`$v0`, **qty0 = the sign-extend, span [6,38), pri 22500 →`$v1`**,
qty3 (20000)→`$v1` — `find_free_reg` scans ascending, so it takes `$v1` because
only `$v0` is occupied across its range.

Target holds it in `$a1`, needing `$v0`, `$v1` **and** `$a0` occupied there.
`tmp/ra/tmv_local.py` gives a **two-part** requirement, neither part sufficient:
(1) an extra qty overlapping [6,38) with pri > 22500 → moves it only to `$a0`;
(2) hard `$a0` live across [6,38) alone → unchanged `$v1`; **both → `$a1`.**
That is why the call-argument family was inert at 22 — it supplies only part 2.
Target supplies both: `addiu a0,sp,16` at line 23 keeps hard `$a0` live across
the sign-extend, and `li v1,24` at 31 consumed at 34 makes the `24` a three-insn
`$v1` quantity. Full write-up in `ra_solver/README`.

## sched_solver goal-mapper round (2026-08-05)

`perturb.py --goal-from-target main`: **blocks 1 and 2 are goal == identity —
the whole loop body's instruction ORDER already matches target;** only block 0
(the prologue) differs, so the banked "one placement" (`addiu a0,sp,16` early)
is not a scheduling question.

Block 0 differed in two clusters (slots 1–5 and 18–20). No single-atom vector;
depth 2 restricted to **spellable atoms** (`--atoms luid,luid_move`) returns a
**fully spellable pair** — two ordinary statement moves, no dependence surgery:

| | edit |
|---|---|
| **A** (`luid swap 9 <-> 14`) | put `buf[1] = 0x60093;` **before** `var_s0 = 0;` |
| **B** (`luid swap 34 <-> 46`) | move `*(s16 *)((u8 *)buf + 0x3C) = 0x4000;` to the **end** of the init block |

**Both tested by compiling the TU.** Honest-stream insns differing from target:
HEAD **19** → A **17** → B **16** → **A+B 14**; the halves compose.

**Re-goaled from the A+B form against a PINNED HEAD target: all three blocks are
now goal == identity — the scheduling class is CLOSED here.** `hon→tgt` goes
`equal 38 / moved 6` → `equal 43 / moved 1`. What is left is `replace 12`
(register naming) and `delete 5 / insert 1` (61 insns to target's 57) — the
extra callee-saves, i.e. the `move_movables` question below.

Note uid 172 is a *reload-generated callee-save store*, so a vector moving it is
evidence about reload's save placement (frame/RA), not statement order.

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
map bit-identical); `t` at loop-body block scope; every `t` placement sweep
(loop top, at each use, store reordered, shift-first, idx+shift-first —
shift-first is worse, 42 differing lines vs 40); call-arg naming per
[[hoist-call-arg-local-flips-jal-delay]] (`s32 *p = buf;` block-wrapped, at
loop-body top, `u8 *p`, the rule's exact shape) — but **at FUNCTION scope it
regresses to 35**; statement order around the call.

Worse or structurally wrong: `t` for the `0x18` only or the `1` only (still
hoisted, 27); reusing `offset` for the `1` (frame 96, 74 insns, 20) and routing
the `1` through `offset` after the stores (74 insns); `s16 t` (74 insns, 16 —
better on the metric but an extra save and extra insns); `t = (s16)var_s0;
buf[0] = 1 << t;` (74 insns). Shift-first at the qty level adds a 5th qty but it
allocates last into `$v0` — does **not** supply part 1. `store-before-jal` does
not apply (the pre-call `sw …,16(sp)` is already in the jal delay slot both
sides). **cc1psx** ([[cc1psx-calibration-only]]) chooses as we do.

**Build gate:** applying the candidate breaks the oracle (SHA1 `31fe8c21`) — the
18 regfix rules are calibrated to HEAD's shape. `src/main.c` reverted, build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Resume here — BANKED at 22, do not force

Fold the A+B statement moves into `candidate.c` and re-measure, then work the
`move_movables` question above, not another placement sweep: both RA parts must
land together, part 1 is unreachable while `t` is shared, and the loop's
instruction order is now proven already correct.


## s1 (2026-08-20, recon — first post-migration session)

- **New measured floor: 15** (was 22). src/main.c:857 is INCLUDE_ASM at HEAD
  (asm-until-matched migration); the 18 regfix rules are RETIRED, so the old
  "applying the candidate breaks the oracle" warning is MOOT — sessions now just
  apply candidate.c over the INCLUDE_ASM line and sandbox. Candidate = floor-22
  form + A+B statement moves folded (A: buf[1]=0x60093 before var_s0=0;
  B: 0x3C store last in init block). build_insns 72 == target_insns 72.
- **Residual-15 map** (ours = tmp/sandbox/func_800858D0/main.o vs
  asm/funcs/func_800858D0.s; full alignment in tmp/grind/func_800858D0/s1/):
  (a) rotation identity — reorg steals the loop-body block-top insn into
  preheader (slot 22) + loop-back delay slot (slot 65): target steals
  `addiu a0,sp,16`, we steal the t-carrier's `li 24`;
  (b) mid-loop cluster — our `li a1,1` / `addiu a0,sp,16` / `sllv` sit right
  after store #1 (t=1 is freed the moment the 24's store retires, and sllv's
  dest kills idx); target keeps `li v0,1; sllv v0,v0,a1` just before the jal;
  (c) register naming — idx sign-extend a1 (target) vs v1 (ours); the 24 in v1
  vs a1; the 1/shift-result in v0 vs a1/v1.
- **loop.c decision profile now MATCHES target**: main.loop shows "Loop from 52
  to 146: 30 real insns" with ZERO movables (t = reg 75 only "possible biv,
  discarded"). Target's stream likewise shows nothing hoisted.
- **Single-set constants are PROVEN always-hoisted here** (kills a whole
  spelling class): threshold = (1+n_non_fixed_regs) = 61 with calls in loop
  (loop.c:532, mips.h FIXED_REGISTERS: 8 fixed of 68); move test
  61*savings*lifetime >= insn_count=30 holds even at savings=lifetime=1
  (loop.c:1631). The reg_single_usage substitution escape (loop.c:735) cannot
  fire: movhi's condition (mips.md:3352) rejects const-source HI stores (only
  const 0 via $zero), sllv rejects const shift operands. So target's 24/1 were
  NOT single-set const pseudos — and ours must not be either. The remaining
  puzzle is that target uses TWO registers (v1 for 24, v0 for 1) while the only
  known no-movable spelling (shared multi-set t) yields ONE.
- **sched1 mechanism for cluster (a)**: main.sched block 2 — backward scheduler;
  our t=24 set (insn 71, priority 1) is ready from T-14 but loses to the
  0x7f000001 address-chain insns every cycle and lands FIRST in the block →
  reorg steals it. Whatever insn sits at block top becomes the
  preheader+delay-slot pair. Target had the a0-set there.
- **Re-measured kill**: block-scope `s32 *p = buf;` at loop-body top is inert at
  15 on the new chassis (canon_reg folds it; rejected/p-loop-top-block-scope-
  inert-15.c). Function-scope p regressed to 35 on the old chassis — do not
  retry either.
- Artifacts: tmp/grind/func_800858D0/dumps/ (main.loop / main.sched / main.greg
  etc., generated FROM the floor-15 form), tmp/grind/func_800858D0/s1/.
- NOTE for next session: the old-chassis sched_solver "goal == identity /
  scheduling CLOSED" claim predates the migration and describes the A+B form vs
  the OLD sandbox — the observed cluster (b) means scheduling questions are OPEN
  again on the new chassis. The proven next tools: sched_solver
  `perturb.py --goal-from-target --atoms luid,luid_move` (it FOUND A+B) and
  ra_solver Phase 5 re-run, both against the floor-15 form.

- [s1] HEAD floor is unmeasurable (INCLUDE_ASM since asm-until-matched migration); migration pin 27; new banked candidate floor 15, measured this session

- [s1] Residual-15 = 3 clusters: (a) reorg preheader/delay-slot steal takes OUR li 24 where target's takes addiu a0,sp,16; (b) our li 1 / addiu a0 / sllv cluster sits after store #1 vs target's just before the jal; (c) register naming idx a1-vs-v1, 24 v1-vs-a1, 1/shift v0-vs-a1/v1 (alignment: tmp/grind/func_800858D0/s1/recon-alignment.md)

- [s1] main.sched block 2 (backward scheduler): t=24 set insn 71 priority 1, ready from T-14, loses to 0x7f000001 address-chain insns, lands FIRST in block -> reorg steals it; whatever insn is at block top becomes the preheader+delay pair

- [s1] Target uses TWO registers for the constants (24 in v1, 1 in v0) while the only known no-movable spelling (shared multi-set t) yields ONE (a1) - this two-carrier split is the core unsolved puzzle

- [s1] Old-chassis 'scheduling CLOSED / goal == identity' claim is stale: cluster (b) shows in-loop order divergence on the new chassis; sched_solver and ra_solver must be re-run against the floor-15 form

- [s1] The old 'candidate breaks the oracle' warning is moot: the 18 regfix rules are retired, src/main.c:857 is INCLUDE_ASM, sessions apply candidate.c directly
