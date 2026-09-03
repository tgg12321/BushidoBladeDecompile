/* s16 2026-09-03 forensics.  score 11 / build 66 / target 66.  Order:
 * C1,C2, S1, S2, C3, S3, P, S4, S5, S6, S7, S8 -- the THIRD COPY STATEMENT moved
 * between the +2 read and the 0x1A store, with the D_800A3478 gp store left after the
 * 0x1A store.
 *
 * WHY IT IS BANKED: it is the first body in the campaign to carry BOTH target register
 * seats at the same time -- QTYDBG reg74 (the +2 halfword value) birth 34 death 38
 * got=4 ($a0) AND reg75 (p10) birth 40 death 48 got=5 ($a1) -- with three 0x10 loads at
 * 66 instructions.  It therefore DISPROVES the s15 mechanism claim that the $a0 seat
 * requires `D_800A3478 = outer + 0x18;` specifically inside the +2 read's range: ANY
 * statement that creates a short-lived, high-priority $v0 pseudo there does it.  The
 * s15 statement is true of the STATEMENT SET s15 tried, not of the mechanism.
 *
 * DEAD AS SPELLED (s16): the moved copy's own three instructions schedule at 21/23/26
 * instead of target's 16/18/20, costing about nine score points.  Every variant
 * measured -- L2, L3, L8 (which copy is moved) and V1, V2, V5, V6, V7, V8, V9, VA
 * (P / S4 / S5 / S7 positions around it) -- measures 10-17.
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
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
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
