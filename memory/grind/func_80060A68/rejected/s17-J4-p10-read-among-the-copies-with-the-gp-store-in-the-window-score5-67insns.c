/* [s17 2026-09-03 - rederive modality.  score 5 / build 67 / target 66.
 * Places the p10 read among the copy statements (so a copy store is its cse separator and it
 * precedes both gp stores) while the D_800A3478 gp store stays inside the +2 value's live
 * range.  Both seats are bought - the +2 value in $a0, p10 in $a1 - and there are three
 * loads, but p10's load is emitted at slot 24 rather than slot 11 and a nop appears at slot
 * 21 where target puts `lw $a0,0x10($v1)`.  J5, J6, J7 are the same class (5-7 / 67).
 * Being textually early is NOT sufficient for the slot-11 hoist; K7/J8/M2 show the hoist
 * happens when p10 wins $a1 AND no gp store precedes its read AND the +2 value is left in
 * $v0 or the copy region is untouched.
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
    p10 = *(s32 *)(outer + 0x10);
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
