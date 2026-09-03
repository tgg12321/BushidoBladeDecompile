/* s16 2026-09-03 forensics.  score 2 / build 65 / target 66 -- ONE INSTRUCTION SHORT.
 * Order: outer = D_800A3468;  P(p10 read);  Z(D_800F10D0 zero store);  C1,C2,C3,
 * S1, S2, S5, S3, S6, S4, S7, S8.
 *
 * WHY IT IS BANKED: with the p10 read placed BEFORE the D_800F10D0 zero store, the body
 * is target's instruction stream exactly, EXCEPT that p10's `lw $a1,0x10($v1)` is spent
 * filling the load-delay slot after `lhu $v0,0x0($v1)` (our slot 4), which target leaves
 * as a nop -- so everything from slot 5 on is target shifted one slot earlier and the
 * body is 65 insns instead of 66.  Target's 25/26/27 addiu / sw %gp(D_800A3478) /
 * sh 0x1A group is reproduced, and both halfword seats are target's.
 *
 * DEAD AS SPELLED (s16): p10's load depends only on $v1 (outer), so it is
 * unconditionally ready at the top of the block once its statement precedes the zero
 * store; no reordering of the remaining statements holds it back.  N1, N5, N7, N8 and
 * every X1_* / X2_* row of tmp/grind/func_80060A68/s16/xsweep.log keep it at slot 4.
 */
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    s32 outer;
    s32 idx;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    p10 = *(s32 *)(outer + 0x10);
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
