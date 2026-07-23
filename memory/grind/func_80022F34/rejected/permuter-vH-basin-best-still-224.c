/* func_80022F34 — REJECTED (grind s4, permuter)
 *
 * WHY DEAD: This is the BEST form the permuter found across ~35k iterations on
 * TWO chassis (base + vH), and it still scores 224 (permuter weighted) — WORSE
 * than the base chassis's own starting 174, and nowhere near a match (0).
 *
 * It is just base's named-temp class (val1 as a named intermediate `new_var`,
 * arg3 inlined) re-derived from vH's frame-correct chassis. Same regime s2
 * already mapped: a named val1 loaded before the reload => the CSE'd `la` base
 * regime (frame-correct) but NOT target's per-access %hi/%lo. No new lever; the
 * per-access<->phantom coupling is untouched.
 *
 * PERMUTER RESULT (s4):
 *   - base chassis (weighted 174, byte-perfect body + phantom frame): 30640
 *     iters, ZERO output dirs => nothing beat 174. Random codegen mutation over
 *     the byte-perfect base CANNOT remove the reg100 combine strand.
 *   - vH chassis (weighted 1250, frame-correct CSE'd body): best find = 224
 *     (this form). Only ever re-finds the known base/vSPLIT/vH classes; never
 *     produces target's (per-access %lo AND vars=0) form.
 *
 * Corroborates s1/s2/s3: the coupling is a fork-level cse2+combine interaction,
 * not reachable by any C the permuter can spell. `new_var` is a legitimate
 * named intermediate (SOTN-sanctioned), NOT a cheat — it simply doesn't help.
 */
void func_80022F34(void)
{
  s32 new_var;
  s32 i;
  u16 *tbl;
  s32 offset;
  i = 0;
  tbl = (u16 *) (&D_80102778);
  offset = 0;
  loop_22F34:
  {
    u8 *a0 = ((u8 *) (&D_80101EC8)) + offset;
    if ((*((s16 *) (a0 + 6))) != 0)
    {
      switch (D_800A38DC)
      {
        case 0:
          *((s16 *) (a0 + 8)) = (&D_80102782)[i] << 4;
          break;
        case 1:
        case 2:
        default:
          *((s16 *) (a0 + 8)) = *tbl;
          break;
        case 3:
          break;
      }
      new_var = (&D_801027BC)[(*((s16 *) (a0 + 0x4A))) * 5];
      single_game_SetStatusUpData(i, new_var, (&D_801027BC)[(*((s16 *) ((*((u8 **) a0)) + 0x4A))) * 5]);
    }
    tbl++;
    i++;
    offset += 0x44C;
  }
  if (i < 2)
  {
    goto loop_22F34;
  }
}
