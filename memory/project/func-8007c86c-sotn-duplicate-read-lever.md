---
name: func-8007c86c-sotn-duplicate-read-lever
description: func_8007C86C (sibling of 8007C7A0, constant 0xE4 vs 0xE3) — session 4 (2026-06-02 post 161c6c3 SOTN-aligned policy) — applying C7A0's session-D Y wide-mask duplicate-read precompute (now ALLOWED) drops honest pure-C floor 20 → 12, mirroring C7A0's session-D result. Same structural ceiling (missing target-idx-16 move a3,v0 park-merge insn → $a2-vs-$a3 X-preserve register cascade). Stays parked — 21 rules calibrated for HEAD form; new lever doesn't retire them.
metadata:
  type: project
---

# func_8007C86C — SOTN duplicate-read lever lowers honest floor 20 → 12

## Status

**INCOMPLETE — re-parked 2026-06-02 (fourth retry session, post-SOTN-policy-update).**
Prior sessions:
- Session 1 (commit `b1ea9da`): score 20 committed (HEAD form with 21 rules
  + `int new_var; int new_var2; if (!D_8009BE7A){}` padding).
- Session 2 (commit `f57b7dd`): clean v8 form (`var_a1 = arg1;`
  unconditional + strip padding) drops floor 20 → 17. NOT committable
  (doesn't retire rules). See [[func-8007c86c-clean-form-floor]].
- Session 3 (`1f07d5a`): clean single-target permuter ~5400 iters; ALL
  closing forms UB-conditional-init (forbidden cheat-by-spelling). Honest
  floor stayed 17. See [[func-8007c86c-permuter-ub-cheat-findings]].
- **Session 4 (this, 2026-06-02): apply C7A0 session-D's SOTN duplicate-read
  lever to C86C. Floor 20 → 12 (same as C7A0).**

The 21 regfix `subst`/`insert`/`delete` rules in `regfix.txt:3073-3094`
remain. Function stays matched-with-rules at HEAD; the new lever lowers the
honest pure-C floor but does not retire any rule.

## The new lever — SOTN-allowed Y wide-mask duplicate-read precompute

Today's policy update (commit `161c6c3`) made the **duplicate-read into
branch arms** technique ALLOWED based on direct SOTN master-branch evidence
(`src/dra/42398.c` `color_fake = *palette;` repeated rebinds). Applied to
C86C, the lever stacks with session 2's clean v8 base + session C's r_e4
named const (and drop dead-code padding):

```c
s32 func_8007C86C(s16 arg0, s16 arg1)
{
    s16 var_v0_2;
    s16 var_a1;
    s32 var_v0;
    s32 var_v1;

    var_a1 = arg1;                                /* session 2's v8 preload */
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
    var_v1 = var_a1 & 0xFFF;                      /* SESSION 4: precompute Y wide-mask */
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        var_v1 = var_a1 & 0x3FF;                  /* overwrites in narrow-mode branch */
        var_v1 = var_v1 << 0xA;
        var_v0 = var_v0_2 & 0x3FF;
    } else {
        var_v1 = var_v1 << 0xC;                   /* uses precomputed wide-mask */
        var_v0 = var_v0_2 & 0xFFF;
    }
    {
        s32 r_e4 = var_v0 | 0xE4000000;
        return var_v1 | r_e4;
    }
}
```

`sandbox --disable all`: 20 → **12**. `build_insns` 50 (target 51, 1 short
— the missing park insn).

### Mechanism (mirror of [[func-8007c7a0-sotn-duplicate-read-lever]])

Target's mode dispatch emits `andi $v1, $a1, 0xFFF` in the bnez **delay slot**
(idx 37). With `var_v1 = var_a1 & 0xFFF;` precomputed BEFORE the dispatch:

- The wide-mode branch READS the precomputed value (`var_v1 << 12`)
- The narrow-mode branch OVERWRITES it (`var_v1 = var_a1 & 0x3FF`)
- The precompute itself sits in the bnez delay slot — matching target exactly

This is the canonical SOTN duplicate-read pattern (now ALLOWED per
161c6c3).

## Index-aligned diff vs target at score 12

Same shape as C7A0 (target uses `$a3` for X preserve; mine uses `$a2`):

| idx | mine (score 12)        | target              | issue |
|---|---|---|---|
| 0  | `move a2,a0`            | `move a3,a0`         | $a2 vs $a3 (X preserve) |
| 8  | `move v1,v0`            | `move a2,v0`         | $v1 vs $a2 (X-limit save) |
| 12 | `addiu a0,v1,-1`       | `addiu v0,a2,-1`     | dest/src cascade |
| 14 | `move a0,a2`           | `move v0,a3`         | mirror |
| 15 | `move a0,zero`         | `move v0,zero`       | mirror |
| 16 | (MISSING)              | `move a3,v0`         | THE PARK INSN |
| 22 | `move v1,v0`           | `move a0,v0`         | $v1 vs $a0 (Y-limit save) |
| 26 | `addiu a1,v1,-1`       | `addiu a1,a0,-1`     | mirror |
| 41 | `andi a0,a0,0x3ff`     | `andi v0,a3,0x3ff`   | mask cascade |
| 43 | `andi a0,a0,0xfff`     | `andi v0,a3,0xfff`   | mask cascade |
| 44 | `lui v0,0xe400`        | `lui a0,0xe400`      | const dest |
| 45 | `or v0,a0,v0`          | `or v0,v0,a0`        | operand swap |

12 diffs total. Identical structural shape to C7A0's session-D result; the
only difference is the constant (0xE4000000 vs 0xE3000000).

## Variants tested this session (all measured negative vs the score-12 base)

| Variant | Score | Notes |
|---|---|---|
| baseline (committed HEAD, 21-rule form) | 20 | reference |
| **clean v8 + SOTN duplicate-read precompute** | **12** | session 4's new floor (this lever) |
| + var_v0_2 self-rebind in dispatch (`var_v0_2 = var_v0_2 & MASK; var_v0 = var_v0_2;`) | 12 | DCE-folded; var-reuse SOTN technique no-op here |

The forbidden score-10 form documented in C7A0 session D's memory
(`s16 raw_arg0 = arg0;` literal rename + reassignment) was NOT tested this
session per [[no-new-park-categories]] / [[param-local-alias-prologue-pair-flip]]
— same form would apply here with same forbidden verdict.

## The remaining structural ceiling (identical to C7A0)

The missing park insn at idx 16 + the X-preserve register choice ($a2 mine
vs $a3 target) cascade through ~12 register-rotation diffs. cc1's default
ascending allocation order ($a2=6, $a3=7) picks $a2 for the X preserve when
both are free. Target picks $a3. Every C lever tested for C7A0 (named
intermediates, xlim/ylim explicit reads, declaration-order swaps, operand
flips, duplicate-read combinations) AND the prior C86C session 2 + 3 ledger
+ this session's variant — none flip this tiebreaker without invoking a
forbidden literal-rename / dead-conditional-store / param-alias cheat.

## Why this is NOT committable (per persistence contract)

The session-4 source change does NOT retire any regfix rules. Test:
`& tools/eng.ps1 build` with the score-12 form produces SHA1
`947ff79e6f88ce0627f3bcf5520d2ed0abe02909` — mismatch with oracle
`62efab4f73f992798c43e8c730aa43baa10bb4fa`. The 21 substs would need
rewriting to handle the new cc1 emission shape. Net = lateral movement,
no rule reduction, no completion progress.

`park_src_guard` correctly blocks `park:` commits modifying build files.
Source reverted to HEAD; commit only the memory note.

## Concrete next-step hypotheses for the next session

The score-12 form is a true lower bound for legitimate (non-cheat) pure-C
in this function as of 2026-06-02 policy. The score-10 form (raw_arg0
chain) is forbidden cheat-by-spelling.

UNTRIED legitimate levers:

1. **`tools/decomp-permuter` from the score-12 base** (different from
   session 3's permuter which started from the score-17 clean v8 base).
   Directed at the X-preserve register slot. CRITICAL: vet output against
   cheat catalog AND invoke `cheat-reviewer` per
   [[review-discipline-before-commit]]. Reject any closing form using
   literal renames, dead conditional stores, or other catalog patterns.
2. **ALLOCDBG-instrumented dump on the score-12 form** to confirm WHY cc1
   picks $a2 over $a3 for the X preserve. The instrumented cc1
   (`tmp/gccdbg/cc1`) from prior sessions can dump allocno priorities. A
   specific livelen/refs imbalance might point at a structural change
   that's not yet flagged as a cheat.
3. **Sibling cross-reference** — `func_8007C748` (3-arg variant, matched
   pure-C) — diff its `.greg` dump to identify what natural C structure
   made its var_v0_2-analog land in the target reg.
4. **m2c-reconstructed structure** of func_8007C86C target.s — read it more
   carefully for the X-clamp shape. m2c's output might suggest a
   control-flow shape that doesn't require literal renames.

## Sibling impact

`func_8007C7A0` (regfix.txt:3050-3071) has IDENTICAL 21-rule pattern (0xE3
vs 0xE4 differs). Both sit at floor 12 with the same SOTN duplicate-read
lever applied. Both stay parked per the same persistence-contract reason.
See [[func-8007c7a0-sotn-duplicate-read-lever]] for the sibling's
session-D evidence ledger (which led to the same floor + same ceiling).

## Files / how to reproduce

- Baseline (committed, score 20): `src/display.c:561-601` (HEAD m2c body
  with `int new_var/2` padding + `if (!D_8009BE7A){}` empty-if)
- Session 2's score 17: drop padding + `var_a1 = arg1;` preload (clean v8
  form)
- **This session's score 12**: clean v8 form + r_e4 named const + add
  `var_v1 = var_a1 & 0xFFF;` BEFORE the mode dispatch (precompute), then
  `var_v1 = var_a1 & 0x3FF;` (overwrite) in the narrow-mode branch
- Verify score 12: apply the lever-set, then `& tools/eng.ps1 sandbox
  func_8007C86C --disable all` → `"score": 12`
- Full build SHA1 with this form: `947ff79e6f88ce0627f3bcf5520d2ed0abe02909`
  (NOT oracle — rules don't accommodate new shape)

## Related

- [[func-8007c7a0-sotn-duplicate-read-lever]] — sibling C7A0 session D
  memory; this session ports the same lever to C86C with the same
  numerical result and identical structural ceiling
- [[func-8007c86c-clean-form-floor]] — session 2's memory note (clean v8
  floor 17); this session adds the SOTN duplicate-read lever on top
- [[func-8007c86c-permuter-ub-cheat-findings]] — session 3's permuter
  exhaustion (UB-conditional-init only)
- [[split-read-defeats-hoist]] — the SOTN duplicate-read-into-branch-arms
  technique used here (now ALLOWED per 2026-06-02 borderline-research,
  commit 161c6c3)
- [[no-new-park-categories]] — register-rotation infrastructure is NOT an
  acceptable carve-out; function stays parked as unfinished work
- [[param-local-alias-prologue-pair-flip]] — explicitly forbids the
  `raw_arg0 = arg0` literal-rename pattern that hits score 10
- [[review-discipline-before-commit]] — the cheat-reviewer gate that would
  catch the score-10 form as FAIL
- [[difficult-is-not-impossible]] — cardinal rule. 20 → 12 is real
  progress; the matching C structure exists, just not yet within the
  legitimate lever set
