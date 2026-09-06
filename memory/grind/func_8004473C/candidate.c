/* CANDIDATE - func_8004473C (s2, 2026-09-06). Honest floor 13 (sandbox --disable all),
 * unchanged from s1; this is the form-C body. Requires the aggregate merge in
 * include/game.h (Unk800A9CF8Header D_800A9CF8) and the sibling rewrites in
 * src/text1a_c.c - apply memory/grind/func_8004473C/candidate_merge.patch to a clean
 * tree (git apply) to reproduce this exact state.
 *
 * Residual (s2, fully attributed): ONE sched1 decision. Every register seat this form
 * produces is already the target's; only block-0 ORDER differs, because the copy insn
 * `src = <call temp>` is a once-set live leaf, gets birthing_insn_p's LAUNCH_PRIORITY
 * boost (sched.c:2505/2584) and takes the blez delay slot ahead of the store. See
 * evidence.md s2 and hypotheses.md H4/H5 - form A3 measures the mirror half (13 at the
 * target's 49 instructions with the target's seats, wrong address anchor).
 *
 * Two unused locals (tmp, n) that the s1 copy of this file carried have been dropped:
 * they were dead scalars with no semantic purpose. */
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
