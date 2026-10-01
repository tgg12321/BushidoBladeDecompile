# Hypothesis ledger — func_80049718

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM (revert 016788ef0). Landed body banked `rejected/review-2026-09-30.c`. Best honest-typed variant b1 (real MulMatrix0, MATRIX copy, `SVECTOR rot`) scores 0: `ff-c-2026-09-30/b1_proto_copy_svector.diff` (its base `tmp/ffc/score_full.py`, `revert_49718_text1b.c` is gitignored - bank it). Reopened: layer-2 rev-48ba4 FAIL (Q37), three load-bearing constructs with no admitting ruling: `p_anim` reused as `vehicle+0x44` (+9 without), `{ s16 *tbl }` block (+12 cumulative), `new_var3` staging of D_800A3820 (+14 cumulative); full honest rewrite 45.
- CONSTRAINTS: no false prototypes / SVECTOR over-read (evidence.md). A cast-typed borrow counts as a pun (func_80057E84 evidence.md last bullet). Staging chains (F1) refused (`.claude/rules/no-new-park-categories.md`; `reused-local-necessity.md:112`). A no-op copy is not a named intermediate (`no-new-park-categories.md:59-61`).
- BLOCKER:
  - tbl block is NOT really blocked [F]: the July record has the mechanism + exhaustion (fold puts symbol_ref second, index insn born first; `git show 797a1845c^:memory/grind/func_80049718/hypotheses.md`); Judge ruled it ordinary (`docs/grind/decisions.md:201-216`). Needs only the pointer-alias `/* FAKE */` annotation.
  - new_var3 [I]: sched1 birthing - `ot` is set twice, so the 2nd D_800A3820 load loses the once-set boost new_var3 restores.
  - p_anim [I]: allocation priority - its longer live range changes how $s0 is shared between p_anim and part.
- PLAN:
  1. Rebuild b1 into a scratch text1b.c; confirm 0.
  2. `/* FAKE: */` on the tbl line citing the July receipts.
  3. new_var3: one fresh once-written `ot` local per append block (two once-set pseudos); `-da` j0 vs b4, compare .sched / .greg.
  4. p_anim: `sandbox --diff` on b2, .greg j0 vs b2; honest forms: type the 0x68-byte object (layout matches `Unk80101DF0Record`, `include/code6cac.h:544-553`: rot +0x10, mat +0x18, work +0x38, work.t +0x4C); `vehicle->parts[flags & 1]`; true table prototype `MATRIX *(*g_anim_func_table[])(SVECTOR *, MATRIX *)`; declaration order.
  5. If only the reuse closes: Ruling 11 package (generic name `work`; casts make it risky) or owner question.
- DEPENDS: restoring `g_anim_func_table[]` (016788ef0 deleted `extern s32 (*g_anim_func_table[])(s16 *, s16 *);` from src/text1b.c) is shared with func_8005490C. Sibling func_80049A2C (text1b.c:978) uses `ot = (u8 *)D_800A3820` directly. Do NOT retype D_800A3820 (s32 in six TUs).
- ODDS/LANE: manual, 1-2 sessions of RTL-dump work, ~50% [I] (p_anim is the risk).
