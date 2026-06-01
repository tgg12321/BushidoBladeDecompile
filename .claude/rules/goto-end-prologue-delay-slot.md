---
name: goto-end-prologue-delay-slot
paths: ["src/*.c"]
description: "Restructure `if (x == NULL) return 0;` as `goto end; ... end: return ret_val;` to make GCC emit the hand-coded-looking 4-insn entry pattern (bnez + delay-slot prologue + j + delay-slot zero)"
metadata:
  type: reference
---

# Goto-end-with-ret-val unlocks the 4-insn entry pattern (bnez + delay-slot prologue + j + delay-slot zero)

## Symptom

A small leaf function carries a 3-4 rule "prologue swap" cluster that rewrites
the cc1 entry sequence into the hand-coded-looking pattern target has:

```
# Target:
bnez $a0, .L_body          # branch if non-null (delay-slot follows)
addiu $sp, $sp, -0x10      # delay slot: prologue
j .L_end                    # else jump to epilogue
addu $v0, $zero, $zero      # delay slot: return value = 0
.L_body:
...
```

The cheats look like (4 rules):

```
func_X: subst "subu\s+\$sp,\$sp,16" "bnez	$4,.LC_BODY" @ 0
func_X: subst "beq\s+\$4,\$0,\.L\d+" "subu	$sp,$sp,16" @ 1
func_X: subst "addu\s+\$2,\$4,\$zero" "addu	$2,$0,$0" @ 2
func_X: insert "j	.LE_END" @ 2
func_X: insert_label ".LE_END:" @ 29
func_X: insert_label ".LC_BODY:" @ 3
```

Naive `if (arg0 == NULL) return 0;` gives cc1's natural emission
`addiu sp,-16; beqz $a0, .L_ret0; ... body ... .L_ret0: addu $v0, zero, zero;
... epilogue` -- prologue first, branch second, separate return-zero block.
That's 2-3 extra instructions vs target's 4-insn layout AND uses the inverted
branch sense (beqz instead of bnez).

## Cause

cc1 emits the function prologue (the `addiu $sp,$sp,-N`) at a fixed position
at the function head -- `function_prologue` runs after RTL gen and is NOT
schedulable by `sched.c` or `reorg.c`. So no C-level structure can move the
prologue per se.

BUT cc1's `reorg.c` (delayed-branch pass) CAN fill the delay slot of an
early-branch IF the branch's delay-slot candidate is the FIRST instruction of
the function. The prologue IS that first instruction. So if the source
generates a forward branch (`bnez`) RIGHT AFTER the prologue, with the branch
target inside the function body, reorg fills `bnez`'s delay slot with the
prologue's `addiu`. The prologue THEN "becomes" the delay slot.

Similarly, if the early-exit emits a `j` to the epilogue label with a `move
$v0, $zero` IMMEDIATELY after (in source order), reorg fills the j's delay
slot with the move.

## The fix -- `goto end` with a `ret_val` accumulator

Replace the early-exit `return 0;` with a `goto end;` to a shared epilogue
label, accumulating the return value in `ret_val`:

```c
s32 func_X(...) {
    s32 ret_val;
    /* declare locals... */

    if (arg0 == NULL) {
        ret_val = 0;
        goto end;
    }
    /* body computes ret_val */
    ret_val = ...;
end:
    return ret_val;
}
```

This makes cc1's RTL emit:
1. Function head + prologue (always runs).
2. `bnez $a0, .L_body` (branch to body if non-null).
3. `j .L_end` (else jump to epilogue with v0=0).
4. The body's `j .L_end` at end.
5. Epilogue (.L_end label = `addiu sp,+N; jr $ra; nop`).

`reorg.c` then:
- Fills `bnez`'s delay slot with the prologue's `addiu $sp,-N` (the only
  in-block instruction before the branch, eligible because `bnez` is annul-false
  / unconditional in the body-thread).
- Fills the first `j`'s delay slot with `addu $v0, $zero, $zero` (cc1's
  `move v0, 0` for the goto's RTL).

The result: 4-insn entry sequence matching target byte-for-byte. The
`addu $v0, $zero, $zero` (assembled from `move $v0, $zero`) is identical to
target's `addu $v0, $zero, $zero`.

## Why GCC threads vs preserves the goto

GCC's `jump.c` may try to thread the `goto end` (collapse it back to a direct
`return 0`). It does NOT thread when:
- The end label is a SHARED merge from multiple paths (body's final code path
  also goes through `end:`).
- The ret_val accumulator is a real auto variable (not an `int` literal).

The shared end label is the load-bearing element -- without it, `goto end`
collapses to `return 0` and the body-path return-via-end disappears.

## Confirmed case -- func_8007C97C (display.c, 2026-06-01)

Queue parked item ("global-label-drift-sibling-cheat + 4-reg-cluster
coupling"). Retired 9 rules: 4 entry-pattern substs + 2 `srl` to `sra` substs + 1
operand-register subst + 2 `insert`/`insert_label` directives.

The combined lever set was:
1. **This rule** -- `goto end` form for the entry-pattern (closes the 4-insn
   entry diff).
2. **[[register-alloc-pure-c]] Lever B** -- `u32 temp_a1; u32 temp_v0;`
   (unsigned types for the byte-load chains) so `r >>= 3` / `g >>= 3` emit
   `srl` (matching target). The signed type yields `sra` which the rules were
   forcing.
3. **m2c-style structure** -- m2c (`tools/m2c/m2c.py`) on the target asm
   produced a single-statement OR-return expression. Adopting it (vs the
   pin-driven step-by-step source) freed the 4-pseudo register-rotation
   cluster.
4. **Named intermediate for OR association** -- `u32 r_e2 = (temp_a1 << 0xA)
   | 0xE2000000;` forces the const-OR to materialize FIRST (in $a1), then
   accumulate into $v0. Without the intermediate, GCC reassociates and OR's
   the const into $v0 first.
5. **Reassignment of temp_v0 for inline scheduling** -- `temp_v0 = temp_v0 <<
   0xF;` as a separate statement (vs `(temp_v0 << 0xF)` in the return expr)
   places `sll $v0, $v0, 15` adjacent to the prior `sw $v0, 4(sp)`, matching
   target's idx-18 position. Folding it into the return expression delays
   the `sll` until after the b2 load, regressing scheduling.

Sandbox `--disable all` went 24 -> 17 (m2c-shape) -> 12 (goto-end) -> 5 (OR
reorder) -> 2 (named r_e2) -> 0 (temp_v0 reassignment). `retire` dropped all 9
rules; full SHA1 == oracle; `queue done` -> COMPLETED-C.

The function was parked with: "the matching C exists but the explored lever
space (if-form, type, order, scope) does not contain it." The novel lever
combination -- m2c-shape + goto-end + named intermediate + reassignment
shift-scheduling -- was not in the prior session's enumeration. The lever
proves [[difficult-is-not-impossible]]: the unseen C structure existed.

## When this rule does NOT apply

- Functions where the early-exit returns a NON-ZERO constant -- `goto end`
  with `ret_val = K` will work, but the j's delay slot is filled with the
  constant materialization (`li $v0, K`), which may differ from target.
- Functions with multiple early-exit conditions -- they don't all need the
  goto-end form; only the ones whose entry-region the regfix paperwork
  was reshaping.
- Functions where target has the early-exit AFTER the prologue (rare --
  target's prologue is always first; this rule applies when target appears
  to have the prologue IN a delay slot, which means reorg performed the fill
  in the original compilation too).

## Related
- [[register-alloc-pure-c]] -- the broader RA-via-C-structure playbook; this
  is a scheduling-via-C-structure variant
- [[difficult-is-not-impossible]] -- the cardinal rule this case study proves
- [[store-before-jal]] -- sibling delay-slot-fill-from-C-structure case
- [[shared-end-label]] -- relatedly forces a multi-path merge for return
  computation; different goal (defeating per-case const-fold) but same
  mechanism (shared end label as a synchronization point)
- [[global-label-drift-sibling-cheat]] -- the cheat-class the 9 retired
  rules belonged to; the parked-with-sibling-coupling state this rule unlocks
