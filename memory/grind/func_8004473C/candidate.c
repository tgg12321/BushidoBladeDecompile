/* CANDIDATE — func_8004473C (s1, 2026-09-06). Honest floor 13 (sandbox --disable all).
 * Requires the aggregate merge in include/game.h (Unk800A9CF8Header D_800A9CF8) and the
 * sibling rewrites in src/text1a_c.c — apply memory/grind/func_8004473C/candidate_merge.patch
 * to a clean tree (git apply) to reproduce this exact state. Residual: pre-loop scheduling
 * only (count load lands after the store; i=0 slides into the blez delay slot); every
 * loop-body byte matches. See evidence.md H2/H3 for the exact mechanism. */
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
    Rec4473C *src;
    Rec4473C *dst;
    s32 i;
    Rec4473C *tmp;
    s32 n;

    D_800A9CF8.unk10 = (s32)game_GetCharData();
    src = (Rec4473C *)D_800A9CF8.unk10;
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
