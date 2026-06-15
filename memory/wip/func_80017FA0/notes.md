# func_80017FA0 (code6cac.c) — WIP / BLOCKED (canonical-asm candidate)

## TL;DR
HEAD byte-matches but ONLY via **6 regfix subst/delete rules** + **asm
barriers** (a `_frame` dead-frame coercion and a `val` `"0"(val)` barrier). Both
are cheats (inline-asm-policy expanded catalog; the ARCHIVED [[scratchpad-gte]]
rule forbids this exact recipe). Honest pure-C floor measured at **16** (full
build, rules neutralized) — and the residual 16 is **two irreducible non-pure-C
gaps**, so this is a **canonical-asm authorization** function, not pure-C-closable.

## What matches in pure C (the inner loop — the bulk)
Keep the inner scratchpad writes as **full-constant addresses**
`*(volatile s32 *)(0x1F800064 + sp_inner) = ...`. That makes GCC re-materialize
`lui 0x1f80` every iteration, which is what the target does (the target does NOT
LICM-hoist the scratchpad base out of the loop). The whole inner double-loop then
matches byte-for-byte. See candidate.c.

## The two irreducible gaps (why it's not pure-C)
1. **Constant-store-address fold.** The two FIXED writes `0x1F8000B8` (=`ptr[0]<<7`)
   and `0x1F800060` (=`a0[3][1]`): target emits `lui at,0x1f80; sw v0,offset(at)`
   (2 insns, low 16 bits folded into the sw displacement). GCC 2.7.2 emits
   `lui r,0x1f80; ori r,r,0xb8; sw v0,0(r)` (3 insns, no fold). `((volatile s32*)
   0x1F800000)[idx]` folds *inconsistently* (folded in the all-index variant,
   unfolded in the hybrid — it's register-pressure-sensitive, not reliable). The 6
   regfix `subst`/`delete` rules manufacture the fold. No pure-C idiom reliably
   produces it (confirmed across this + the cpu_check_tubazeri scratchpad family).
2. **Unused 8-byte frame.** Target does `addiu sp,sp,-8` … `addiu sp,sp,8` with
   **nothing stored to the frame**. GCC emits no frame for this body. HEAD forces
   it with `s32 _frame; asm volatile("" :: "m"(_frame))` — a dead-var + barrier,
   forbidden by the expanded catalog.

Both are hallmarks of hand-written asm (or a different cc1 fold behavior).

## Endpoint
Per [[scratchpad-gte]] path (b): authorize as **canonical-asm** (user decision —
outside worker scope). Blocked on the board with this reason. 24 prior commits
(2026-03/04) never reached a clean pure-C match — consistent with no pure-C path.
