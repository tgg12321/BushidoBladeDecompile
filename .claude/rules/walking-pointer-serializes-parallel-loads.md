---
name: walking-pointer-serializes-parallel-loads
paths: [".claude/rules/walking-pointer-serializes-parallel-loads.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "Memory-clobber barriers between independent parallel-array element stores (G0=r[0]+a[0]; barrier; G1=r[1]+a[1]; ...) retire by walking both arrays with post-increment pointers (*rp++ + *ap++) — the pointer dependence serializes the loads so GCC keeps the target's per-element lw/lw/nop/addu/sw and stops stealing later loads into the load-delay slots."
metadata:
  type: reference
---

# Walking pointers serialize parallel-array loads (retire a delay-slot memory barrier)

## Symptom

A function carries one or more `asm volatile("" ::: "memory")` barriers (cheat-asm)
and **zero** regfix/asmfix rules, sitting between a run of **independent
parallel-array element computations stored to separate globals**:

```c
D_G0 = result[0] + a0[0];
asm volatile("" ::: "memory");      /* CHEAT */
D_G1 = result[1] + a0[1];
asm volatile("" ::: "memory");      /* CHEAT */
D_G2 = result[2] + a0[2];
```

`canonical` routes `C`; `sandbox --disable all` (which strips the barriers) reads a
small non-zero distance (≈ 3 × the element count). An index-aligned objdump of
`build/` vs the stripped sandbox object shows the ONLY real diff is in this block:
the target emits a strict **per-element** sequence with a genuine load-delay `nop`

```
lw v0,Nr(sp)    ; result[i]
lw v1,Na(s2)    ; a0[i]
nop             ; load-delay (v1 not yet available)
addu v0,v0,v1
sw  v0,Gi
```

while your stripped build **hoists the later elements' loads** (`lw a1,4(s2)`,
`lw a2,8(s2)`) up to fill the delay slots and uses extra registers — collapsing
3 nops and shaving ~3 insns.

## Cause

The element computations are independent (different stack slots `result[i]`,
different pointer offsets `a0[i]`, different global destinations). GCC's scheduler
proves no aliasing, schedules all the loads together, and fills each `lw`'s
load-delay slot with the next element's load. The prior agent papered over this
with memory barriers (an un-reorderable fence between elements) instead of
restoring the source shape that makes GCC serialize naturally. The barrier is
cheat-asm ([[inline-asm-policy]]); the sandbox strips it, so it's score-inert and
`queue done` refuses it.

## Fix — walk both arrays with post-increment pointers

Rewrite the block to consume both arrays through **walking pointers**:

```c
{
    s32 *rp = result;
    s32 *ap = a0;
    D_G0 = *rp++ + *ap++;
    D_G1 = *rp++ + *ap++;
    D_G2 = *rp++ + *ap++;
}
```

Each `*ap++` post-increment threads a data dependence through the pointer, so GCC
can no longer hoist element i+1's load ahead of element i's store. It emits the
loads one element at a time, leaves the genuine load-delay `nop`, and reuses
`v0`/`v1` per element — matching the target byte-for-byte. This is idiomatic
parallel-array C (a human walking two arrays writes exactly this); it is **not** a
coercion device (no volatile, no asm, no dead store) — it is net anti-cheat
(removes the barriers, adds zero rules).

## Confirmed case — func_80046BF4 (sound.c, 2026-06-14)

Stage/camera init, 112 insns, 0 rules, two `asm volatile("" ::: "memory")` barriers
around `D_80101E3C/40/44 = result[i] + a0[i]` (3 elements). Honest distance 10; the
entire diff was this block (GCC hoisted the a0[1]/a0[2] loads into the delay slots,
−3 insns). Swept 5 block forms (operand swap, per-element temp block, walking
pointers, inline reorder); **only the walking-pointer form scored 0**. Applied,
`verify-oracle --rebuild` SHA1 == oracle, `queue done` COMPLETED-C, integrity OK.
The per-element-temp and operand-swap forms did NOT help (≥10) — the post-increment
pointer dependence is the load-bearing part, not just "split into statements".

## Confirmed case — func_8006517C (text1b.c, 2026-06-14; rule adopted 2026-06-15)

Extends the pattern to a **re-read** cluster + an **interleaved const halfword
store**. Body reads p[0..2] -> globals, then RE-READS p[0],p[1] -> more globals,
then chases p=p[2]. HEAD cheated with `register volatile s32 *p asm("$3")` +
`register s32 t asm("$2")` pins (the `volatile` was a CSE-defeat coercion) + 3
regfix rules (reorder + two $4->$2 subst). Two pure-C extensions made it score 0:

- **A SECOND walking pointer for the re-read batch.** `s32 *ap=p, *bp=p;` — `ap`
  walks the first three reads, `bp` walks the re-reads, `p=(s32*)*bp` chases. GCC
  re-reads naturally (the intervening global stores block CSE — NO volatile
  needed; the volatile was pure cheat). One walking pointer alone left the re-read
  batch in scattered regs (dist 8); the second pointer serialized it. (Direct
  refinement of the base technique — a re-read is just another parallel-load
  cluster.)
- **Explicit temp to drop an independent const store into a LOAD-delay slot.**
  The `D_800F0BC0 = 0;` (a real, always-required `sh zero`) had to land BETWEEN
  the `p[2]` load and its store (target fills the load-delay slot with it).
  Writing `t = *ap; D_800F0BC0 = 0; D_800F0D38 = t;` — load into a named temp,
  then the independent const store, then the global store — places `sh zero`
  exactly there. Inlining as `D_800F0D38 = *ap;` with `BC0=0` before/after
  scheduled the `sh` one slot off (dist 4-5).

`retire` dropped all 3 rules, SHA1 == oracle, COMPLETED-C.

**Why the explicit-temp bullet is legitimate (not a codegen-steer-by-spelling).**
It is the **load-delay-slot analog of the already-sanctioned
[[defer-store-past-later-compute-into-jal-delay]]** technique: hoist the stored
value into a named local, then move the `GLOBAL = val;` store statement AFTER an
**independent, already-required** operation, so the scheduler fills the
(jal- there, load- here) delay slot. The decisive legitimacy test is the same:
the interleaved op (`D_800F0BC0 = 0;`) is a **real instruction the function
already needs** — NOT a manufactured/dead store inserted to steer scheduling.
Both the load-into-temp and the statement-reorder have a plain reading a human
would write; only the ORDER is chosen, and it is semantics-preserving (distinct
globals). Contrast the FORBIDDEN func_80062020 epilogue (same session), which
wrote the **same lvalue two different ways** purely to force two addressing modes
— that has no semantic content and IS a steer. Independent cheat-reviewer +
user sign-off 2026-06-15 (the technique was NOT shipped in the func_8006517C
match commit; held for review per [[review-discipline-before-commit]] rule 2).

## Confirmed case — func_800613C8 (text1b.c, 2026-06-14)

Same mechanism, but the cheat was **`register s32 t asm("$2")` pins** (not memory
barriers) on a `D_800F1140/44/48 = a0[i]` parallel-store cluster (a "ChangeXxx"
sprite-setup sibling family — func_800611A4/61250/6133C/61454 all share the pin
pattern). Honest full-build diff (the masked sandbox score is unreliable here —
register pins make `--disable all` read a flat 12; build the .o and diff with
`engine.score.normalized_insns(..., mask=False)` instead): walking pointers
(`s32 *ap=a0; D=*ap++;`) dropped 13→5. The residual 5 was an **independent
constant store** (`D_800A3464 = 0x8080FF;`) that GCC hoisted its `lui`
materialization too early; **moving that store to AFTER the last `*ap++` load**
(it's a different global, so reorder is semantics-preserving) let GCC schedule
the `lui`/`ori` into the later loads' delay slots → score 0, SHA1 == oracle,
COMPLETED-C. Lesson: after walking-pointers serialize the loads, an interleaved
independent constant/global store may still need to move PAST the loads so its
materialization lands in the freed delay slots.

## When it applies / when it doesn't

- Applies to **straight-line** independent parallel-array element writes guarded by
  per-element memory barriers, OR the same cluster cheated with per-load
  `register asm("$N")` pins. The walking-pointer dependence is what serializes.
- If the block is a **loop** whose barrier blocks a delay-slot *steal* off a
  loop-exit branch, that's a different mechanism — see
  [[loop-note-fixes-delay-slot-steal]] (fix the loop *form*, not pointer walking).
- A plain `{ s32 t = r[i] + a[i]; G = t; }` per-element temp does NOT serialize (the
  loads are still independent) — measured ≥ the barrier-stripped distance.

## Related
- [[loop-note-fixes-delay-slot-steal]] — sibling "barrier-only-blocks-a-delay-slot"
  case in LOOP form (front-end loop note flips branch prediction).
- [[store-before-jal]] — another "C statement structure drives delay-slot scheduling".
- [[inline-asm-policy]] — the `asm volatile("" ::: "memory")` barrier is cheat-asm.
