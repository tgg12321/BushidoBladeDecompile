---
name: loop-exit-work-inside-loop-sched-fence
description: "Next construct's inits hoisted above/into a post-loop store region: move the loop's one-time exit work inside the loop (`if (c) continue; tail; break;`) so the LOOP_END note lands mid-block and fences sched."
paths: [".claude/rules/loop-exit-work-inside-loop-sched-fence.md"]
metadata:
  type: reference
---

# Move the loop's exit work INSIDE the loop so its LOOP_END note fences sched

## Symptom

Target's post-loop region is strictly source-ordered with a genuine load-delay `nop`:

```mips
lw   $v0, 0x0($a2)     # tail word copy
nop                    # REAL stall
sw   $v0, 0x0($a3)
sw   $t4, 0x6C($t3)    # checksum store
move $a1, $zero        # next loop's inits, AFTER the stores
move $a0, $t1
```

Your build hoists the next loop's init moves above the tail copy and into the `lw` delay slot.

## Mechanism (GCC 2.7.2 sched.c)

- sched.c:2067-2095: a `NOTE_INSN_LOOP_BEG/END` in the MIDDLE of a block is a hard scheduling fence
  ("no instructions are scheduled across it").
- `schedule_block` (~sched.c:3233) skips LEADING notes: a loop note at a block head is inert.

With `do { copy } while (src != end); tail; checksum;` the LOOP_END lands at the head of the
post-loop block, so the inits float up. Respell so the exit work sits inside:

```c
for (;;) {
    *dst = *src;
    src++;
    dst++;
    if (src != end) continue;
    *(s32 *)dst = *(s32 *)src;     /* tail copy on the exit path */
    break;
}
*(s32 *)(base2 + 0x6C) = checksum;
```

Same control flow after jump1, but the note now lands mid-block between the tail `sw` and the
checksum store, pinning everything after it in source order.

**Depth caveat:** statements moved inside get loop-depth-weighted `reg_n_refs`, which can perturb
global RA. Move the MINIMUM exit work inside — just enough that the note lands after the insns that
must precede the fence.

**Companion:** `dst = (Quad *)(offset + (s32)base)` — casting the pointer to integer keeps source
operand order in the `addu` (pointer+int canonicalizes the pointer first);
[[compare-operand-order-register]] family.

Every statement is real (the actual copy loop); owner-sanctioned 2026-06-11 (func_80037F40, 7 → 0).

## Does NOT apply when

- a natural true dependency could order the insns instead;
- there is no real adjacent loop — never manufacture a degenerate loop for its note;
- the exit work is large / touches many outer pseudos (depth weighting).

Related: [[switch-break-shared-return-sched-hoist]] · [[loop-note-fixes-delay-slot-steal]] ·
[[do-while-zero-exception]] · [[register-alloc-pure-c]]
