
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern s32 single_game_getEnemyCharId(s32, s32);
extern s16 Judge;
void cpu_get_dist(s32 *a0, s16 *a1)
{
  s32 angle = single_game_getEnemyCharId(a1[0], a1[2]);
  s16 cos_val = *((&Judge) + ((angle + 0x400) & 0xFFF));
  s32 vx = *((s32 *) (((u8 *) a0) + 0x44));
  long sin_val = *((&Judge) + (angle & 0xFFF));
  s32 vz = *((s32 *) (((u8 *) a0) + 0x4C));
  s32 rx = (((*((s32 *) (((u8 *) a0) + 0x44))) * cos_val) + (vz * sin_val)) >> 12;
  s32 rz = -((((-(*((s32 *) (((u8 *) a0) + 0x44)))) * sin_val) + (vz * cos_val)) >> 12);
  s32 v48 = *((s32 *) (((u8 *) a0) + 0x48));
  *((s32 *) (((u8 *) a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
  *((s32 *) (((u8 *) a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
  if (v48 < 0)
  {
    v48 += 3;
  }
  *((s32 *) (((u8 *) a0) + 0x48)) = v48 >> 2;
}

/* REJECTED 2026-06-13 — cheat-reviewer FAIL (orchestrator-confirmed).
 * Re-seeded permuter (from clean long-sin_val base) score-0 form. Two coercions:
 *   (1) inline duplicate-read of *((s32*)((u8*)a0+0x44)) at both use sites instead
 *       of caching in `s32 vx` — straight-line (NOT the branch-arm shape that
 *       [[split-read-defeats-hoist]] sanctions); necessary ONLY because caching
 *       scores 21 and inlining scores 0 (cheats-by-spelling test #4). Target loads
 *       a0+0x44 ONCE + negu; m2c reconstructs the cached form.
 *   (2) `long sin_val` asymmetric type-widening vs `s16 cos_val` — no semantic
 *       purpose, only shifts the load RTL.
 * The as-committed permuter output ALSO left `s32 vx` declared-but-unused (dead local).
 * Do NOT apply. Clean floor stays 21. */
