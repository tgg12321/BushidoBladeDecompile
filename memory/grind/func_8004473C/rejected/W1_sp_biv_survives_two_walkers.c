/* REJECTED (s3, 2026-09-06) - 23 / 52 insns on the merged-struct chassis.
 * WHY IT IS DEAD: anchoring the source walker at +0x34 as an s32* with an
 * add_val-0 access (sp[0]) does exactly what it was designed to do - loop.c
 * leaves the biv register live, reg_n_sets[sp] == 2, birthing_insn_p fails and
 * sched1 emits the TARGET's block-0 order, anchor and delay slot verbatim
 * (move a2,zero / lw a1,12 / lh v0,6 / addiu a0,16 / blez / sw v1,0(a0)).
 * The cost is structural and unavoidable in this shape: the surviving biv does
 * NOT absorb the two remaining reads, so the loop carries TWO walking pointers
 * ($a3 the biv, $a0 = biv+48 the giv) with TWO increments, and the call-result
 * copy stays live into the preheader, so it is multi-block, global.c seats it in
 * $v1 and the local pre-check count temp takes $v0 (target: src $v0, count $v1).
 * Net +3 instructions. W9 (dst loaded first) and W13 (Rec* reload then &src->unk34)
 * both collapse to the identical 23/52 output. */
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
    s32 *sp;
    Rec4473C *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)game_GetCharData();
    sp = (s32 *)(D_800A9CF8.unk10 + 0x34);
    dst = (Rec4473C *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; i++, dst++, sp += 0x1A) {
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
        dst->unk4C = sp[-2];
        dst->unk50 = sp[-1];
        dst->unk54 = sp[0];
        dst->unk58 = -1;
    }
}
