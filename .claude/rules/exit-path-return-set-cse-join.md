---
name: exit-path-return-set-cse-join
description: "At a shared end label the op after `move v0,sN` reads $sN (cse canon_reg) and floats above the move: set the return value in EACH exit path instead; cross-jump re-merges the copies."
paths: [".claude/rules/exit-path-return-set-cse-join.md"]
metadata:
  type: reference
---

# Set the return value in EACH exit path so the join's op reads $v0

## Symptom

A shared finish label copies a callee-save into the return register and then uses it:

```c
done:
    ret = (s32)s2;          /* move v0,s2 */
    v1 = ret - (s32)s4;     /* target: subu v1,v0,s4 */
    *s5 = v1;
    return ret;
```

Target: `move v0,s2; subu v1,v0,s4`. Yours: `subu v1,s2,s4` scheduled above the move. (A dead
`ret++; ret--;` pair "fixing" this is a forbidden coercion.)

## Mechanism

With the copy inside the multi-predecessor label block, cse puts `v0` in `s2`'s quantity class and
`canon_reg` rewrites the subu operand to `s2`; the dependence vanishes and sched1 reorders.

## Fix — assign in each predecessor path

```c
    if (v1 == -2) { ret = (s32)s2; goto done; }
    ... loop ...
    ret = (s32)s2;
done:
    v1 = ret - (s32)s4;     /* reads ret's pseudo (allocated $v0) */
    *s5 = v1;
    return ret;
```

`done:` starts a fresh cse block, so the subu reads `ret`. jump2's cross-jump then merges the
identical `[move v0,s2; jump]` suffixes into one move before the label — same layout as before, but
with a true dependence. Each assignment is the function's real return value on a real exit path
(mixed-exit-forms family, [[cross-jump-store-tail-merge]]); unlike the forbidden goto-end accumulator
shape, `done:` holds real shared work.

## Applies when

1. Target's shared end has `move vN,sM` first and the next op reads `vN`; yours reads `sM` and/or
   swaps order;
2. 2+ exit paths converge on the shared end;
3. the copied value is genuinely the return value (or used after the join) — relocate a real
   assignment, never invent one.

Verify cross-jump re-merges the copies (insn count must NOT grow), then full SHA1. Example:
hirahira_w_frie (text1a_c.c).

Related: [[cross-jump-store-tail-merge]] · [[shared-end-label]] · [[cse-block-extension-controls-fold-span]]
