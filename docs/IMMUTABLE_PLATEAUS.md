# Immutable plateaus — diff types the targeted permuter can't crack

When `bb2_retire.py` reports `min_score > 0` (plateau), run:

```
python3 tools/bb2_diag_diff.py <func>
```

to see the asm-level diff. If the remaining diff matches one of the
patterns below, **stop investigating**. The current toolchain
(targeted permuter + 5 BB2 passes + heavy mode + auto-diag) cannot
solve them, and burning more CPU won't help.

Document these here so future agents don't re-investigate.

---

## Pattern 1: synthetic-intermediate register diff (`mfhi`, division)

**Example** — `cdrom_FramesToBcd`, base score 10:

```
off=0x  9c: TARGET: mfhi  t3        BUILT:  mfhi  t2
off=0x  b0: TARGET: sra   a0,t3,2   BUILT:  sra   a0,t2,2
```

**Why unfixable**: `mfhi`'s destination is the high half of the multiplier,
allocated by GCC's RA to whatever temp register is free at that instant.
There's no C variable in scope that "holds" the mfhi result -- it's a
synthetic intermediate the optimizer emits. No `register T x asm("$N")`
pin can target it because no `x` exists.

The only way to redirect it would be inline-asm injection
(`__asm__ volatile("mfhi $11")`), which is BANNED per the
inline-asm-injection rule.

**Recommendation**: keep the regfix rule. This is a legitimate
post-asm rewrite (matches SOTN community standard for this exact
case).

## Pattern 2: addressing-mode diff with specific base register

**Example** — `func_80078A68`, plateau 10:

```
off=0x  30
  TARGET: sh   a1,8(v1)        [a4650008]
  BUILT:  sh   a1,16(zero)     [a4050010]
```

Both stores write to the same address (target: `$v1 + 8`, built: `$zero + 16`).
Different *expressions* for the same address. GCC chose one form; target
used another.

**Why unfixable**: GCC's expression-to-addressing-mode lowering is based
on its internal cost model. Source-level C changes rarely flip this
decision because the optimizer recognizes the alternates as equivalent
and picks the cheaper form. The `perm_bb2_strength_reduce` pass tries
some rewrites but doesn't target addressing mode specifically.

**Recommendation**: keep the regfix rule. If it's a 1-instruction subst,
treat as the project's pragmatic compromise.

## Pattern 3: reorder of non-adjacent C statements

**Example** — `myRobGeneiDraw2`, plateau 20:

```
off=0x  c0: TARGET: lw v1,4(s0)        BUILT: lw v0,36(s0)
off=0x  c4: TARGET: lw v0,36(s0)       BUILT: lw v1,4(s0)
```

The two `lw` instructions are emitted in swapped order. Their originating
C statements aren't adjacent in the source body (they're members of
different struct accesses far apart in the C).

**Why unfixable**: `perm_bb2_scheduler_perturb` only wraps ADJACENT
statements in `do { } while (0)`. Wrapping non-adjacent statements
requires arbitrary code motion that the permuter's mutation set doesn't
include.

**Recommendation**: keep the regfix rule, or do manual structural
rewrite of the function body if it's worth the effort.

## Pattern 4: 1-instruction single-source-register diff

**Example** — `func_80089E30`, plateau 15:

```
off=0x  28: TARGET: move a1,v0       BUILT: move a1,v1
```

Same `move` instruction, same destination, different source register
($v0 vs $v1). Fixable in principle by pinning the source variable to
$v0, but the random `perm_bb2_add_pin` doesn't search efficiently
enough -- 5 passes × thousands of iters × low probability of picking
the exact var+reg combo means it rarely lands.

**Why nearly-unfixable with current tools**: the search is random.
A diff-targeted pin pass (see Phase 7 recommendations) would convert
this from "plateau" to "match" by exhaustively trying the obvious
pin combinations, but doesn't exist yet.

**Recommendation**: until a diff-targeted pass lands, keep the regfix
rule. Or manually identify which var feeds the source reg and add a
`register T x asm("$v0")` pin to it.

## Pattern 5: epilogue / early-return shape difference

**Example** — `func_80078B70`, plateau 60, drain_delay regfix:

```
off=0x  28: TARGET: li v0,1           BUILT: sw v1,4(a1)
off=0x  2c: TARGET: sw v1,4(a1)       BUILT: jr ra
off=0x  30: TARGET: jr ra             BUILT: li v0,1
off=0x  34: TARGET: nop               BUILT: (none)
```

Target and built emit the same instructions, just in different order
relative to the function epilogue. Target also has a trailing `nop`
that built lacks (different delay-slot fill).

**Why unfixable**: epilogue ordering and delay-slot fill are GCC's
late-stage decisions, driven by scheduler cost model. Source-level
C changes rarely influence epilogue shape.

**Recommendation**: keep the regfix rule. `drain_delay`/`fill_delay`
rules exist for exactly this category and SOTN-equivalents accept
them as legitimate.

---

## Patterns the toolchain CAN handle

For comparison, these are the patterns that DO get matched:

- **Uniform 2-way swap** like `$2 <-> $25`: pin a relevant variable
  to one of the two registers. `perm_bb2_add_pin` does this.
- **Adjacent-statement reorder**: wrap two statements in
  `do { } while (0)`. `perm_bb2_scheduler_perturb` does this.
- **Multiplication strength-reduce**: `x * 4` <-> `2 * (2 * x)`.
  `perm_bb2_strength_reduce` does this.
- **Volatile qualifier on existing pin/extern**: e.g., `s32 ra0` to
  `volatile unsigned int ra0`. `perm_bb2_type_qualifier` does this.
- **Function-local extern type override**: change an `extern T *X` to
  `extern volatile T *X` locally. `perm_bb2_type_qualifier` does this.
- **Intermediate variable introduction**: `result = expr; assign = result;`
  instead of `assign = expr;`. Upstream `perm_temp_for_expr` does this.

If your diff doesn't match Patterns 1-5 above AND the permuter
plateaus at non-zero, you're probably hitting a search-budget issue
(not enough iters/seeds) rather than a fundamental tool gap. Try
`--time 600` or `--max-base 500` first before giving up.

---

## Quick triage flowchart

```
Plateau at min_score > 0 in bb2_retire.py
    |
    v
Run: python3 tools/bb2_diag_diff.py <func>
    |
    +-- mfhi / division intermediate register?    -> PATTERN 1, keep regfix
    |
    +-- addressing mode (e.g., 8(rb) vs K(zero))? -> PATTERN 2, keep regfix
    |
    +-- two same-opcode lines swapped?
    |       |
    |       +-- originating C statements ADJACENT? -> tool should catch it;
    |       |                                          retry with longer time
    |       |
    |       +-- originating C statements DISTANT?  -> PATTERN 3, keep regfix
    |
    +-- 1 line with 1 reg differing?              -> PATTERN 4, manual pin
    |                                                or wait for diff-targeted
    |
    +-- epilogue / `jr ra` ordering changed?     -> PATTERN 5, keep regfix
    |
    +-- Other / unclear                          -> longer search; or document
                                                    a new pattern here.
```
