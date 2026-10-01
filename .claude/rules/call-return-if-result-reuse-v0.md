---
name: call-return-if-result-reuse-v0
description: "Two-constant if/else select on a call result lands in $v1/$a0 with flipped branch sense: init the result var FROM the call return and test ==0, so it stays in $v0 through bnez+delay-slot."
paths: [".claude/rules/call-return-if-result-reuse-v0.md"]
metadata:
  type: reference
---

# Initialize the if-else result var from the call return to land it in `$v0`

## Symptom

Target: `bnez $v0,.L; addiu $v0,$zero,A; addiu $v0,$zero,B; .L: subu rD,$v0,rS`. Yours: the
mirror image — `beqz`, the two constants swapped, and the result in `$v1`/`$a0`, for C like

```c
if (call() != 0) base = A; else base = B;
dist = base - other;
```

Small distance (3-4), all diffs in the if-else emission. GCC ends `$v0`'s live range at the branch
and gives the fresh result pseudo the next free register.

## The fix

Initialize the result with the call return, then conditionally overwrite, testing `== 0` (swap
then/else bodies to keep semantics):

```c
s32 base = call();
if (base == 0) base = B;    /* fall-through */
else           base = A;    /* taken branch; fills the delay slot */
dist = base - other;
```

`base` starts in `$v0` (the call return) and the branch tests the same register, giving the target's
merge form. This is idiomatic C (caching a call return before testing it), not a coercion.

Example: `tslLineG5Init` (code6cac_c2.c) — block-local `base` (4→4), swap branch sense (4→3), init
from `game_GetPlayerCount()` (3→0).

## Does NOT apply when

- the result flows immediately into `$a0`/`$a1` for a call (destination chosen early);
- the call return has a second use besides the test;
- the select is not a two-constant select.

Related: [[register-alloc-pure-c]] · [[restore-discarded-return-displaces-v0]] · [[store-before-jal]]
