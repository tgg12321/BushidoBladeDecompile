---
name: defeat-licm-hoist-var-reuse
description: "Target recomputes a loop-invariant (e.g. limit-1) INLINE but GCC hoists it: reuse one C variable for a USED loop-variant AND the invariant — a multi-set pseudo isn't a loop.c movable. Pure C."
paths: [".claude/rules/defeat-licm-hoist-var-reuse.md"]
metadata:
  type: reference
---

# Defeat loop.c invariant-hoisting by reusing a scratch variable

## Symptom

The target computes a loop-invariant **inline, every iteration** (e.g. `addiu v0,s5,-1` =
`limit-1` each pass). Your `if (i == limit - 1)` makes GCC **hoist** `limit-1` into a fresh
callee-save — frame grows, prologue/epilogue and branches cascade. `i + 1 == limit` avoids the
hoist but emits the wrong operands. More generally: you save more `s` registers than the target
because loop.c hoisted more invariants (symbol addresses, constants, givs) than the target did.

## Why

`limit - 1` is a non-trapping, single-set, loop-invariant pseudo, so `scan_loop` admits it as a
*movable* (loop.c ~705: `n_times_set==1 || consec_sets_invariant_p`) and `move_movables` hoists it
when `threshold * savings * lifetime >= insn_count` — essentially always for small loops. Placing
the computation inside an `if` or reordering does not prevent it. Treat the desirability test as
unwinnable (shrinking the loop hoists MORE); attack movable *admission* instead.

Forbidden alternatives: `volatile`-qualifying or `*(volatile T *)`-casting a game-state global to
defeat the hoist ([[legitimate-volatile-interrupt-touched]]), and `__asm__` barriers
(`"=r"(x):"0"(x)`) — cheat-asm ([[inline-asm-policy]]).

## The pure-C fix — make the invariant's pseudo MULTI-SET

Route the invariant through a variable that is **also assigned a *used* loop-variant value earlier**
(non-consecutively): `n_times_set > 1` → not a movable → recomputed inline, in the variant's
register. Read the target for which register it reuses and pick that variable.

```c
s32 tmp;
tmp = (*pal & 0xFF000000) | ((u32)colors & rgb_mask);  /* variant value, USED next */
*pal = tmp;                                            /* the use — prevents DCE */
colors = (s32 *)((u8 *)colors + 0x30);
tmp = limit - 1;                                       /* REUSE tmp for the invariant */
if (i == tmp) {                                        /* addiu limit-1; bne i,limit-1 inline */
    D_800905F8 = idx;
}
```

The variant write must genuinely be **used**, otherwise DCE removes it and `tmp` collapses back to
a single-set invariant that hoists again. The same lever kills hoisted constants
(`vflag = 1; e2[0x58] = vflag;`) and giv hoists (`v1 = t4 << 5; a3 = D_800A8FB0[v1 + t1];`).

This is the frozen-list family "Variable reuse for codegen control" ([[no-new-park-categories]]);
when claimed as that family, the self-vet gate requires a `/* FAKE: ... */` annotation
(SOTN: "FAKE but makes register allocation work"), plus documented exhaustion and review.

## Instruments

`pwsh tools/grinder/dump.ps1 <func>` writes `tmp/grind/<func>/dumps/<tu>.loop`; each movable logs
`Insn N: regno R (life L), savings S  moved to M` or `not desirable`. Grep the function's slice for
`call_insn` to settle `loop_has_call`. Threshold = `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`
(loop.c:532); with `-msoft-float` (canonical since 2026-09-07) the FP regs are fixed, so it is
≈58 call-free / ≈29 with a call. `insn_count` doubles cumulatively once any movable's regno was
already moved (loop.c:1612).

If hoisted bases land in `s0`/`s1` instead of the target's `t8`/`t7`, that is the allocator's
ascending scan, not this lever — see [[local-alloc-death-count-class-wall]].

Related: [[register-alloc-pure-c]] · [[split-read-defeats-hoist]] (trapping-invariant case) ·
[[no-compiler-divergence]]
