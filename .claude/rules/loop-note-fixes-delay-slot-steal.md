---
name: loop-note-fixes-delay-slot-steal
paths: [".claude/rules/loop-note-fixes-delay-slot-steal.md"]
description: "A single-insn op at a forward branch target stolen into its delay slot in a goto-formed polling loop: write the loop as a real while/do (NOTE_INSN_LOOP_BEG flips reorg's branch prediction). Never a barrier."
metadata:
  type: reference
---

# Delay-slot steal in a goto-formed loop → write a real `while`/`do`

## Symptom

The only real diff is one instruction's position: a cheap single-insn op at the *target* of a forward
conditional branch (commonly `move $sN,zero`, a loop-counter init) is **stolen into that branch's delay slot**
in our build, shifting the branch target by +4 and cascading. The surrounding control flow is built from
`label: ... goto label;`. (Historically papered over with an `__asm__ volatile("" ::: "memory")` barrier —
cheat-asm, [[inline-asm-policy]]; the sandbox strips it.)

## Cause

reorg.c steals from the branch *target* only when `mostly_true_jump` predicts taken. Its loop-exit heuristic
keys off `NOTE_INSN_LOOP_BEG`, which the front end emits only for real `for`/`while`/`do` statements, never
for goto-formed loops. Without the note the loop-exit branch is mispredicted taken and the target op is stolen.

## Fix — genuine nested loops, same block layout

```c
while (1) {                                 /* outer: re-poll (replaces goto poll_loop) */
    while (1) {                             /* inner poll loop */
        if (vsync() != 0) break;            /* forward loop-exit branch */
        if (timer() - base >= 0x7801) return 0;
    }
    s0 = 0;                                 /* stays at the block head */
    do { ... } while (s0 < 1000);
    if (s0 >= 1000) break;
}
```

Keep the exit block after the in-loop fall-through path; use `break` for the forward exit and the outer loop
for re-entry (a goto back into a loop is multi-entry and gets no clean note). func_8003A450: 2 → 0.

## Diagnosis

Index-aligned objdump of ours vs target shows the single moved insn; `cc1 -dd` `.dbr` dump shows the
`(insn/s ...)` stolen into the `jump_insn`'s slot; the loop is goto-formed.

## Related

[[store-before-jal]] · [[switch-vs-ifchain-branch-sense]] (restore the real control structure)
