# Hypothesis ledger — func_800198D0

## KILLED (s2, 2026-09-29)
- Expression (?:) GETBITS macro with a function-scope value carrier (`hi`/`v`): join copies `move t3,t0` at every site, 588 (tmp/func_800198D0/v2_exprmacro_588.c).
- GNU statement-expression GETBITS arms: the simple arm's value carrier cannot be combined into a plain destination (`srl v0; ...; move t2,v0`), 501.
- Any spelling that assigns `bits = 32 - need` directly (17 micro variants incl. nested assignment, bits += 20, u32/s16/u8 need, separate lo): no bits copy.
- Index-form copy loops `((u32*)d)[k] = ((u32*)s)[k]`: offset folds into the lw displacement.
- `~(delta / 2)` in the zigzag ternary: expand_expr singleton path hoists the division.

## OPEN (s2)
- v12 = 305: remaining is register assignment only (plus the order of two hoisted li constants). Frontier: allocation work with BB2_ALLOC_DEBUG dumps (tmp/func_800198D0/alloc.sh).
