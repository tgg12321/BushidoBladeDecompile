---
name: switch-vs-ifchain-branch-sense
paths: ["src/*.c", "regfix.txt", "asmfix.txt"]
description: "A multi-way dispatch carrying branch-sense-swap rules (bne<->beq + swapped j targets) on ONE case was hand-decompiled as an if-goto chain. GCC emits the inverted branch sense for the case nearest default and won't cross-jump its body into the shared merge label. Rewrite the if-goto chain as a real `switch` statement — GCC's switch decision-tree codegen picks the positive branch sense and cross-jumps the sibling case bodies on its own, retiring the rules."
metadata:
  type: recipe
---


# Branch-sense-swap rules on a dispatch func → rewrite the if-goto chain as a real `switch`

## Symptom

A function carries a small cluster of **branch-sense-swap** rules — a `bne`↔`beq`
flip paired with two `j` targets swapped — on ONE case of a multi-way dispatch:

```
# func_XXX: ==6 branch sense swap (bne->beq, swap j targets)
func_XXX: subst "bne\s+\$3,\$2,\.L355" "beq\t$3,$2,.L351" @ 96   # regfix.txt
func_XXX: subst "j\s+\.L351"           "j\t.L355"          @ 98
func_XXX: replace_first "^bne\t\\$3,\\$2,\\.L354$" "beq\t$3,$2,.L350"   # asmfix.txt
func_XXX: replace_first "^j\t\\.L350$"             "j\t.L354"
```

`canonical` routes `C`, `diagnose` says **CONTROL-FLOW**, and the cheat-free
`sandbox --disable all` distance is tiny (1). objdump shows the only real diff is
one case's polarity: target `beq $v1,$v0,<set_block>` (positive, with the value
`li` in the delay slot, fall-through `j <default>`), your build `bne $v1,$v0,<default>`.
Every other diff is just a whole-function address shift from cheat-asm stripping.

## Cause

The function's body was hand-decompiled as an **if-goto chain** that mimics GCC's
switch decision tree (e.g. an explicit `if (x < 5) ...` range split with per-case
`if (x == K) { ...; goto set_val; } goto default;`). GCC compiles each such block
as "test, skip-block-if-false (`bne`), block, jump" — so the case nearest `default`
comes out with the **inverted** branch sense and does **not** cross-jump its body
into the shared merge label the way the target does.

GCC's real `switch` codegen instead emits a canonical balanced decision tree:
positive `beq ==K → <shared merge>` with the common assignment folded into the
delay slot, and sibling case bodies (`var = C; break;`) cross-jumped together.
That is exactly what the target is.

## Fix

Replace the if-goto chain with the actual `switch` statement it was reconstructing.
The merge label (`set_val:`) becomes the post-switch statement; each `goto set_val`
becomes `break`; the early-return case keeps its `return`:

```c
switch (D_800A38DC) {
case 4:  var_v0 = 0xC; break;
case 1:
    if (D_800A3748 == 0) { func_8001DA2C(); D_800A3768 = 2; mottest_disp(); return; }
    var_v0 = 0xC; break;
case 6:  var_v0 = 0xC; break;
default: func_8001DA2C(); var_v0 = 2; break;
}
D_800A3834 = var_v0;   // was `set_val:`
```

GCC then picks the decision-tree pivot (`beq ==4`, `slti <5` split, `beq ==1` low /
`beq ==6` high) and the correct positive branch sense for every case on its own —
**no regfix/asmfix needed.** `retire` drops the rules; full SHA1 == oracle.

## Confirmed case — func_8001EFA0 (code6cac.c, 2026-05-26)

Queue top, verdict C, distance 1, 4 rules (2 regfix subst + 2 asmfix replace_first,
all the `==6` `bne↔beq` + `j` swap). The body was an if-goto chain with an explicit
`if (D_800A38DC < 5)` split and `goto set_val`/`goto default_case` labels. Rewriting
it verbatim as `switch (D_800A38DC) { case 4 / case 1 / case 6 / default }` →
`sandbox --disable all` dropped 1→0; `retire` dropped all 4 rules; SHA1 == oracle.
100% pure C.

## When this does NOT apply

- The diff is a register rename, not a branch polarity flip → see
  [[register-alloc-pure-c]] / [[sandbox-zero-retire-fails]].
- target uses an actual jump-table `lw`/`addu`/`jr` (not a `beq` comparison tree),
  and the table sits in `asm/data` rodata → [[jtbl-rodata-split-infrastructure]]
  (park, don't grind).
- The dispatch is by function-pointer calls that over/under-merge → arg-count lever,
  [[cross-jump-call-merge]].

## Related
- [[shared-end-label]] — the inverse: a real `switch` whose per-case `return`s
  constant-fold; there you ADD a shared end label. Here you REMOVE the hand-rolled
  gotos and restore the `switch`.
- [[cross-jump-call-merge]] — GCC's cross-jumping (jump.c) is what merges the sibling
  `var = C; break;` bodies the if-goto chain kept separate.
- [[dead-branch-scheduling]] — another control-flow-shape codegen artifact.
