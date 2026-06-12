---
name: defeat-licm-hoist-var-reuse
description: When GCC hoists a loop-invariant (e.g. limit-1) that the target recomputes INLINE, reuse one C variable for a used loop-variant AND the invariant — multi-set pseudo isn't a loop.c movable, so it's not hoisted. Pure C, no asm.
paths: [".claude/rules/defeat-licm-hoist-var-reuse.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
---

# Defeat loop.c invariant-hoisting in pure C by reusing a scratch variable

## Symptom

The target computes a loop-invariant **inline, every iteration** (e.g. `addiu
v0,s5,-1` = `limit-1` recomputed each pass, with `limit` live in a callee-save).
Your pure-C `if (i == limit - 1)` instead makes GCC **hoist** `limit-1` out of the
loop into a fresh callee-save (`s8`) — the frame grows and the whole prologue/
epilogue + branch targets cascade into a large diff. Switching to `i + 1 == limit`
avoids the hoist but emits `addiu i+1` / `bne i+1,limit` — the wrong operands
(target has `addiu limit-1` / `bne i,limit-1`).

## Why GCC hoists it (loop.c, confirmed in `tools/gcc-2.7.2/loop.c`)

`limit - 1` is a **non-trapping, single-set, loop-invariant** pseudo, so `scan_loop`
records it as a *movable* (the `may_trap_p` exclusion at line ~715 only blocks
*trapping* invariants — memory loads, divides — from being speculatively hoisted past
a conditional/call; a plain `addiu` is never excluded). `move_movables` then hoists
it whenever `threshold * savings * lifetime >= insn_count`, which for a small
(~29-insn) loop-with-call is essentially always true. So **no amount of placing the
computation inside an `if` or reordering prevents the hoist** — the value is simply
invariant and cheap.

`volatile`-qualifying the base *does* defeat the hoist (the value becomes non-
invariant) but **spills it to the stack** (an extra `lw` each iteration) — wrong,
the target keeps the base in a register. **Also FORBIDDEN as of 2026-05-31**:
declaring a game-state global as `volatile` (or casting via `*(volatile T *)`) to
coerce GCC's hoist analysis is a cheat per [[inline-asm-policy]] (expanded
catalog). An `__asm__` barrier (`"=r"(x):"0"(x)`) also works but is **cheat-asm**
(never committable); it's what the sibling `func_8003DE14` settled for — that
function's status now needs re-evaluation under the expanded catalog.

## The pure-C fix — make the invariant's pseudo MULTI-SET

A movable must be set **once** in the loop (loop.c line ~705: `n_times_set==1 ||
consec_sets_invariant_p`). If the invariant lands in a pseudo that is **also assigned
a *used* loop-variant value earlier** (non-consecutively), `n_times_set > 1` and it is
**not a movable → not hoisted → recomputed inline.** And it naturally reuses that
variant's register — which is exactly what the target does.

**Read the target for which register it reuses.** In func_8003DBE4 the target's `v0`
holds the `*pal` store result (`sw v0,0(a0)`), then is **immediately reused for
`limit-1`** (`addiu v0,s5,-1`). So route both through one C variable:

```c
s32 tmp;
*colors = (*colors & 0xFF000000) | (*pal & rgb_mask);
tmp = (*pal & 0xFF000000) | ((u32)colors & rgb_mask);  /* variant value, USED next */
*pal = tmp;                                            /* the use — prevents DCE */
colors = (s32 *)((u8 *)colors + 0x30);
tmp = limit - 1;                                       /* REUSE tmp for the invariant */
if (i == tmp) {                                        /* -> addiu limit-1; bne i,limit-1 inline */
    D_800905F8 = idx;
}
```

The first `tmp = …; *pal = tmp;` must genuinely **use** `tmp` (here, the store),
otherwise DCE removes the variant assignment and `tmp` collapses back to a single-set
invariant that hoists again. Pick the variable the target actually reuses (diff the
target asm), so the register matches too.

## Confirmed case — func_8003DBE4 (code6cac_c2.c, 2026-05-28)

Honest distance 35 → 0; **36 regfix rules dropped** (incl. a brittle hardcoded-global-
label `subst ".L280-4"`), 100% pure C, SHA1 == oracle. The full match needed three
pure-C levers found by reading the RTL/target, not the compiler:
1. **dead local** `s32 buf[2];` for the target's 8 extra frame bytes ([[dead-vars-local-array]]).
2. **redundant dead store** — `step = base_val - arg0;` duplicated into one branch arm
   — to steer the constant into `v0` (a scheduling/RA tie diagnosed from the `.greg`
   dump; see [[register-alloc-pure-c]]).
3. **this rule** — reuse `tmp` for the `*pal` value and `limit-1` to defeat the
   loop-invariant hoist.

The hoist had looked like a "compiler-fork wall"; it was not — see
[[difficult-is-not-impossible]]. The matching C existed; it just required reading the
target's exact register reuse and the loop.c movable rules.

## Related
- [[difficult-is-not-impossible]] — this is the case study; "hard" ≠ "impossible"
- [[register-alloc-pure-c]] — companion RA levers (dead store, block-local split)
- [[split-read-defeats-hoist]] — the *trapping*-invariant hoist case (memory address);
  defeated by branch-duplication. This rule is the *non-trapping* case (register
  arithmetic), defeated by multi-set var reuse.
- [[inline-asm-policy]] — why the `__asm__` barrier alternative is cheat-asm, not a match
