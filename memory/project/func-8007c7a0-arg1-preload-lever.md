---
name: func-8007c7a0-arg1-preload-lever
description: func_8007C7A0 — sandbox 20 → 18 via unconditional `var_a1 = arg1` preload (new lever, novel beyond prior session's 19-22 floor); still NOT COMPLETED-C, 1 missing park-merge insn structural ceiling persists
metadata:
  type: project
---

# func_8007C7A0 — arg1-preload lever lowers honest floor from 20 to 18

## Status

**INCOMPLETE — re-parked 2026-06-01.** Prior session (commit `bc65a49`,
2026-05-22) explored ~30h: 23 attempts logged, 52k+ permuter iters, and
documented the structural floor 19-22 for semantically-correct pure C.
This session pushed the floor to **18** via a novel one-line restructure
the prior session did not explicitly test, but the matching C structure
that produces target's full byte sequence remains unreached.

The 21 regfix `subst` rules in `regfix.txt:3037-3058` remain the
prescribed register-rename handling for an unreachable RA tie. The
function stays matched-with-rules.

## The new lever — arg1 preload (sandbox 20 → 18, NOT committable)

Moving `var_a1 = arg1;` from inside the m2c body's `else { var_v0_2 = 0;
var_a1 = arg1; }` block to be **unconditional** at the top of the function:

```c
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 var_a1;
    s16 var_v0_2;
    int new_var;
    s32 var_v0;
    int new_var2;
    s32 var_v1;

    var_a1 = arg1;                     /* LEVER: moved from inside else */
    new_var = arg0 >= 0;
    if (new_var) {
        if ((D_8009BE78 - 1) < arg0) {
            var_v0_2 = D_8009BE78 - 1;
        } else {
            var_v0_2 = arg0;
        }
    } else {
        var_v0_2 = 0;
        /* var_a1 = arg1; removed from here */
    }
    /* ... rest unchanged ... */
}
```

`sandbox --disable all`: 20 → **18**. `build_insns` 51 → 50 (one short
of target's 51). This is **new** measured progress beyond the prior
session's 19-22 floor for clean+correct C.

## Mechanism

Preloading `var_a1 = arg1` unconditionally extends `var_a1`'s live range
to span the entire function from entry. This changes GCC's allocator
decisions:

- Before (m2c body): `var_a1` was assigned ONLY in the negative-arg0
  path, so its live range starts at `.L103` (the bltz target). GCC
  treats arg1 ($a1) as a short-lived parm pseudo, dies quickly.
- After (preload): `var_a1` is live from entry through y_clamp. arg1's
  parm pseudo (in $a1) gets coalesced into var_a1's long-lived pseudo.

This shifts which register holds the raw-arg0 preserve:
- Before: raw arg0 → $a3 ($7), clamp result → $a0 ($4) or $a3 ($7)
- After: raw arg0 → $a2 ($6), clamp result → $a0 ($4) ← register usage rotated

The diff vs target stays at 18 because target's bytes are also 51
(target has 1 extra `move a3,v0` "park" insn at .L8007C7E0 that GCC
doesn't emit when it directly accumulates the clamp result into
arg0's register).

## The remaining structural ceiling — the missing park insn

Using `tmp/diff_v8.py` against `tmp/sandbox/func_8007C7A0/display.o` vs
`build/src/display.o`, the precise diff is:

```
idx | ours (v8, score 18)         | target                       | gap
  0 | move a2,a0                  | move a3,a0                   | raw preserve reg
  4 | addiu sp,sp,-24             | addiu sp,sp,-16              | frame size
  8 | move v1,v0                  | move a2,v0                   | maxX preserve reg
 12 | addiu a0,v1,-1              | addiu v0,a2,-1               | result reg
 14 | move a0,a2                  | move v0,a3                   | else result
 15 | move a0,zero                | move v0,zero                 | neg path reg
 16 | (missing — next is arg1 sx) | move a3,v0                   | THE PARK MERGE
 ... cascade ...
```

Target builds clamp result in `$v0` and **parks it into $a3** at
.L8007C7E0 via `move a3,v0`. GCC cannot be coerced to emit this park
because once it knows the clamp result is assigned to var_v0_2 (whose
lifetime equals var_v0_2 -> the long-lived destination), it allocates
that destination directly as the in-register clamp accumulator.

To force the 2-step pattern (build in $v0, park into $a3), the C
source needs simultaneously-live pseudos for the temp and the
destination — but every variant tested (block-local `tmp`, separate
named locals, dead-store after assignment) gets coalesced by GCC's
copy-propagation. The remaining gap is exactly this missing park insn
plus its cascade through the rest of the function.

## Variants tested this session (all ≥ 18, none reach 0)

| Variant | Score | Build insns | Notes |
|---|---|---|---|
| baseline (m2c body) | 20 | 51 | the committed form |
| **v8: var_a1 = arg1 unconditional** | **18** | **50** | NEW lever, this session |
| v1: full goto-end restructure | 24 | 50 | shape divergence |
| v2: var_v0_2=arg0 init + goto | 28 | 50 | regression |
| v3: explicit raw_x + maxX_save locals | 20 | 51 | CSE'd, no change |
| v4: maxX/maxY locals | 20 | 51 | CSE'd, no change |
| v5: W1 form (var_v0_2=arg0 init) | 20 | 51 | prior session's W1 |
| v7: separate `s16 raw = arg0;` | 20 | 51 | CSE'd |
| v8 (best, retained as lever) | 18 | 50 | this finding |
| v9: v8 + var_v0_2=arg0 init | 22 | 50 | regression |
| v10: v8 + new_var3 bool extraction | 18 | 50 | no change |
| v11: arg1<0 inverted clamp | 25 | 51 | regression |
| v12: v8 + dead D_8009BE7A read | 18 | 50 | DCE'd |
| v13: block-local tmp + outer sx | 18 | 50 | tmp coalesced into sx |

## Why this is NOT committable (per persistence contract)

The src change (v8) does NOT retire any regfix rules. The function still
matches via its 21 register-rename substs, just with a slightly
different cc1 emission shape. `regfix.txt` would need its rules rewritten
to handle the new cc1 output's register positions. That rewrite would
**not** retire the rules — it would just translate them to the new
shape. Net: no rule reduction, no completion progress, just lateral
movement.

The `park_src_guard` hook (commit `3c85c55`) correctly blocks
`park:` commits that modify build files. Reverted; commit only the
memory note.

## Concrete next-step hypothesis for the next session

The structural ceiling is the missing `move a3,v0` park insn. To break
it would require:

1. **Find a C form where var_v0_2 has 2 conflicting pseudos that cannot
   be coalesced.** Tested in v13 (block-local `tmp` inside the if,
   outer `sx` for storage); GCC coalesces. Possible escape: make
   `tmp` aliased by another live variable (an address-of? but that's
   forbidden cheat-class per dead-vars-local-array).
2. **Find a use of `var_v0_2` that "needs" the value in $v0 first.**
   Like a function call where var_v0_2 is the return value, then
   subsequent code expects it in a callee-save. There are no calls in
   this function, so this lever doesn't apply directly.
3. **Examine the matched sibling `func_8007C748`** (commit `35720d9`,
   completed with var_v0 + arm-1 if-else). It has similar shape but
   simpler structure (3 args, no clamp+park pattern). Compare its
   regfix history — was there ever a similar register-renaming wall
   that fell out? Worth a focused diff.
4. **`func_8007C2A0` matched yesterday** via the param-local-alias
   prologue-pair-flip rule. That rule does not apply here (no
   prologue pair to flip in C7A0), but the underlying technique
   (declaration-order-driven register allocation) deserves more
   targeted exploration. Specifically: declaring `var_a1` BEFORE
   `var_v0_2` in the locals list (forcing GCC's pseudo-creation
   order to favor arg1).

## Files / how to measure

- Baseline (committed, score 20): `src/display.c:523-562` (the m2c body)
- v8 (NEW lever, score 18, NOT committed): apply the 2-line diff
  documented above, then `& tools/eng.ps1 sandbox func_8007C7A0 --disable all`
- ALLOCDBG harness: `tmp/c7a0_iso2.c` + run via
  `wsl bash -c 'cd ... && BB2_ALLOC_DEBUG=1 tmp/gccdbg/cc1 ... > /tmp/dbg.txt'`
- Diff tool: `tmp/diff_v8.py` (index-aligned objdump comparison)
- Prior session's full evidence: `.bb2_attempts/func_8007C7A0.jsonl` (a1-a23),
  `docs/func_8007C7A0_HANDOFF.md`

## Related

- [[register-alloc-pure-c]] — the playbook applied (Lever B narrow
  type, Lever A block-local var split); both tested negative for
  closing this function's residual.
- [[goto-end-prologue-delay-slot]] — the technique that worked on
  func_8007C97C; tested here (v1, v2) but regressed (the func_8007C7A0
  body has 2 clamp blocks not 1 entry-exit pattern).
- [[param-local-alias-prologue-pair-flip]] — the technique that
  worked on func_8007C2A0; not applicable here (no prologue
  save+def pair to flip).
- [[difficult-is-not-impossible]] — the cardinal rule. This session's
  18 vs prior's 19-22 confirms the floor is NOT a hard ceiling — but
  the matching C structure still hasn't been found.
- [[no-new-park-categories]] — register-rotation infrastructure is
  NOT an acceptable carve-out; this function stays parked as
  unfinished work, not as a "new category."
