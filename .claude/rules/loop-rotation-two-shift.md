---
name: loop-rotation-two-shift
paths: ["src/*.c"]
description: "A bit-search/shift loop with an inline-asm initial `sllv` + register pins: target has the shift twice (peeled before the loop + recomputed in the bottom delay slot), a GCC loop-rotation artifact you can't hand-write. Use a natural for-loop (peeled shift is NOT folded), an opaque `one` var to defeat the `(x>>i)&1` bit-test transform, and the original explicit-goto post-loop order + mask-var reuse for branch sense/register."
metadata:
  type: recipe
---

# Two `sllv` from loop rotation — retire the inline-asm "initial mask" cheat

## Symptom

A bit-search / shift-loop function carries cheat-asm: register pins
(`register s32 mask asm("v0")` …) **plus** an inline `__asm__("sllv %0,%1,%2" …)`
that computes the *initial* mask before the loop. The target asm has the
shift **twice** — once before the loop (initial value) and once in the loop's
bottom delay slot:

```mips
sllv  v0,a2,v1        # mask = one << i   (INITIAL, i=0)
and   v0,a0,v0        # loop top:  arg0 & mask
bnez  v0, found
addiu v1,v1,1         # i++
slti  v0,v1,24
bnez  v0, <loop top>  # back-edge targets the `and`, NOT the first sllv
sllv  v0,a2,v1        # delay slot: recompute mask for next iter
```

This is GCC **loop rotation** (a.k.a. loop inversion): it peels the first
iteration's `mask = one << i` and recomputes at the bottom. The original C had
**no** inline asm — a prior agent injected the initial `sllv` because writing it
as straight-line C folds it away.

## Why straight-line C loses the instruction

Writing the initial mask explicitly before the loop:

```c
i = 0; one = 1;
mask = one << i;   /* i and one are known constants here → GCC folds to mask = 1 */
loop: ...
```

GCC constant-propagates `one=1, i=0` and emits `move`/nothing instead of `sllv`
→ one instruction short. An explicit-goto loop has the same problem: GCC respects
your single computation and just loops back to it (one `sllv`, back-edge to the
`sllv`). **You cannot hand-write the two-shift form** — it is a rotation artifact.

## The fix — let a natural `for` loop rotate

```c
bit_found = -1;
i = 0;
one = 1;                       /* opaque var (NOT literal 1) — see below */
for (; i < 0x18; i++) {
    mask = one << i;
    if (arg0 & mask) {
        bit_found = i;
        break;
    }
}
```

GCC rotates this and emits **both** `sllv`s — and the peeled first-iteration
shift is **not** constant-folded (rotation happens after the fold point). The
`break` also produces the right `found` block placement (set index, fall to the
post-loop check) without an explicit label.

## Two companion levers this function needed

1. **Defeat the single-bit-test transform with an opaque `one` variable.**
   `arg0 & (1 << i)` (literal `1`) is rewritten by GCC to `(arg0 >> i) & 1`
   → emits `srav; andi` instead of target's `li one,1; sllv; and`. Introduce
   `s32 one = 1;` and write `mask = one << i` — GCC can't prove `one` is a single
   bit, so it keeps the `sllv`+`and` mask form. (`1 << bit_found` *outside* the
   loop, used as a value, stays `sllv` and is fine as a literal.)

2. **Match the original statement order/var-reuse for register + scheduling.**
   The post-loop block layout (branch sense `bne`-to-work vs `beq`-to-skip,
   load scheduling, which register a value lands in) follows the **source
   statement structure**, not semantics. Restoring the original's explicit
   gotos (`if (x != -1) goto work; return -1; work: …; if(!flags) goto …`) and
   its **statement-by-statement** work block — including **reusing the same
   `mask` variable** for the masked load (`mask = g_spu_base_addr; … mask =
   1 << bit_found;`) so it allocates to the loop's register (`v0`) — took the
   masked sandbox score 6 → 0. A *separate* temp var for that load pushed it to
   `a0` (wrong reg). See [[register-alloc-pure-c]] (global RA), [[store-before-jal]]
   (statement order drives scheduling).

## Confirmed case — func_8008ACD0 (main.c, 2026-05-26)

Queue top, verdict C, distance 1, 0 regfix/asmfix rules but cheat-asm pins
(`a1/v1/a2/v0/a0`) + inline `sllv`. Rewrote as the natural `for` loop above +
opaque `one` + original goto post-loop → `sandbox --disable all` 1→…→0;
`verify-oracle --rebuild` SHA1 == oracle. 100% pure C, all 5 pins + the inline
asm removed. The grind: clean `for`+`break`+structured-`if` got the two `sllv`
and correct loop registers but wrong branch senses (GCC laid out structured
`if`/`return` differently); restoring the original's explicit-goto block order
+ `mask` var reuse closed the rest.

## Related
- [[inline-asm-policy]] — the inline `sllv` was cheat-asm, never a match
- [[register-alloc-pure-c]] — global RA; var live-range / reuse is the lever
- [[switch-vs-ifchain-branch-sense]] — branch sense follows C structure (sibling)
- [[store-before-jal]] — statement order → instruction scheduling
