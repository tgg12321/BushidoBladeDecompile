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
    s32 temp2;
    s32 p10;
    u16 h4;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    result = *(s32 *)(outer + 0xC);
    result = *(s32 *)(result + 0);
    *(s32 *)(outer + 0x20) = result;
    idx = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(idx + 4);
    p10 = *(s32 *)(outer + 0x10);
    idx = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x28) = *(s32 *)(idx + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    h4 = *(u16 *)(p10 + 4);
    temp2 = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = h4;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + temp2) + *(s32 *)((s32)&D_800F10D0 + temp2 * 4)) * 4))(temp2, h4);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
