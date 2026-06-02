---
name: func-8007c7a0-sotn-duplicate-read-lever
description: func_8007C7A0 — sandbox 16 → 12 via SOTN-allowed Y-wide-mask duplicate-read precompute (new lever post 2026-06-01 policy update); structural ceiling = $a2-vs-$a3 X-preserve register tiebreaker remains; raw_arg0=var_v0_2 literal-rename reuse hits score 10 but is FORBIDDEN as cheat-by-spelling per [[param-local-alias-prologue-pair-flip]] / [[no-new-park-categories]]
metadata:
  type: project
---

# func_8007C7A0 — duplicate-read lever lowers honest floor from 16 to 12

## Status

**INCOMPLETE — re-parked 2026-06-02 (third retry session).** Prior sessions:
- Session A (commit `bc65a49`, 2026-05-22): 19-22 floor across 23 attempts + 52k+ permuter iters
- Session B (memory `func-8007c7a0-arg1-preload-lever.md`, 2026-06-01): 18 floor via `var_a1 = arg1` unconditional preload
- Session C (memory `func-8007c7a0-named-intermediate-lever.md`, 2026-06-01): 16 floor via C97C-style `r_e3` named intermediate + drop coercion vars + m2c-shape inline
- **Session D (this, 2026-06-02): 12 floor via SOTN-allowed duplicate-read precompute on Y wide-mask**

The 21 regfix `subst` rules in `regfix.txt:3050-3071` remain. Function stays
matched-with-rules.

## The new lever — SOTN-allowed duplicate-read precompute

Today's policy update (commit `161c6c3`) made the **duplicate-read into branch arms**
technique ALLOWED based on direct SOTN master-branch evidence (`src/dra/42398.c`
`color_fake = *palette;` repeated rebinds). Applied here: precompute the Y wide
mask (`var_a1 & 0xFFF`) BEFORE the mode dispatch, then overwrite it with the
narrow mask inside the narrow-mode branch:

```c
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 var_v0_2;
    s16 var_a1;
    s32 var_v0;
    s32 var_v1;

    var_a1 = arg1;                                /* session B's preload */
    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            var_v0_2 = D_8009BE78 - 1;
        } else {
            var_v0_2 = arg0;
        }
    } else {
        var_v0_2 = 0;
    }
    if (var_a1 >= 0) {
        if ((D_8009BE7A - 1) < var_a1) {
            var_a1 = D_8009BE7A - 1;
        }
    } else {
        var_a1 = 0;
    }
    var_v1 = var_a1 & 0xFFF;                      /* SESSION D: precompute Y wide-mask */
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        var_v1 = var_a1 & 0x3FF;                  /* overwrites in narrow-mode branch */
        var_v1 = var_v1 << 0xA;
        var_v0 = var_v0_2 & 0x3FF;
    } else {
        var_v1 = var_v1 << 0xC;                   /* uses precomputed wide-mask */
        var_v0 = var_v0_2 & 0xFFF;
    }
    {
        s32 r_e3 = var_v0 | 0xE3000000;
        return var_v1 | r_e3;
    }
}
```

`sandbox --disable all`: 16 → **12**. `build_insns` 50 (target 51, 1 short — the
park insn). 

### Why this works (mechanism)

Target's mode dispatch emits `andi $v1, $a1, 0xFFF` in the bnez **delay slot**
(idx 37). cc1's `reorg.c` fills delay slots from the fall-through stream — but
only if the candidate insn doesn't conflict with the branch's polarity. In the
baseline (no precompute), cc1 didn't emit a fall-through insn that could fill
the slot because the narrow-mode branch did its own mask compute.

With `var_v1 = var_a1 & 0xFFF;` precomputed BEFORE the dispatch:
- The wide-mode branch READS the precomputed value (`var_v1 << 12`)
- The narrow-mode branch OVERWRITES it (`var_v1 = var_a1 & 0x3FF`)
- The precompute itself sits in the bnez delay slot — matching target exactly

This is the canonical SOTN duplicate-read pattern: pin the offset computation
inside one branch via duplication, where the speculative computation acts as the
delay-slot fill.

## Variants tested this session

| Variant from baseline (HEAD) | Score | Build insns | Notes |
|---|---|---|---|
| baseline (committed m2c form) | 20 | 51 | reference |
| session C's 16-score form | 16 | 50 | r_e3 + drop coercion + inline (re-confirmed) |
| **+ Y wide-mask precompute (SESSION D LEVER)** | **12** | **50** | NEW — duplicate-read |
| + X narrow-mask precompute (`var_v0 = var_v0_2 & 0x3FF;` before dispatch) | 15 | 50 | regression — interferes with dispatch |
| + arg0 reuse for var_v0_2 (no separate decl) | 20 | 50 | param-aliasing collapse |
| + var_v0_2 = arg0 init + reuse | 21 | 48 | tight collapse |
| + var_v0_2 self-assign (`var_v0_2 = var_v0_2;`) | 16 | 50 | DCE'd |
| + xlim/ylim named intermediates (extending compare live range) | 12 | 50 | CSE'd — no change |
| + raw_x = arg0 + use in else | 12 | 50 | CSE'd to arg0 |
| + raw_arg0 = arg0 + raw_arg0 = var_v0_2 reassign | **10** | **51** | best so far — BUT see "forbidden" below |
| + opaque arithmetic chain | 12 | 50 | DCE'd |
| + operand-order flip (local > global) | 12 | 50 | RTL canonicalized |

## The borderline-forbidden score-10 form (NOT COMMITTABLE)

Adding `s16 raw_arg0 = arg0;` (literal rename) followed by `raw_arg0 = var_v0_2;`
mid-function (reuse for clamp result) drops score 12 → **10** AND produces
**build_insns 51** (target match). The structural shape is correct; the missing
park insn appears as `move $a2, $a0` (wrong direction vs target's `move $a3, $v0`).

**This form is forbidden per [[param-local-alias-prologue-pair-flip]] /
[[no-new-park-categories]] / [[review-discipline-before-commit]]:**

1. **`s16 raw_arg0 = arg0;`** is a **literal rename** of arg0 (identical type,
   identical lvalue, no semantic information added). Per the param-local-alias
   rule explicitly forbidden 2026-06-02: "Local variables that are literal renames
   of params with leading underscores [or other coercion-pattern names], declared
   in a specific order to bend [register allocation], are not [legitimate]."

2. **`raw_arg0 = var_v0_2;` mid-function** is a literal-rename-followed-by-reuse
   pattern — a chain that extends the live range of an artificial local solely
   to coerce GCC's allocator into picking a specific register.

3. The construct has **zero semantic purpose** — the original semantics are
   identical whether you write `var_v0_2 & MASK` directly or chain through
   `raw_arg0`. The intent is purely RA coercion.

4. The justification references GCC internals (allocno priority, pseudo
   creation order biasing $a3 vs $a2 tiebreaker) — the canonical cheat-by-
   spelling signal.

The cheat-reviewer agent would FAIL this form. Documented here ONLY so future
agents recognize the pattern shape and don't re-derive it.

## The remaining structural ceiling (UNCHANGED)

The missing park insn at `.L8007C7E0` plus the X-preserve register choice
($a2 mine vs $a3 target) cascade through 10 register-rotation diffs. Index-
aligned diff at score 12:

| idx | mine (score 12)        | target              | issue |
|---|---|---|---|
| 0 | `move a2,a0`            | `move a3,a0`         | $a2 vs $a3 (X preserve) |
| 8 | `move v1,v0`            | `move a2,v0`         | $v1 vs $a2 (X-limit save) |
| 12 | `addiu a0,v1,-1`       | `addiu v0,a2,-1`     | dest/src cascade |
| 14 | `move a0,a2`           | `move v0,a3`         | mirror |
| 15 | `move a0,zero`         | `move v0,zero`       | mirror |
| 16 | (MISSING)              | `move a3,v0`         | THE PARK INSN |
| 22 | `move v1,v0`           | `move a0,v0`         | $v1 vs $a0 (Y-limit save) |
| 26 | `addiu a1,v1,-1`       | `addiu a1,a0,-1`     | mirror |
| 41 | `andi a0,a0,0x3ff`     | `andi v0,a3,0x3ff`   | mask cascade |
| 43 | `andi a0,a0,0xfff`     | `andi v0,a3,0xfff`   | mask cascade |
| 44 | `lui v0,0xe300`        | `lui a0,0xe300`      | const dest |
| 45 | `or v0,a0,v0`          | `or v0,v0,a0`        | operand swap |

cc1's default ascending allocation order ($a2=6, $a3=7) picks $a2 for the X
preserve when both are free. Target picks $a3. Every C lever tested (named
intermediates, xlim/ylim explicit reads, declaration-order swaps, operand
flips) leaves cc1's choice unchanged — the legitimate pure-C levers don't
flip this tiebreaker.

## Why this is NOT committable (per persistence contract)

Same as sessions B and C: the score-12 src change does NOT retire any regfix
rules. The 21 substs would need rewriting to handle the new cc1 emission shape.
Net = lateral movement, no rule reduction, no completion progress.

Tested: with the score-12 form in src, `verify-oracle` (no rebuild) passes
because the cached build/ is stale. A FULL `eng.ps1 build` produces SHA1
`838d440d19f7c50d2820558cce2486339d537a13` — mismatch with oracle. So the
existing rules do NOT accommodate the new shape; they would need refactoring.

The `park_src_guard` hook correctly blocks `park:` commits that modify build
files. Source reverted to HEAD; commit only the memory note.

## Concrete next-step hypotheses for the next session

The score-12 form is a true lower bound for legitimate (non-cheat) pure-C in this
function. The score-10 form (raw_arg0 chain) is forbidden cheat-by-spelling.

UNTRIED legitimate levers:

1. **`tools/decomp-permuter` from the 12-score base** — directed at the X-preserve
   register slot. Permuter from a 12-base might find a structural variant the
   manual search missed. CRITICAL: vet output against cheat catalog before
   proposing per [[no-new-park-categories]] AND invoke `cheat-reviewer` per
   [[review-discipline-before-commit]]. Reject any closing form using literal
   renames, dead conditional stores, or other catalog patterns.

2. **ALLOCDBG-instrumented dump on the score-12 form** to confirm WHY cc1
   picks $a2 over $a3 for the X preserve. The instrumented cc1
   (`tmp/gccdbg/cc1`) from prior sessions can dump allocno priorities. A
   specific livelen/refs imbalance might point at a structural change that's
   not yet flagged as a cheat.

3. **Read target's m2c-reconstructed structure** more carefully for the X-clamp.
   m2c's output for func_8007C7A0 (from prior session's permuter setup) might
   suggest a control-flow shape that doesn't require literal renames.

4. **Sibling cross-reference** — `func_8007C748` (3-arg variant, matched
   pure-C) and `func_8007C86C` (sibling with 0xE4 instead of 0xE3, same 21-rule
   cheat pattern). Compare C748's .greg dump to identify what natural C
   structure made its var_v0_2-analog land in the target reg.

## Sibling impact

`func_8007C86C` (regfix.txt:3083-3104) has IDENTICAL 21-rule pattern (0xE4 vs
0xE3). The session-D lever (Y wide-mask precompute) directly applies to C86C —
expected sandbox 20 → 12 same way. Both stay parked.

## Files / how to reproduce

- Baseline (committed, score 20): `src/display.c:520-560` (HEAD m2c body)
- Session C's score 16: drop new_var/new_var2 + var_a1=arg1 preload + r_e3
  named intermediate + inline `(var_a1 & MASK) << SHIFT`
- **This session's score 12**: add `var_v1 = var_a1 & 0xFFF;` BEFORE the mode
  dispatch (precompute), then `var_v1 = var_a1 & 0x3FF;` (overwrite) in the
  narrow-mode branch
- The forbidden score 10: above + `s16 raw_arg0 = arg0;` decl + `raw_arg0 =
  var_v0_2;` mid-function reassign + use raw_arg0 in mask exprs
- Verify score 12: apply the lever-set, then `& tools/eng.ps1 sandbox
  func_8007C7A0 --disable all` → `"score": 12`

## Related

- [[func-8007c7a0-named-intermediate-lever]] — session C's prior memory note;
  this session's lever stacks on top of session C's r_e3 + drop-coercion form
- [[func-8007c7a0-arg1-preload-lever]] — session B's prior memory note
- [[split-read-defeats-hoist]] — the SOTN duplicate-read-into-branch-arms
  technique used here (now resolved as ALLOWED per 2026-06-02 borderline-
  research, commit 161c6c3); pin offset computations inside their branch via
  duplication
- [[no-new-park-categories]] — register-rotation infrastructure is NOT an
  acceptable carve-out; function stays parked as unfinished work
- [[param-local-alias-prologue-pair-flip]] — explicitly forbids the
  `raw_arg0 = arg0` literal-rename pattern; the score-10 form documented here
  hits exactly this forbidden case
- [[review-discipline-before-commit]] — the cheat-reviewer gate that would
  catch the score-10 form as FAIL
- [[difficult-is-not-impossible]] — the cardinal rule. 20 → 12 is real
  progress; the matching C structure exists, just not yet within the legitimate
  lever set
