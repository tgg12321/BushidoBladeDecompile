# Hypothesis ledger — func_800198D0

## KILLED (s2, 2026-09-29)
- Expression (?:) GETBITS macro with a function-scope value carrier (`hi`/`v`): join copies `move t3,t0` at every site, 588 (tmp/func_800198D0/v2_exprmacro_588.c).
- GNU statement-expression GETBITS arms: the simple arm's value carrier cannot be combined into a plain destination (`srl v0; ...; move t2,v0`), 501.
- Any spelling that assigns `bits = 32 - need` directly (17 micro variants incl. nested assignment, bits += 20, u32/s16/u8 need, separate lo): no bits copy.
- Index-form copy loops `((u32*)d)[k] = ((u32*)s)[k]`: offset folds into the lw displacement.
- `~(delta / 2)` in the zigzag ternary: expand_expr singleton path hoists the division.
- One-variable-per-value spellings of the six reused locals (r11/proof.md section 2): all miss (3 .. 419).

## CONFIRMED (s2)
- The landing form (candidate.c) scores 0 and builds to the oracle SHA1 (2026-09-29). Remaining gate: layer-2 review of the Ruling 11 record (r11/proof.md).
