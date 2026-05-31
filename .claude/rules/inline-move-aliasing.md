---
name: inline-move-aliasing
paths: ["src/*.c"]
description: "How to force the `addu RD, RS, $zero` aliasing pattern when GCC collapses `register asm` pins into direct addressing. Recurring shape in GTE wrappers."
metadata:
  type: reference
---


> **Critical distinction before reading further:** the technique below uses
> `%0` / `%1` operand placeholders bound to `register T x asm("$N")` C
> variables via `=r` / `r` constraints. **The asm template never contains
> hardcoded `$N` register references.** If you read this memory and write
> `__asm__ volatile("addu $8, $3, $zero")` (no `%N`, hardcoded GPRs), you
> have replicated the lost_codegen regfix cheat in C source — see
> [[inline-asm-injection]]. The audit will block it.

> **RETIRED AS AN END STATE (2026-05-21).** Per [[completion-standard]], an
> `inline_move_aliasing` block is **cheat-asm — never a committed match.** The
> old "escape valve with an `INLINE_MOVE_ALIASING:` comment + `Pure-C attempts:`
> block" allowance is **gone.** You may still use this idiom **diagnostically** —
> to observe which base/destination registers and ordering the target wants — but
> the committed result must reproduce that through **C structure**, with the
> `__asm__("move ...")` and the `register asm` pins **removed.** Everything below
> explains the pattern target emits and *why* GCC folds it: treat it as a map of
> the gap you must close in pure C, not a snippet to ship. (GTE wrappers keep
> their `ctc2`/`mtc2`/`lwc2` ops — canonical inline asm — but the register *setup*
> around them must be pure C.)

# Required form (auditor-enforced)

```c
/* INLINE_MOVE_ALIASING: pure-C alternatives failed.
 *   - technique=plain_copy: `T dst = src;` -- GCC CSE collapsed per regalloc dump
 *   - technique=volatile_copy: `volatile T dst = src;` -- sw+lw, wrong shape
 *   - technique=intermediate_xor: `T dst = src ^ 0;` -- folded
 * Per feedback_inline_move_aliasing.md, single-insn escape valve.
 */
register T dst asm("$RD");
register T src asm("$RS");
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));
```

The `INLINE_MOVE_ALIASING:` header is detected by `audit_asm_cheats.py`
within 8 lines above the asm block. Bullets must be `- technique=<name>:
<outcome>` with ≥2 present. Each outcome must reference observable state
(regalloc dump output, specific GCC behavior, dump-text content) — vague
outcomes are caught by the LLM auditor and rejected.

The same techniques/outcomes must also appear in the commit message's
`Pure-C attempts:` block (≥3 entries).

# The pattern target emits but GCC won't reproduce from clean C

Many already-matched functions (and ALL the GTE wrappers in `src/text1b.c` / `src/code6cac.c`) contain sequences shaped like:

```mips
addiu $v0, $sp, 0x10        # v0 = &mat[0]
addu  $t4, $v0, $zero       # t4 = v0   (== `move $t4, $v0`)
lw    $t5, 0($t4)            # use t4 as base register
lw    $t6, 4($t4)
ctc2  $t5, $0
...
```

The middle line `addu $t4, $v0, $zero` is the assembler form of `move $t4, $v0` — a redundant register-to-register copy. It exists because the original C had a *separate pointer variable* that got materialized into one register, then copied into another (typically a $t-reg) for the subsequent indexed loads.

# Why naïve C doesn't reproduce it

```c
s32 *mp = mat;             // attempt 1: plain ptr
m0 = mp[0];                // GCC: `lw $t5, 0x10($sp)` — collapsed
```

GCC's optimizer sees that `mp` is just `&mat[0]` and that the only uses are `mp[N]` indexed loads, so it folds everything to direct sp-relative addressing. No `addu $t4, $v0, $zero`; just `lw $t5, 16($sp)`. **Same bytes, different alignment** — every subsequent diff cascades from this single missing instruction.

Adding `register s32 *mp asm("$12") = mat;` doesn't fix it either. GCC honors the pin **only if `mp` is materialized as a register elsewhere** (e.g., passed to an inline asm "r" constraint). With only `mp[N]` accesses, GCC's view is still "no need to materialize, fold into sp-relative."

# The technique

Use a single-instruction inline asm `move` to force the redundant copy:

```c
{
    register s32 *mp asm("$12");
    s32 *p = mat;                                            // ① in some $v reg
    __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(p));    // ② forces $t4 = $v?
    m0 = *mp;                                                // ③ uses $t4 as base
    m1 = *(mp + 1);
    /* ... */
}
```

What this does:

- The `s32 *p = mat;` line forces GCC to compute `&mat[0]` into some scratch register (typically `$v0`). Without it, the `move` asm would collapse `mp = mat;` to a single `addiu`.
- The `__asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(p))` forces a register-to-register copy `mp <- p`, materializing both into distinct physical registers. The `"=r"(mp)` with the `register asm("$12")` declaration pins the destination to `$t4`.
- After this, all `*mp` / `*(mp + N)` accesses use `$t4` as base — matching target's `lw $tN, K($t4)` pattern.

The emitted MIPS:

```mips
addiu $v0, $sp, 0x10         # ①: p = &mat
addu  $t4, $v0, $zero        # ②: mp = p (move expanded)
lw    $t5, 0($t4)            # ③: *mp
lw    $t6, 4($t4)
```

Exactly target's three-step pattern.

# NOT a compliant end state (was §6.1)

This idiom used to be filed under "single-instruction asm for codegen control."
**It no longer counts as a finished match** ([[completion-standard]],
[[inline-asm-allowed]]). The `move`/`addu` it emits is a register-allocation
workaround — GCC *can* produce that copy from C, so the job is to find the C
structure that makes GCC emit it, not to inject the instruction. Use the idiom
only to *diagnose* the target's register flow, then remove it before committing.
If you reach for it on every other line, the function wants a different C
structure entirely (re-read m2c's base.c).

# Companion technique: pin the load destinations

The inline-move forces the BASE register. To also match target's *load destinations* (typically `$t5/$t6/$t7`), pin those too:

```c
{
    register s32 *mp asm("$12");
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");
    s32 *p = mat;
    __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(p));
    t5 = *mp;
    t6 = *(mp + 1);
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t6));
    t5 = *(mp + 2);
    t6 = *(mp + 3);
    t7 = *(mp + 4);
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t5));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t6));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
}
```

This combination (mp pinned to $t4 + values pinned to $t5/$t6/$t7) is what closed the last 21 → 0 diff jump in `calc_fc_frame_800203B4` (commit `a502eb4`).

# Where this pattern shows up

The recurring shape is **"function takes a pointer, passes it to a callee that fills it, then loops over the result loading N words into GTE."** Variants in this codebase:

- Matrix loads after `func_8002EECC(src, mat)` → 5 `lw` from mat → 5 `ctc2`
- Vector packing: `vec[]` filled from `arg2[]`, then `lhu` from vec → pack → `mtc2`
- Output stores: arg0 incremented by struct offset → 3 `swc2` from GTE result regs

Search asm for the giveaway `addu $rD, $rS, $zero` (move) following an `addiu $rS, $sp, K` or `addiu $rS, $sN, K`. If it precedes a run of indexed loads/stores, this technique applies.

# When NOT to use this

If your function has a `register asm("$12")` pin AND the pinned value is **also** used as an inline-asm operand (e.g., `__asm__ volatile("lwc2 $1, 8(%0)" :: "r"(vp))`), GCC is *already* materializing the pin — you'll get the indirection automatically. The inline-move trick is only needed when the pinned value's only uses are C-level `mp[N]` / `*mp` (which GCC folds to sp-relative).
