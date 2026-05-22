# func_8007C7A0 — handoff (rewritten 2026-05-22, corrected diagnosis)

`s32 func_8007C7A0(s16 arg0, s16 arg1)` in `src/display.c:527`. Clamps two screen
coords to `[0, maxX-1]` / `[0, maxY-1]` and packs a GP0(0xE3) "set drawing area"
command. One of a 5-function draw-area family (C7A0, C86C, C748, C938, C97C).

## Status
- **Builds GREEN** via **21** `func_8007C7A0:` regfix `subst` rules (`regfix.txt`
  ~lines 3155-3176). All are **register-rename + frame-size** substs. Nothing broken.
- **Goal:** retire those rules → tier-4 (0 regfix, 0 pins, 0 inline asm). **Not achieved.**
- **This is a PURE REGISTER-ALLOCATION problem, structurally a 100% match.**

## CORRECTION to the prior handoff (read this — the old framing was a red herring)
The old handoff said "the whole problem is getting raw arg0 into `$a3` (the funnel)."
**That is wrong. The committed body ALREADY saves raw arg0 → `$a3` correctly** (the
`move a3,a0` matches target; harness reports `rawreg=a3`). Independent measurement
(2026-05-22): the committed m2c body compiles to **51 instructions in the exact same
order as target** — zero structural / reordering / missing-instruction differences.
The 21 diffs are **20 register-renames + 1 frame-size** (`sp-24`→`sp-16`; the extra
8 bytes are unused — no spill). `dc.sh verify` lies here; use `tmp/c7a0_try.py`.

## The real wall (precise)
Target and our GCC-2.7.2 fork pick **two different but internally-consistent register
allocations**. Per-value (target → our fork's natural choice):

| value | target | our fork |
|---|---|---|
| clamped x | **a3** | a0 / t0 (fragmented) |
| maxX (bound) | a2 | v1 |
| maxY (bound) | a0 | v1 |
| clamped y | a1 | a2 |

**Root pattern:** target uses the *argument* registers `a0–a3` heavily and **avoids
`$v1`**; our fork follows GCC's default ascending preference (`$v0,$v1` first), so it
parks the short-lived clamp bounds in `$v1`. Per `register-alloc-pure-c.md` step 0,
**target uses HIGHER registers than us** (a3>a0, a2>v1, a0>v1) — the documented
"rarer / harder" direction. We could not find clean C that flips it.

## NEW lever found 2026-05-22 (genuine progress over prior agents)
**`var_v0_2 = arg0;` as an explicit init** (before the clamp `if`) makes the CLAMPED x
land coherently in `$a3` — `xreg=a3` AND `rawreg=a3` in a **clean 51-insn frame=16
body at edit-21** (`tmp/c7a0_W1.c`). Prior agents (attempt a17) believed both-a3 was
reachable *only via stack-spill garbage*; W1 disproves that. W1 is the new best clean
base and the right permuter seed.

Tension discovered: the `var=arg` init drives x→a3 only with an *implicit* inner else
(which restructures the x-clamp → schedules the y-sign-extend early); restoring the
*explicit* `else var=arg` round-trip + the init is redundant and GCC collapses it. The
m2c `new_var2` copy lowers maxX/maxY/y diffs (→ edit-19) but re-splits x to `$t0`.
No single hand-structure gets x-coherence AND the maxX/maxY/y allocation; clean
variants cluster **edit 19–23** (see `.bb2_attempts/func_8007C7A0.jsonl` a19/a20).

## Permuter result (2026-05-22, from the NEW clean W1 seed)
`tmp/c7a0_fullbuild_permute.py 750` (reseeded SEEDS=[W1,W6,e20], scored by real
full-build edit + both-a3 bonus): 3870 iters, descended **21→20→19→18→17**, all
`xreg=a3 rawreg=a3` — **beat the prior ~19 plateau**. BUT the edit-17 body
(`tmp/c7a0_fb_best.c`) is **permuter garbage** (25 junk locals, `(float)1`,
`volatile int D_8009BE7A`, comma-exprs) — not a legitimate decomp. Confirms the prior
finding: sub-19 edit is reachable only via non-legitimate bodies.

## Refuted hypotheses (don't redo)
- **arg-count / "GPU helper had 4 args":** 4-param signature (2 unused) → byte-identical
  to 2-param (edit-21). Unused params are dead at entry; do NOT occupy a2/a3. (a21)
- **register pin `var_v0_2 asm("$7")`:** ignored by GCC on the m2c body (it splits x
  into 3 pseudos, no single x to pin); pushed rawreg to v1, edit unchanged.
- **clean natural-C clamp (separate x/y, standard if/else-if):** edit-29, loses raw→a3.
- compiler flags (settled, `compiler-flags-canonical.md`); cc1psx parity (closed).

## Option 3 result (2026-05-22) — legitimacy-filtered permuter + correct sweep
Ran `tmp/c7a0_legit_permute.py` (decomp-permuter with garbage passes disabled —
volatile/float/comma/self-assign/extern-type/pad — and decl-reorder/temp/narrow-type/
block-scope levers boosted; syntactic legit gate). **Plateaued at edit-20, and that
body is SEMANTICALLY BROKEN** (`var_v0 = var_v1 & 0x3FF` after `var_v1 = (y&0x3FF)<<10`
→ x is dropped). **Key finding: decomp-permuter hill-climbs by byte-alignment and
rewards semantic-breaking mutations; a syntactic legitimacy filter cannot catch them.
So the permuter cannot reliably climb to a CORRECT match here — only edit-0 would be
trustworthy, and no run (mine or the prior 52k+ iters) ever reached edit-0.**
Follow-up `tmp/c7a0_sweep4.py` (5 *semantically-correct* pressure levers: y-copy,
decl-order, artifact-removal, uncond-y-init, partial-xfield-split) → all edit-22, none
flip maxX→a2 / maxY→a0 / y→a1. **The clean+correct floor is firmly 19 (W6, x→t0) – 22.**
Root: target avoids `$v1` for the clamp bounds, but NO semantically-correct C value
naturally lives in `$v1` across the clamp blocks, so the arg-register allocation cannot
be coerced from clean C.

## Honest assessment
Pure-C **semantically-correct** floor ≈ **19–22 register-rename diffs**; the permuter's
sub-19 "results" are semantic garbage (don't count); absolute garbage floor ≈ 17;
**none reach 0**. Target's arg-register-heavy / v1-avoiding allocation was not
reproduced by any clean C across ~30 h combined effort + a fresh permuter run from a
better seed. Strong evidence this is at/below the toolchain's clean pure-C ceiling.

## Options for closing the gap (pick per appetite)
1. **Accept the 21 regfix register-renames** as the `register-asm-pins.md`-prescribed
   handling for an unwinnable RA tie (tier-3 debt, function stays matched). Lowest risk.
2. **Reverse-engineer the exact original source** that produced target's allocation
   (the v1-avoidance suggests the original had an extra live value occupying v1 across
   both clamp blocks — find what, in clean C). Highest payoff, low odds, high effort.
3. **Permuter marathon from W1** with a *legitimacy filter* (reject bodies with junk
   locals / wrong types) — only accept score drops from real structural levers. Could
   confirm whether a *clean* sub-21 exists. Medium effort.

## Files / how to measure
- `tmp/c7a0_W1.c` — NEW best clean base (edit-21, both-a3). Start here.
- `tmp/c7a0_try.py` — ground-truth harness (real display.o build + objdump diff).
  `cp <body> tmp/c7a0_cand.c && bash tools/wsl.sh 'C7A0_FULL=1 python3 tmp/c7a0_try.py'`.
- `tmp/c7a0_sweep2.py` / `c7a0_sweep3.py` — structural variant sweeps (this session).
- `tmp/c7a0_fullbuild_permute.py` — real-edit-scored permuter (reseeded to W1/W6/e20).
- `tmp/c7a0_ref.norm` — normalized target objdump (matches `asm/funcs/func_8007C7A0.s`).
- Attempt log: `.bb2_attempts/func_8007C7A0.jsonl` (a19–a21 = this session).
