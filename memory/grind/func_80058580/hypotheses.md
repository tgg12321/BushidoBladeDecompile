# Hypothesis ledger — func_80058580

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM. `candidate.c` scores 30 of 2991 insns (floor history 2989 -> 30). Jump-table blocker resolved (rodata-object-alignment ruling); maspsx `.L` hazard fix adopted (-2).
- CONSTRAINTS: no compiler/maspsx changes (no-compiler-divergence.md). work1..work4 each carry 7-12 roles -> lands only under Ruling 11 (`.claude/rules/reused-local-necessity.md:89-132`): dumps for both spellings, a mechanism, every reviewer-proposed split banked, manual-path layer-2. Constant-only values refused unless the Q20 per-branch-constant exception covers them.
- REMAINING DIFFS: $a1 local-temp cascade from the case-2 `w >> 27` temp; two `0x438 >> 4` sites (ours lhu/sll/sra, target lh/sra - combine forms the sign-extract; the original kept the lh separate somehow); the lim-chain `j`; three jtbl %lo sandbox artifacts that vanish at landing.
- PLAN:
  1. Re-measure the candidate.
  2. Fix the lone `(&D_8009A838)[...]` scalar-array spelling at candidate.c:242 (shape retired by judge-decl-cleanup 8e007927d).
  3. Bank a Ruling 11 (C)(3) value audit per work variable; flag `work1 = 100000` (:240), `work2 = -1` (:578), the `work3 = 0/1` writes against Q20.
  4. Diagnostics: `0x438 >> 4` - forms where the lh result has a second use in the same block, or read through a typed struct member; $a1 cascade - read .lreg for the `w >> 27` temp's local-alloc seat.
  5. Resolve the func_80057E84 prototype conflict (below).
- DEPENDS [F]: candidate has `extern void func_80057E84(u8 *, u8 *, s32, s32)` (:21, call :346) but func_80057E84's body takes `PathWalker *` and sits earlier in text1b.c (2959 vs 2997) -> land func_80057E84 first and adopt PathWalker here. Landing deletes the transcribed jtbl arrays just before line 2997. Shares the D_80099D8F alias with func_80055B60 and the record with func_80023F08.
- ODDS/LANE: manual, 3+ sessions (mostly Ruling 11 paperwork), ~35% [I].
