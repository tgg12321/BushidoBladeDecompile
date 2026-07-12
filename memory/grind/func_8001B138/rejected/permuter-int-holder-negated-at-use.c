/* REJECTED — s4 permuter find (nonmatchings/func_8001B138 output-0-1, iter 337/336,
   score 0). Same forbidden typed-literal-holder family the Judge already banned
   (s2/s3): a separately-typed local (`int new_var`) whose sole purpose is to carry
   the clamp literal so its own tree-level constant survives the u16-target-type
   reconversion, materializing as addiu instead of ori. This spelling additionally
   fails the human-programmer test worse than the s16/s32 variants already rejected:
   the holder is declared at the TOP of the function (with the zero-init block),
   holds an unrelated POSITIVE magnitude (0x1C00), and is negated only at the single
   use site 20 lines later — no semantic purpose, no FAKE annotation, no documented
   lever-exhaustion. Confirms (independently, via 337-iteration black-box random
   search rather than hand derivation) that the ONLY score-0 spelling in this
   residual's search space is a member of the forbidden family — see
   memory/grind/func_8001B138/evidence.md [s4]. */
void func_8001B138(s32 *arg0)
{
  int new_var;
  D_800FF5C8 = 0;
  D_800FF5CC = 0;
  new_var = 0x1C00;
  D_800FF5D0 = 0;
  D_800FF5D8 = 0;
  D_800FF5DA = 0;
  D_800FF5DC = 0;
  D_800FF5E0 = 0;
  if ((D_800A38BA != 0) && (D_800A3834 == 1))
  {
    if ((*arg0) & 1)
    {
      D_800A37E0 = 1;
      if ((*arg0) & 8)
      {
        g_file_vram_timer = (u16) (g_file_vram_timer + 0x4CC);
      }
      if ((*arg0) & 2)
      {
        g_file_vram_timer = (u16) (g_file_vram_timer - 0x4CC);
      }
      if (((s16) g_file_vram_timer) < (-0x1C00))
      {
        g_file_vram_timer = -new_var;
      }
      if (((s16) g_file_vram_timer) >= 0x7401)
      {
        g_file_vram_timer = 0x7400;
      }
      *arg0 = (*arg0) & (~0xB);
    }
    {
      s32 v;
      v = (s16) g_file_vram_timer;
      if (v < 0)
      {
        v = v + 0xF;
      }
      D_800FF5E0 = v >> 4;
    }
  }
  *arg0 = (*arg0) & (~0x10001);
}
