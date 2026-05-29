---
name: loop-note-fixes-delay-slot-steal
paths: ["src/*.c"]
description: "A `__asm__ volatile(\"\" ::: \"memory\")` barrier that only stops a delay-slot steal retires by writing the polling loop as a real while/do loop (front-end NOTE_INSN_LOOP_BEG flips reorg's branch prediction)."
metadata:
  type: reference
---

# A scheduling barrier that only blocks a delay-slot STEAL → write the loop as a real `while`/`do`

## Symptom

A function carries one tier-3 `__asm__ volatile("" ::: "memory")` barrier and
**zero** regfix/asmfix rules. `canonical` routes `C`; `sandbox --disable all`
(which strips the barrier) reads a small non-zero distance (e.g. 2). An
index-aligned objdump of `build/` (barrier present) vs the stripped sandbox
object shows the only real diff is **one instruction's position**: a cheap
single-insn op at the *target* of a forward conditional branch (commonly
`move $sN,zero` = a loop-counter init) has been **stolen into that branch's
delay slot**, shifting the branch target by +4 and cascading the rest.

The barrier sits right at the branch target, before the stolen instruction, with
a comment like "prevent GCC from hoisting `s0=0` into earlier delay slot."

## Cause — `dbr` mispredicts a goto-formed loop-exit branch

The stolen op lives at the head of the branch-*taken* block. GCC's delayed-branch
pass (`reorg.c`) steals from the **target** thread only when `mostly_true_jump`
predicts the branch **taken**. For a forward conditional branch that is actually a
**loop exit**, the target compiler predicts it **not-taken** (you usually stay in
the loop) and fills the slot from the **fall-through** instead — leaving the
counter-init at the block head where the target has it.

The lever: `mostly_true_jump`'s loop-exit heuristic keys off
**`NOTE_INSN_LOOP_BEG`**, which the C **front-end** emits only for real
`for`/`while`/`do` statements — **NOT** for loops built from `goto`. A goto-formed
poll loop has no loop note, so reorg never sees the branch as a loop exit,
mispredicts it taken, and steals the target op into the slot. A prior agent papered
over this with the `__asm__ volatile("" ::: "memory")` barrier (an un-stealable
first insn at the target) instead of fixing the loop form. The barrier is tier-3
debt ([[inline-asm-tiers]]); the sandbox strips it, so it can't move the score —
`queue done` refuses it.

## Fix — express the polling as genuine nested `while`/`do` loops

Rewrite the `label: ... goto label;` control flow as real loop statements so the
front-end emits the loop notes. Keep the block **layout** identical to the target
(the exit block stays *after* the in-loop fall-through path) — use `break` for the
forward loop-exit branch and fall-through / `continue` for the back-edge:

```c
/* goto form (no loop note -> dbr steals s0=0 into the bnez delay slot): */
poll_loop:
    if (vsync() != 0) goto got_vsync;          /* forward loop-exit branch */
    if (timer() - base < 0x7801) goto poll_loop;
    return 0;
got_vsync:
    s0 = 0;                                     /* STOLEN into the bnez slot */
    do { ... } while (s0 < 1000);
    if (s0 < 1000) goto poll_loop;

/* real-loop form (front-end NOTE_INSN_LOOP_BEG -> bnez seen as loop exit,
   predicted not-taken, slot filled from fall-through; s0=0 stays put): */
    while (1) {                                 /* outer: re-poll wrapper */
        while (1) {                             /* inner poll loop */
            if (vsync() != 0) break;            /* -> loop-exit (bnez fwd) */
            if (timer() - base >= 0x7801) return 0;
        }
        s0 = 0;
        do { ... } while (s0 < 1000);
        if (s0 >= 1000) break;                  /* success -> exit outer */
    }                                           /* else continue outer = re-poll */
```

The inner `while(1)` makes the vsync `break` a recognized loop-exit; the outer
`while(1)` replaces the `goto poll_loop` re-entry (a goto back into a loop is a
multi-entry loop and won't get a clean note). No barrier, no rules.

## Confirmed case — func_8003A450 (code6cac_c_mid.c, 2026-05-28)

Queue top, verdict C, distance 2, **0 rules**, one `__asm__ volatile("" ::: "memory")`
barrier guarding `s0 = 0;` at the `got_vsync` label. The `.dbr` dump showed the
`bnez v0 -> got_vsync` (forward, single-pred target) with `(insn/s ... (set (reg 16
s0) 0))` stolen into its delay slot. Goto-formed poll loop → no `NOTE_INSN_LOOP_BEG`
→ mispredicted taken. Rewriting both poll loops as nested `while(1)` (vsync = inner
`break`, re-poll = outer loop, retry = `do/while`) → `sandbox --disable all` 2→0;
`verify-oracle --rebuild` SHA1 == oracle. 100% pure C, barrier removed.

## Diagnosis (decisive)

1. `canonical` → C, one barrier, 0 rules, sandbox distance small.
2. objdump `build/src/<file>.o` vs `tmp/sandbox/<func>/<file>.o`, index-aligned
   (`tmp/norm_diff.py`): the sole real diff is a single-insn op moved into a forward
   branch's delay slot (+4 target shift). The `jal`-name diffs are relocation noise.
3. Confirm with the `.dbr` dump (`cc1 <flags> -dd a.i`): find the `(jump_insn ...
   (label_ref <target>))` and check whether the next slot insn is `insn/s` stolen
   from the target block.
4. Check the loop is goto-formed (no `for`/`while`/`do`). If so, this rule applies.

## Related
- [[sandbox-zero-retire-fails]] — sibling "barrier + rules, sandbox 0, retire fails";
  there the barrier causes a register shuffle the *rules* undo. Here there are no
  rules — the barrier only blocks a delay-slot steal, fixed by loop *form*.
- [[inline-asm-tiers]] — the `__asm__ volatile("" ::: "memory")` barrier is tier-3 debt.
- [[store-before-jal]] — another "C statement structure drives delay-slot scheduling".
- [[switch-vs-ifchain-branch-sense]] — another "restore the real C control structure
  (switch / loop) and GCC's codegen falls out" case.
