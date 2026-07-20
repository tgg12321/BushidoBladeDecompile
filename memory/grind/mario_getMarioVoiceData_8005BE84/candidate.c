/* Candidate body for mario_getMarioVoiceData_8005BE84 (src/text1b.c).
 * Pure C, ZERO rules / pins / asm. Honest pure-C floor = 4 (sandbox --disable all).
 * Replaces the HEAD cheat form (register asm("$16") + asm("$3") pins).
 * Requires the data decl: extern s16 D_8009AD1C[][2];
 */
s32 mario_getMarioVoiceData_8005BE84(s32 arg0)
{
  s32 result;
  s16 *p;
  s16 temp_a0;
  s16 *base;
  title_mv_exec2(0);
  base = &D_8009AD1C[0][0];
  p = base + arg0 * 2;
  temp_a0 = (arg0 << 1) + 1;
  if (*p >= 0)
  {
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
    result = func_80085EE4(*p);
    md_game_check_change_main_mode_katinuki(*p);
    func_80085E4C(temp_a0, temp_a0);
    func_80085FB8();
  }
  else
  {
    result = -1;
  }
  return (s16) result;
}
