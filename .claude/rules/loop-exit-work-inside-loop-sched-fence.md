---
name: loop-exit-work-inside-loop-sched-fence
description: Post-loop inits hoisted above a tail store region — move the loop's one-time exit work inside the loop (if (cond) continue; tail; break;) so the LOOP_END note lands mid-block and fences sched
paths: ["regfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
---

# Post-loop inits hoisted above a tail store region — move the loop's exit work INSIDE the loop (`if (cond) continue; tail; break;`)

## Symptom

A function carries a small regfix cluster of the shape:

```
func: $X <-> $Y @ N            # operand-order swap on an addu
func: insert "nop" @ M         # load-delay nop the build lacks
func: reorder a,b,c,... @ R    # post-loop region reorder
```

Target's post-copy-loop region is strictly source-ordered with a genuine
unfilled load-delay slot:

```mips
lw   $v0, 0x0($a2)     # tail word copy load
nop                    # REAL stall — nothing filled it
sw   $v0, 0x0($a3)
sw   $t4, 0x6C($t3)    # checksum/result store
move $a1, $zero        # next loop's inits, in source order, AFTER the stores
move $a0, $t1
move $v1, $t1
.Lnext_loop:
```

Your cheat-free build instead HOISTS the three init moves above/into the
tail-copy region (two before the `lw`, one in its load-delay slot) — the
scheduler found them as independent fillers.

## Mechanism (verified in GCC 2.7.2 sched.c, func_80037F40 session)

Two sched.c facts combine:

1. **`sched.c:2067-2095`** — a `NOTE_INSN_LOOP_BEG` / `NOTE_INSN_LOOP_END`
   in the MIDDLE of a basic block is a hard scheduling fence: the first
   insn after the note gets a dependence on every prior use/set in the
   block, and `reg_pending_sets_all = 1` makes every later insn depend on
   it. Comment: *"we must be sure that no instructions are scheduled
   across it."*
2. **`schedule_block` (~sched.c:3233)** — leading notes are SKIPPED
   (`head = NEXT_INSN(head)` while head is a NOTE). A loop note sitting at
   the head of a block is INERT.

With the natural `do { copy } while (src != end); tail; checksum;` source,
the copy loop's `LOOP_END` note lands at the HEAD of the post-loop block →
inert → the next loop's init moves hoist freely (GCC 2.7.2 schedules each
block BACKWARD, preferring independent insns over starting a load's
latency chain, so the inits float up and fill the lw's delay slot).

Respelling the loop so its one-time exit work sits INSIDE the construct:

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

compiles to identical control flow (jump1 threads the `continue` to the
loop top and deletes the `break`'s jump-to-next, merging blocks) — but the
`LOOP_END` note now lands MID-block, between the tail-copy `sw` and the
checksum store. The fence pins the checksum store and the following inits
after the tail copy in source order, and nothing may move into the `lw`'s
load-delay slot → cc1 emits `lw; sw` adjacent and `as` inserts the target's
genuine `nop`.

## Register-allocation caveat (the depth-2 trap)

Loop depth weights `reg_n_refs`, which feeds the global allocno priority.
Statements moved inside the loop get their pseudos' refs depth-bumped.
In the confirmed case, moving BOTH the tail copy AND the checksum store
inside flipped a t2/t3/t4 three-cycle allocation rotation (checksum and
base2 refs got weighted up; score 7 → 8). Moving ONLY the tail copy inside
(checksum store back outside, after the loop) kept the fence (the note
falls between them) while leaving the outer pseudos at depth-1 weights →
score 0. **Move the MINIMUM exit work inside the loop — just enough that
the note lands after the instructions that must precede the fence.**

## Companion lever — integer-cast the address add to control addu operand order

Target `addu $a3, $t2, $t1` (offset first, base second) from
`dst = (Quad *)(offset + base)` where `base` is a pointer: pointer+int RTL
canonicalizes the POINTER first regardless of source order. Casting the
pointer to integer (`(Quad *)(offset + (s32)base)`) makes it a plain
integer add, which preserves source operand order. Same family as
[[compare-operand-order-register]] (RTL-emission-order lever).

## Why this is not a "cheats by any spelling" violation

Cheat-reviewer PASS (2026-06-11). Every statement in the respelled form
has real semantic content — the quad copy, the advances, the exit test,
the tail copy, the break; no dead code, no degenerate construct. "Loop
forever; copy; on reaching the end, pick up the remainder and stop" is an
idiomatic spelling a real programmer might write. Structurally the same
sanctioned family as [[switch-break-shared-return-sched-hoist]] (per-case
`return` → `break`, also a sched-pass-driven respelling of real control
flow) and the SOTN-accepted mixed-exit-forms family
([[cross-jump-store-tail-merge]]). Contrast [[do-while-zero-exception]]:
that sanctions a DEGENERATE wrapper (`do { } while (0)`) for one narrow
reorg.c interaction — this rule involves no degenerate construct; the loop
is the function's real copy loop, and its note is the natural consequence
of expressing a real loop.

## When this applies

1. Target has a strictly source-ordered post-loop store region with a
   genuine load-delay `nop`, and your build hoists later-statement init
   moves into/above it (the regfix cluster manufactures the order).
2. The hoisting insns are the NEXT construct's loop inits (or similar
   independent moves) in the same scheduling block.
3. The preceding construct is a real loop — so it has a `LOOP_END` note
   whose position you can control by placing the one-time exit work
   inside vs after the loop.

## When this does NOT apply

- The hoisted insns have true dependencies you could create instead —
  prefer a dataflow fix if one is natural.
- There is no loop adjacent to the store region (the note has to come
  from a REAL construct; do NOT manufacture a degenerate loop — that is
  the forbidden do-while-zero-out-of-scope shape).
- The exit work is large or touches many outer-scope pseudos — the
  depth-2 ref weighting will perturb register allocation (see caveat).

## Confirmed case — func_80037F40 (code6cac_c_mid.c, 2026-06-11)

Queue top, verdict C, distance 7, 3 regfix rules (`$9 <-> $10 @ 14`,
`insert "nop" @ 30`, `reorder 29,30,32,33,27,28,31 @ 27-33`). Save-slot
copy function: checksum loop over a 0x24-byte header, 3× (quad-copy loop +
tail word + checksum store + zero loop). The (s32) cast fixed the addu
operand order; the for(;;)/continue/break respelling moved the LOOP_END
note mid-block, pinning the zero-loop inits after the stores and
materializing the nop. Intermediate variant (checksum store also inside)
scored 8 via the depth-2 rotation; final form sandbox 7 → 0, retire
dropped all 3 rules, SHA1 == oracle, COMPLETED-C.

## Status: USER-SANCTIONED 2026-06-11 — with a process-violation record

This rule was originally committed BY THE WORKER in the same commit as its
confirmed case (89bfc882) — a violation of review-discipline hard rule #2
(no self-sanctioning rule docs; new technique families are held for user
sign-off, NOT committed alongside the match). The layer-2 retroactive audit
FAILed the commit on that process ground. The USER then evaluated the
technique itself via the standing three-question test and SANCTIONED it
2026-06-11 (mechanism: target's strictly source-ordered post-loop bytes
require the LOOP_END fence mid-block, which requires the exit work
lexically inside the loop; natural spelling measured 7, this 0; the
respelling carries real semantics in every statement — the actual copy
loop, not a degenerate wrapper). The commit stands. The (s32) addu
operand cast is covered by the same sanction (compare-operand-order
family). The process violation stands recorded in
review-discipline-before-commit.md — sanction of the technique does NOT
excuse the pathway.

## Related

- [[switch-break-shared-return-sched-hoist]] — sibling sanctioned
  control-flow respelling driven by the same scheduler (there: per-case
  return → break; here: post-loop exit work → inside-loop exit path).
- [[loop-note-fixes-delay-slot-steal]] — sibling loop-note rule for
  reorg.c's `mostly_true_jump` (goto-loop → real while); this rule is the
  sched.c fence side of the same note machinery.
- [[do-while-zero-exception]] — the narrow degenerate-wrapper exception;
  NOT what this rule uses; its warning against bending other passes with
  `do{}while(0)` stands.
- [[compare-operand-order-register]] — the RTL operand-order family the
  (s32)-cast companion lever belongs to.
- [[register-alloc-pure-c]] — the allocno-priority background for the
  depth-2 ref-weighting caveat.
