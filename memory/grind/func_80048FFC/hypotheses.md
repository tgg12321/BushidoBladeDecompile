# Hypothesis ledger — func_80048FFC

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated (distance 232). No ledger before this entry; only `pre-include-asm-body.c`. The score-31 attempt (2026-09-21, `--disable lost-codegen`, NOT `--disable all`) survives only in the gitignored permuter workspace `tools/decomp-permuter/nonmatchings/func_80048FFC_regs/base.c` (permuter score 570) - banked here as `permuter-base-score31.c`. It reuses `x_mask`/`high_mask` across unrelated mask/height/arg roles (the rotation's "not landable").
- CONSTRAINTS: no Ruling 5-12 admission for those roles; constant holders are FAKE-only (named-local-fake-exception).
- BLOCKER [I]: an artifact of how it was written. The hoisted `0xFFFFFF`/`0xFF000000` masks (s5/fp) look like PsyQ `addPrim` on a P_TAG bitfield (`addr:24`); the repo has `OTag` (`include/gpu.h:20-23`, used at code6cac_c2.c:1002/1030). The `-0x40`/`-0x100` register constants fit `& ~0x3F` / `& ~0xFF` on promoted s16s. With those forms no mask variables - so no reuse - should be needed.
- PLAN:
  1. Reference: `permuter-base-score31.c` (banked 2026-10-01).
  2. Fresh honest body: the 0x134-byte channel record at D_800EF848 (+0 s32 counter; +4 two 0x90 DR_MOVE banks; +0x124 a 7-halfword control record filled by func_80048F58); a RECT local (sp+0x10); SetDrawMove twice per iteration; addPrim via OTag (needs gpu.h in text1b.c or a local typedef - check name collisions, byte-neutral); `x % 64`, `& ~0x3F`.
  3. The 12 spill slots 0x18..0x70 follow declaration order (as func_80057E84 showed) - tune declaration order first.
  4. Permuter from that body.
- DEPENDS: possibly a gpu.h include in text1b.c.
- ODDS/LANE: Grinder or manual, 1-2 sessions, structural re-derivation, ~45% [I].
