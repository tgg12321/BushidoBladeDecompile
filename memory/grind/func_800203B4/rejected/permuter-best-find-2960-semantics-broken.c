/* REJECTED (s4 permuter, 2026-09-01): the BEST novel find of a ~35k-iteration permuter
 * campaign over the pure-C (islands-deleted) chassis. asm-differ score 2960 vs base 3000 -
 * a sub-one-instruction move - and SEMANTICALLY INVALID: `arg0 += 0x354;` is hoisted above
 * the 0x350/0x352 stores and the game_GetPlayerData load, so every subsequent `arg0 + 0xNNN`
 * offset is shifted by 0x354. Permuter randomizations are not required to preserve semantics;
 * this one does not. Banked so no future session mistakes the 2960 datapoint for a lead.
 * Campaign: tmp/grind/func_800203B4/s4/perm (label purec-no-islands-fnonly). See evidence.md
 * facts 28-29 for why no valid C form can score below 26 here. */
void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2)
{
  s32 mat[8];
  s32 vec[3];
  s32 src;
  *inline_fn(arg0) = 1;
  arg0 += 0x354;
  *((s16 *) (arg0 + 0x352)) = *((u16 *) (((u8 *) (&D_8008D59E)) + (arg1 * 20)));
  {
    s32 new_var;
    new_var = game_GetPlayerData(*((s16 *) (arg0 + 4)));
    src = *((s32 *) ((((s32) (*((s16 *) (arg0 + 0x352)))) << 2) + new_var));
  }
  func_8002EECC(src, mat);
  vec[0] = arg2[0];
  vec[1] = arg2[1];
  vec[2] = arg2[2];
}
