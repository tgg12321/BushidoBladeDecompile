/* REJECTED (s4, 2026-09-06) - permuter proposal, engine score 0 at 49 insns (BYTE MATCH) but a
 * CHEAT: `D_800A9CF8.unk10 += 0;` (and its equivalent `D_800A9CF8.unk10 = (s32)src;`) is a
 * redundant store to a GLOBAL whose only effect is on codegen. The frozen dead-store family
 * (.claude/rules/dead-store-fake-exception.md) covers LOCALS and PARAMS only, so this is a
 * first reach of an unsanctioned family regardless of spelling. Banked because it LOCATED the
 * fix: it proved the block-0 residual dies when the standalone `src = <call temp>` copy insn
 * stops being a boostable birthing insn. The ordinary-C form that achieves the same thing is
 * the chained assignment in candidate.c (`D.unk10 = (s32)(src = ...)`), which also measures 0.
 */
void func_8004473C(void)
{
  s32 i;
  Rec4473C *src;
  Rec4473C *dst;
  D_800A9CF8.unk10 = (s32) game_GetCharData();
  src = (Rec4473C *) D_800A9CF8.unk10;
  D_800A9CF8.unk10 += 0;
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
