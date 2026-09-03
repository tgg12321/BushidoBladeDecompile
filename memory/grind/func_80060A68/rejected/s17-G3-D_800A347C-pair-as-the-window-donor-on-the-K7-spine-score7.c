/* [s17 2026-09-03 - rederive modality.  score 7 / build 66 / target 66.
 * THIS CLOSES THE s16 FRONTIER'S NAMED NEXT PROBE.  s16 asked for the D_800A347C addiu/store
 * pair (S7) to be swept through the window between the +2 read and the 0x1A store on the
 * spine that already has the slot-11 load.  s17 measured it in seven placements - G2, G3, G4,
 * G5, GA, HC, HD, HE - and every one scores 7-9.  S7 is a gp store exactly like S5, so it
 * buys the $a0 seat by the same local-alloc mechanism AND blocks the p10 load by the same
 * sched2 mechanism; it also drags target's slots 30/31 addiu/sw %gp(D_800A347C) pair up into
 * the 25/26 window where target puts the D_800A3478 pair.  The window donor cannot be either
 * gp store if the slot-11 load is to survive.
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
    D_800A347C = outer + 0x20;
    D_800A3478 = outer + 0x18;
    p10 = *(s32 *)(outer + 0x10);
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    idx = *(u16 *)outer;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
