---
name: scratchpad-gte
paths: ["src/text1b.c", "src/display.c"]
description: "Recipe for matching functions that compute into PS1 scratchpad (0x1F800xxx) then run a GTE op (mvmva/op/etc) on the result. Combines inline-asm sw/lw absolute-address idiom + register-pinned base-pointer + 4-rule compound regfix to schedule lui+ori for $a3 inline with the last diff section. Originally the project's recipe doc said \"bridge stays\" for these; that's wrong, they match."
metadata:
  type: recipe
---

## Symptom

A function that:
1. Performs s32 arithmetic on inputs and stores results to scratchpad RAM
   addresses `0x1F8003xx` (e.g. `out[i] = a1[i] - a0[i]`)
2. Loads values from scratchpad as a base+offset block (e.g. `lw $t5, 0($t4);
   lw $t6, 4($t4); lw $t7, 8($t4)` after `addu $t4, $a3, $zero`)
3. ctc2/mtc2/lwc2/swc2 to set up GTE registers
4. Runs a GTE op (mvmva, op, nclip, etc.) — emitted as `.word 0x4...`
5. swc2 results back to scratchpad
6. Maybe a function call followed by reads from another scratchpad probe

The recipe doc historically classified these as "bridge stays" (per the
old `feedback_quick_reference.md:563`), claiming maspsx chokes on hex
offsets like `sw $val, 0x360($1)`. **That's wrong as of 2026-05-12** —
maspsx handles `sw $X, 0x1F800360` (1 maspsx line) by expanding to
`lui $at, 0x1F80; sw $X, 0x360($at)` cleanly during the as pass.

## C body recipe (the easy parts)

```c
s32 fn(s32 *a0, s32 *a1, s32 *a2) {
    s32 *va, *vb, *out;
    s32 ret;
    /* Diff stores: inline-asm `sw %0, 0xABS_ADDR` is the canonical scratchpad
     * write idiom (origin: code6cac.c:417). Natural-C
     * `*(volatile s32*)0x1F800360 = expr` would emit lui+ori+sw 0(reg) =
     * 3 insns; inline-asm sw absolute = 2 insns (lui+sw lo($at)). */
    {
        volatile s32 *va0 = (volatile s32 *)a0;
        volatile s32 *va1 = (volatile s32 *)a1;
        volatile s32 *va2 = (volatile s32 *)a2;  /* volatile casts defeat CSE
                                                    on the source loads — target
                                                    re-reads a0[N] every diff */
        register s32 v0 asm("$2");                /* pin v0/v1 so subu uses */
        register s32 v1 asm("$3");                /* the same regs as target */
        s32 d;
        v0 = va1[0]; v1 = va0[0]; v0 = v0 - v1;
        __asm__ volatile ("sw %0, 0x1F800360" :: "r"(v0));
        /* ... 5 more diffs ... */
    }
    /* Matrix coef load: pin source ptr to $a3 + move to $t4 + base+offset
     * lw's, matching target's `lui $a3; ori $a3; addu $t4, $a3, $zero;
     * lw $t5, 0($t4); lw $t6, 4($t4); ctc2; lw $t7, 8($t4); ctc2; ctc2`. */
    va = (s32 *)0x1F800360;
    {
        register s32 *q asm("$7") = va;          /* pin to $a3 */
        register s32 *mp asm("$12");             /* pin scratch to $t4 */
        register s32 t5 asm("$13");
        register s32 t6 asm("$14");
        register s32 t7 asm("$15");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(q));
        t5 = *mp;
        t6 = *(mp + 1);
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        t7 = *(mp + 2);
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }
    /* Vec load + GTE op + result store: same q/mp pin trick */
    /* ...  */
}
```

## The 4 regfix rules that close the gap (the hard part)

Pure-C even with all the pins above gets to ~32 byte diffs. The remaining
gap is in **instruction scheduling**: target schedules `lui $a3; ori $a3`
INTERLEAVED with the last subu/sw of the diff section (so $a3 is live
across the gap to the matrix-load `addu`); cc1 schedules them right
before the addu. **No C-level trick coerces this scheduling** — register
pin variants oscillate between 32-44 diffs. Use compound regfix:

```
# 1+2: Interleave lui+ori for $a3 with the last diff (move them earlier)
fn: delete @ <nop_idx>
fn: reorder <new_csv> @ <range>

# 3+4: Swap the order of `lw $a1, ABS` and `lw $a0, 0($v0)` before the jal
fn: subst "lw \$5, 0xABS"   "lw $4,0($2)" @ <maspsx_idx_of_lw_a1>
fn: subst "lw\s+\$4,0\(\$2\)" "lw $5, 0xABS" @ <maspsx_idx_of_lw_a0>
```

For cpu_check_tubazeri specifically:
```
cpu_check_tubazeri: delete @ 29
cpu_check_tubazeri: reorder 31,32,29,30 @ 29-32
cpu_check_tubazeri: subst "lw \$5, 0x1F800388" "lw $4,0($2)" @ 55
cpu_check_tubazeri: subst "lw\s+\$4,0\(\$2\)" "lw $5, 0x1F800388" @ 56
```

The reorder math: original maspsx idx 29-33 = `[nop, subu, sw, lui $7, ori $7]`.
After delete @ 29 (nop), idx 29-32 = `[subu, sw, lui, ori]`. Reorder
`31,32,29,30 @ 29-32` puts `[lui, ori, subu, sw]` — matching target's
inline-with-diff scheduling.

## Why the swap rules need to be `subst`, not `reorder`

Target wants `[lw $a0, lui $a1, lw $a1, jal, nop]`. Mine source-order
puts arg1 first → `[lui $a1, lw $a1, lw $a0, jal, nop]`.

Naive `reorder 56,55 @ 55-56` (swap the two lws) **breaks**: the post-reorder
order is `[lw $a0, lw $a1, jal, nop]`. With `.set reorder` (default),
**`as` then auto-fills the jal delay slot with `lw $a0`** (the immediately
preceding instruction), producing `[lw $a1, jal, lw $a0 in delay]`.
This is semantically valid (delay slot runs before the call) but doesn't
match target's `[lw $a0, lui $a1, lw $a1, jal, nop]` byte sequence.

Two `subst` rules that **swap the line text in place** work because they
keep the lines in the same source positions but change which is `lw $a0`
vs `lw $a1` — `as` then sees the same surrounding instructions and
doesn't promote one into the delay slot.

This is a general lesson: **prefer `subst` over `reorder` when the
reorder would put a movable instruction immediately before a jal/branch**.

## Reference matches

- `cpu_check_tubazeri` (commit `bb6e989`, 2026-05-12): 4 regfix rules,
  full pure-C body. The first scratchpad+GTE function matched in the
  project — proof the family is tractable.

## When the recipe extends to other functions

The retirement queue has dozens of bridged functions that fit this
profile (search asmfix.txt for ones with 0x1F8003xx scratchpad accesses
or GTE ops). Apply the recipe per-function, adjusting:
- The `subst` indices to the function's specific maspsx output positions
- The `reorder` csv based on how many diffs precede the matrix-load setup
- The pin set if the function uses different working registers

## What does NOT work

- **m2c base.c is empty `void fn(void) { /* TODO */ }`** for these — m2c
  can't decompile the GTE+scratchpad combination. Don't waste time on
  the permuter (it permutes m2c's TODO, which is useless).
- **Pure-C without the move-to-$t4 pattern**: cc1 emits `lui+lw`
  absolute for each scratchpad load, not the base+offset target wants.
  Plateau ~32 diffs.
- **Adding `register asm("$N") = expr` at function scope** for the
  matrix pointer (cc1 keeps it alive too long, conflicts with
  caller-save register conventions). Only pin in the inner block.
- **`__asm__ volatile (".set noreorder") ... (".set reorder")` around
  the jal**: disrupts other parts of the function. Stick with the
  subst-swap approach above.
