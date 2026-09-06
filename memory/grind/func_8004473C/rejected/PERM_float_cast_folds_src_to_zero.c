/* REJECTED (s4, 2026-09-06) - permuter proposal, engine score 3 at 49 insns. CHEAT and also
 * SEMANTICALLY WRONG: `src = (Rec4473C *)((float)D_800A9CF8.unk10);` round-trips a pointer
 * through float, which GCC 2.7.2 folds so that src becomes a constant - the loop then reads
 * from the wrong address (`li a0,52` where the target has `addiu a0,v0,52`). A cast that is
 * not a real conversion, present only for codegen, is the redundant-cast family (F2).
 * Diagnostic value: with NO copy insn in block 0 the schedule is the target verbatim, which
 * is the same evidence the chained assignment then delivers legitimately.
 */
void func_8004473C(void)
{
  Rec4473C *src;
  Rec4473C *dst;
  s32 i;
  D_800A9CF8.unk10 = (s32) game_GetCharData();
  src = (Rec4473C *) ((float) D_800A9CF8.unk10);
  dst = (Rec4473C *) D_800A9CF8.unkC;
  for (i = 0; i < D_800A9CF8.unk6; i++, dst++, src++)
  {
    dst->unk0 = 0;
    dst->unk1 = 0;
    dst->unk2 = 0;
    dst->unk4 = D_800A9CF8.unk0;
    dst->unk8 = 0;
    dst->unkC = 0;
    dst->unkA = 4;
    dst->unk10 = 0;
    dst->unk12 = 0;
    dst->unk14 = 0;
    dst->unk4C = src->unk2C;
    dst->unk50 = src->unk30;
    dst->unk54 = src->unk34;
    dst->unk6 = 0;
    dst->unk58 = -1;
  }

}
