/* REJECTED (s1, 2026-09-06): form A — src = game_GetCharData(); D.unk10 = src.
   Honest floor 15. Sched1 order matches the target (i=0, dst, cnt, addr, blez, sw)
   but the call-result pseudo is the multi-block user var src (no local temp), so
   local-alloc seats the short pre-check count temp in v0 first (REG_ALLOC_ORDER);
   global.c then gives src a0 -> "move a0,v0" + "sw a0" + "lh v0"/"addiu v1" seats.
   Mechanism: local-alloc.c copy-suggestion pass needs a LOCAL qty covering the
   count load; src is global. Kill scope: instance. */
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

    src = game_GetCharData();
    D_800A9CF8.unk10 = (s32)src;
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
