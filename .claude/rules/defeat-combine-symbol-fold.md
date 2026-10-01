---
name: defeat-combine-symbol-fold
description: "Displaced access through a symbol-derived pointer emits lui+sym-60 form (combine folded (plus symbol const)): pre-compute the displaced pointer into its own local before an intervening call."
paths: [".claude/rules/defeat-combine-symbol-fold.md"]
metadata:
  type: reference
---

# Pre-compute a displaced pointer to defeat combine's `(plus symbol const)` fold

## Symptom

`p = &D_xxxxxxxx` then a displaced store/load through `p` (`((s16 *)p)[-0x1E] = 1`). Your build
emits symbol-form addressing and loses the delay-slot fill:

```
li $2, 1
sh $2, D_800A15B4-60      <- maspsx: lui $at; sh -X($at)
jal func_X
addiu $16, $16, -60
```

Target: `li $2,1; jal func_X; sh $2,-60($16)`. Small honest distance (~4). Historically "fixed" by
an `asm volatile("" : "=r"(p) : "0"(p))` identity barrier — cheat-asm ([[inline-asm-policy]]).

## Why

`p`'s pseudo is single-set with a known `symbol_ref`; combine substitutes it into
`(mem (plus p -60))` and folds to `(mem (symbol_ref - 60))`, which maspsx must expand via `$at`; the
scheduler then fills the `jal` delay slot with something else.

## The fix — pre-compute the displaced pointer as its own local

Declare it at the top of the block, before an intervening call:

```c
s32 *s0b = &D_800A15B4;
s16 *flag_ptr = (s16 *)((char *)s0b - 60);   /* names the location written */

*s0b = (s32)s0b + 0xFDC;
bios_SetCustomExitFromException(s0b - 1);    /* call between def and use */
*flag_ptr = 1;                               /* sh $2,-60($16) in the jal delay slot */
```

`flag_ptr` names a real location the function writes — ordinary C with a semantic reading, not a
coercion (descriptive name, not `_pad`/`_dummy`). Example: func_80082C58 (ings2.c), 4 → 0, both
barriers removed.

## Applies when / not

- Applies: displaced access with a nonzero constant offset through a symbol-derived pointer, at
  least one call between definition and use, diff limited to addressing form + lost delay slot.
- Not: definition and use in the same block with no intervening call; the displaced access is the
  only use of the symbol; the diff shows other work besides the addressing form.

Related: [[store-before-jal]] · [[register-alloc-pure-c]] · [[cse-block-extension-controls-fold-span]]
