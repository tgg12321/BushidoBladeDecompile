/* CANDIDATE - func_8004473C (s3, 2026-09-06). Honest floor 13 (sandbox --disable all,
 * build_insns 50 vs the target's 49). Unchanged from s1/s2; this is the form-C body.
 *
 * CHASSIS: requires the D_800A9CF8 aggregate merge. s3 measured that the declaration can be
 * TU-LOCAL (the typedef below plus `extern Unk800A9CF8Header D_800A9CF8;` placed in
 * src/text1a_c.c immediately before func_80044670) with byte-identical results, which keeps a
 * grind session inside its allowed edit surface; include/game.h placement is a packaging step
 * for the final candidate only. memory/grind/func_8004473C/candidate_merge.patch reproduces
 * the header-canonical variant plus the sibling rewrites (git apply to a clean tree).
 *
 * RESIDUAL (s3, trace-complete): every instruction this form emits is already one of the
 * target's - same &D_800A9CF8+0x10 anchor (addiu $a0,$a0,0x10 / addiu $a3,$a0,-0x10 /
 * sw $v0,0($a0)), same seats (dst $a1, src $v0, count $v1, i $a2, addiu $a0,$v0,0x34,
 * addiu $v1,$a1,0x58), byte-exact loop body. Only the block-0 ORDER differs, and the extra
 * 50th instruction is the load-delay nop forced by `lh $v1,6` landing immediately before
 * `blez $v1`.
 *
 * The whole divergence is ONE sched1 ready-list decision. In
 * tmp/grind/func_8004473C/s3/traceC/text1a_c.sched the T-2 ready list is
 * `15 (1) 20 (7f000001) 25 (1) 129 (1)`: insn 20 (`(set (reg/v:SI 72) (reg:SI 75))`, the
 * `src = <call temp>` copy) is a once-set live leaf, birthing_insn_p (sched.c:2505) holds,
 * adjust_priority (sched.c:2584) raises it to LAUNCH_PRIORITY, and since schedule_block
 * selects in groups of equal priority (sched.c:2674-2727) it wins T-2 outright. Without that
 * boost the store wins T-2 on potential_hazard, the addr insn takes T-3, the count load T-4,
 * the store becomes the last real insn before the branch (so reorg fills the delay slot with
 * it) and the nop disappears - 49 instructions and the target's bytes.
 *
 * W1 (memory/grind/func_8004473C/rejected/W1_sp_biv_survives_two_walkers.c) is the measured
 * proof of that: giving the source walker a surviving second set removes the boost and emits
 * the target's block-0 order, anchor and delay slot exactly - at the cost of a second walking
 * pointer in the loop (23 / 52). */

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;   /* stage id (stage_GetId) */
    s16 unk6;   /* entry count */
    s32 unk8;
    s32 unkC;   /* entry table (stride 0x68) */
    s32 unk10;  /* game_GetCharData() table (stride 0x68) */
    s32 unk14;
} Unk800A9CF8Header;

extern Unk800A9CF8Header D_800A9CF8;

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
