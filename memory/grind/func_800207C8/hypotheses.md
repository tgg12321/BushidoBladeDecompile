# func_800207C8 hypotheses

- Start from the phase-local score-99 shape and reproduce m2c's working-pointer
  roles (`var_a0`, `var_a1 = var_a0 + 8`, `var_a3`, then `temp_a2`) without
  extending their lifetimes across phases.
- Search declaration order for the `$s5` player-data / `$s6` fourth-output swap;
  do not use explicit register variables.
- The final three-instruction excess comes from scalar copies of `arg1[0..2]`;
  test a truthful three-word record assignment or a small vector struct.

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated 2026-09-24; INCLUDE_ASM. "ASM-PARTIAL" = contains cop2 (44/317), nothing inline in src. A Codex session's full semantic draft reached 320/317, score 99, frame 0x40 (evidence.md); the body was NOT saved. Gap mostly register allocation: s5/s6 swap, arg-register seats in loops 1-3, 3 extra insns in the root-copy region.
- CONSTRAINTS: islands are exactly the PINNED units gte_SetRotMatrix, gte_ldv0 (`lwc2 $0/$1`), gte_rtv0 (nop nop word), gte_stlvnl (asm 80020858-800208A8). The DMPSX command word needs a per-function owner grant (`.claude/rules/inline-asm-policy.md:93-118`). The `.L80020A70` load-delay label nop is handled globally by maspsx since 2026-09-14 (`memory/grind/func_80058580/evidence.md`).
- BLOCKER [I]: register allocation under an untyped data model. Real types: arg0 = `&g_practice_menu_table[i]` (`src/code6cac_c_mid.c:1505`); ability table `D_800F5F68 + pid*0x1B8` = 22 entries x 0x14 (u16, s16 bone idx, SVECTOR at +4, s16[4]) - matches what func_800206B0 writes (tu2:3013); `game_GetPlayerData()` returns `MATRIX*[]` (+0x14/+0x18/+0x1C = t[0..2]); arg1/arg2/arg3 are scratchpad Vec3i32 arrays (22 bone outputs, up to 3 attachment vectors, 2 extra).
- PLAN:
  1. Redraft from the target: `PracticeMenuRec *rec` + members at 0x8C, 0x198/0x1A4 (Vec3i32), 0x1B0 (s32[2]), 0x1BA, 0x1C2, 0x1EC (Vec3i32); an AbilityEntry struct; `MATRIX **` player data; `Vec3i32 *` outputs; D_8008D864 as `u8[]`, D_8008D86C / D_8008D88C as pointer arrays (evidence.md). inline_o.h islands from the start. `sandbox --diff`. SAVE THE BODY in this ledger.
  2. hypotheses.md leads: declaration order for s5/s6; m2c pointer roles kept local per phase. Try a Vec3i32 struct assignment for the arg1 -> rec+0x180 copy.
  3. Then permuter and `do { } while (0)` wraps. No register pins.
  4. At 0: owner DMPSX-word grant (batch with func_800204C0).
- DEPENDS: PracticeMenuRec + ability-table typing shared with func_800204C0 / func_800206B0.
- ODDS/LANE: 2-4 sessions, manual to start; ~50% to 0 [I].
