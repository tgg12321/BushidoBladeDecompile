/* REJECTED (s3, 2026-09-06) - 13 / 50 insns, byte-identical output to plain form C.
 * HYPOTHESIS TESTED: pseudo numbers are assigned in declaration order, so declaring
 * src in an inner block AFTER the store statement should make its pseudo NEWER than
 * expand's call-result temp and let cse2 canonicalise the loop preheader's giv init
 * onto the temp, killing the copy insn. It does not: the .flow chain is unchanged
 * (insn 20 `(set (reg/v:SI 72) (reg:SI 75))` still feeds insn 145
 * `(set (reg 97) (plus (reg/v 72) 52))` in the preheader). Z7 (dst inner-scoped too)
 * is also 13/50. */
extern void *game_GetCharData(void);
/* Per-entry record (stride 0x68) shared by the D_800A9CF8.unkC table and the
 * game_GetCharData() table; the sibling func_80044B30 walks both with the
 * same stride and field offsets. */
typedef struct {
    s8 unk0;
    s8 unk1;
    s16 unk2;
    u16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    s8 pad18[0x14];
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s8 pad38[0x14];
    s32 unk4C;
    s32 unk50;
    s32 unk54;
    s32 unk58;
    s32 unk5C;
    s8 pad60[8];
} Rec4473C;
void func_8004473C(void)
{
    Rec4473C *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)game_GetCharData();
    {
        Rec4473C *src = (Rec4473C *)D_800A9CF8.unk10;

        dst = (Rec4473C *)D_800A9CF8.unkC;
        for (i = 0; i < D_800A9CF8.unk6; i++, dst++, src++) {
            dst->unk0 = 0;
            dst->unk1 = 0;
            dst->unk2 = 0;
            dst->unk4 = D_800A9CF8.unk0;
            dst->unk6 = 0;
            dst->unk8 = 0;
            dst->unkA = 4;
            dst->unkC = 0;
            dst->unk10 = 0;
            dst->unk12 = 0;
            dst->unk14 = 0;
            dst->unk4C = src->unk2C;
            dst->unk50 = src->unk30;
            dst->unk54 = src->unk34;
            dst->unk58 = -1;
        }
    }
}
