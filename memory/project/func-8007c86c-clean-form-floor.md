---
name: func-8007c86c-clean-form-floor
description: func_8007C86C (sibling of 8007C7A0, constant 0xE4 vs 0xE3) — clean-form rewrite drops sandbox floor 20 → 17, 1 below sibling C7A0's 18 floor (NOT committable, doesn't retire rules)
metadata:
  type: project
---

# func_8007C86C — clean-form rewrite hits sandbox floor 17 (1 below C7A0 sibling's 18)

## Status

**INCOMPLETE — re-parked 2026-06-01.** Prior session committed C86C with
score 20 (51 insns). This session's clean rewrite — strip `int new_var;
int new_var2; if (!D_8009BE7A) { }` dead-code padding, m2c-shape body with
`var_a1 = arg1;` unconditional at entry, single fused `var_v0 = ... & 0xFFF`
expression in the dispatch — measures **sandbox 17 (50 insns)**.

C7A0 (the sibling with constant 0xE3 instead of 0xE4) is at floor 18 with
the equivalent `var_a1 = arg1` lever (commit `e98fdbb`,
`memory/project/func-8007c7a0-arg1-preload-lever.md`). C86C reaches 17 with
the cleaner C form — 1 less masked diff because the dead-code-padding-and-
ceremony was DCE'd anyway (`int new_var2 = var_v0_2;` is just a same-type
alias coalesced into var_v0_2's pseudo; `if (!D_8009BE7A) { }` empty body
DCE'd before flow.c).

Same structural ceiling persists: the missing target-`move a3,v0` "park
merge" insn that GCC cannot be coerced to emit while var_v0_2 has a
single coalescable pseudo lifetime.

## The clean v8 form (sandbox 17, NOT committable)

```c
s32 func_8007C86C(s16 arg0, s16 arg1) {
    s16 var_a1;
    s16 var_v0_2;
    s32 var_v0;
    s32 var_v1;

    var_a1 = arg1;                       /* the [[goto-end-...]] / C7A0 v8 lever */
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
    var_a1 = var_a1 & 0xFFF;
    if ((u32) (D_8009BE74 - 1) >= 2U) {
        var_v1 = (var_a1 & 0x3FF) << 0xA;
        var_v0 = var_v0_2 & 0x3FF;
    } else {
        var_v1 = var_a1 << 0xC;
        var_v0 = var_v0_2 & 0xFFF;
    }
    return var_v1 | (var_v0 | 0xE4000000);
}
```

`sandbox --disable all`: 20 → **17**. `build_insns` 51 → 50 (one short of
target's 51). The 17 vs sibling C7A0's 18 is incidental masked-diff
arithmetic: same structural state, fewer normalised differences only
because the cleaner C form doesn't carry the `int new_var2 = var_v0_2;`
explicit alias that creates one extra index-aligned register-rename.

## Variants tested this session

| Variant | Score | Build insns | Notes |
|---|---|---|---|
| committed baseline | 20 | 51 | the HEAD form (`int new_var/2` + `if (!D_8009BE7A) { }`) |
| HEAD + v8 lever (`var_a1 = arg1;` at top) | 18 | 50 | matches C7A0's v8 result |
| clean form (no padding) + v8 lever | **17** | 50 | this session's new floor |
| m2c-shape (unconditional `var_v0_2 = D_8009BE78 - 1` first, then conditional) | 25 | 52 | regression (extra sign-ext insns) |
| clean + s32 var_a1/var_v0_2 | 28 | 44 | wrong type — loses critical sign-ext insns |
| clean + `clamped_x = var_v0_2` (s16 block-local) | 17 | 50 | copy DCE'd — same as clean alone |
| clean + `clamped_x = var_v0_2` (s32 block-local) | 20 | 52 | s16→s32 sign-ext = +1 insn |
| clean + `new_var2 = var_v0_2;` (int) in both >=2U paths | 17 | 50 | DCE'd same-pseudo coalesce |
| clean + `u32 hi = 0xE4000000;` named const | 17 | 50 | constprop'd |
| clean + saved_lo = (u16) var_v0_2 | 18 | 50 | the (u16) cast added an andi insn |
| clean + arg1 > (D_8009BE7A-1) operand flip | 17 | 50 | GCC sees equivalent, no flip |
| clean + goto after_x form | 21 | 50 | goto restructure regressed |

## The remaining structural ceiling — the missing park insn

Mirror of C7A0's gap. Target's idx 16 has `move a3, v0` (park clamped
arg0 from $v0 into $a3). My sandbox at every variant tested coalesces
var_v0_2's pseudo into the merge-pseudo, so cc1 lands the merge value
directly in whichever reg it picks for var_v0_2 — no copy needed, 1
fewer insn.

Every C-source attempt to force the simultaneously-live pseudo split has
been coalesced by GCC's copy propagation. The C97C lever set (m2c shape +
goto-end + named intermediate) doesn't directly apply because:
- C97C had a clear single-entry-zero-exit (NULL check) shape
- C86C has 3-way branching on arg0 (bad / clamp_low / clamp_high) merging
  via fall-through, with NO early-return possible (target always falls
  through to the final OR-emit)

The 5 levers of C97C's COMPLETED-C match are tested here; none of them
transfer because the entry/exit shape is fundamentally different.

## Concrete next-step hypotheses (untried this session)

1. **Permuter from clean-v8 base.** The minimal isolated standalone build
   needs `cpp -nostdinc` carefully to avoid display.c's mass duplicate
   declarations; this session's setup attempt hit the duplicates and
   stalled. A clean permuter setup mimicking permuter/dbe4's would
   unblock a directed search.
2. **`PERM_GENERAL` directed permuter macros** to mutate the
   declaration order, OR-tree associativity, or break clauses across
   the if/else nest.
3. **Cross-reference matched siblings' declaration order.** Both
   func_8007C748 and func_8007C97C went through pure-C completion;
   their declaration-order patterns + named-intermediate choices
   might suggest analogous transforms for C86C.
4. **C7A0's "find a use of var_v0_2 that needs the value in $v0 first"**
   resume avenue — also applies here. No function calls exist; without
   one, the structural park is hard to motivate naturally.
5. **ALLOCDBG dump of the clean-v8 base.** Run `tmp/gccdbg/cc1` with
   `BB2_ALLOC_DEBUG=1` to extract the precise pseudo→hardreg map for
   this specific source, see which pseudo is missing the simultaneously-
   live constraint that would force the copy.

## Why this is NOT committable (per persistence contract)

The src change (clean v8 form) does NOT retire any regfix rules. The
function still matches via its 21 register-rename substs in
`regfix.txt:3060-3081`, just with a different cc1 emission shape. The
rules would need to be REWRITTEN for the new shape — which would NOT
retire them, just translate them laterally.

The `park_src_guard` hook (commit `3c85c55`) correctly blocks `park:`
commits that modify build files. src reverted to HEAD; only the memory
note is committed.

## Files / how to measure

- Baseline (HEAD): `src/display.c:564-604` (the m2c body with int new_var
  padding)
- v8 clean form (sandbox 17, NOT committed): apply the 41-line replacement
  documented above, then `& tools/eng.ps1 sandbox func_8007C86C --disable all`
- Sandbox vs target dump script: write `tmp/dump86c.sh` invoking
  `mipsel-linux-gnu-objdump -d tmp/sandbox/func_8007C86C/display.o` and
  `mipsel-linux-gnu-objdump -d build/src/display.o`, sed/sectioning the
  func_8007C86C body
- Prior C86C session memory: this is the FIRST C86C-specific memory note;
  C7A0's `memory/project/func-8007c7a0-arg1-preload-lever.md` is the
  sibling reference

## Related

- [[register-alloc-pure-c]] — Lever B (narrow type), Lever A (block-local
  var split); both tested negative for closing the missing park insn.
- [[goto-end-prologue-delay-slot]] — the C97C lever set; the entry shape
  here already gets prologue-in-delay-slot from `bltz $a0` naturally
  (no `goto end` restructure needed), but the park-merge insn isn't in
  this rule's surface.
- [[difficult-is-not-impossible]] — cardinal rule. 20→17 is honest
  progress; the matching C structure that produces the park insn still
  hasn't been found.
- [[no-new-park-categories]] — register-rotation infrastructure is NOT a
  carve-out; C86C stays parked as unfinished work.
- `memory/project/func-8007c7a0-arg1-preload-lever.md` — sibling's same-
  shape evidence ledger (the v8 lever IS the C7A0 finding from yesterday;
  C86C extends it with the dead-code-padding strip for 1 additional
  masked-diff reduction).
