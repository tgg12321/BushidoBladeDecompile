/* s16 2026-09-03 forensics.  score 5 / build 66 / target 66.  Order:
 * C1,C2,C3, S1(0x18 store), S2(temp2 = +2 read), S3(0x1A store), P(p10 read), S4(idx),
 * S5(D_800A3478 gp store), S6, S7, S8 -- i.e. the gp store AFTER the 0x1A store.
 *
 * WHY IT IS BANKED: this body reproduces TARGET'S EXACT THREE-LOAD GEOMETRY --
 * lw $a1,0x10($v1) at slot 11 and lw $a0,0x10($v1) at slots 19 and 22 -- at 66
 * instructions, in ordinary C with no FAKE construct of any kind.  Its entire residual
 * is the +2 halfword value's seat: QTYDBG qty11 / reg74, birth 36 death 38, refs 2,
 * allocation order 5 of 25, got=2 ($v0) where target needs $a0 -- plus the tail order
 * that follows from it (the idx read hoisted into slot 25 and the addiu / sw %gp pair
 * pushed to 27/28 instead of target's 25/26).
 *
 * DEAD AS SPELLED (s16): on this spine no statement insertable between the +2 read and
 * the 0x1A store buys $a0 without displacing instructions target puts elsewhere --
 * S4 (idx) leaves got=2 (L4, score 5); S7 costs an instruction (L5, 67); a copy
 * statement DOES buy $a0 but schedules its own three insns late (L1, score 11).
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
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    *(u16 *)(outer + 0x1A) = temp2;
    p10 = *(s32 *)(outer + 0x10);
    idx = *(u16 *)outer;
    D_800A3478 = outer + 0x18;
    temp_a1 = *(u16 *)(p10 + 4);
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
