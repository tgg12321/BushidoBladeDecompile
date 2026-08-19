/* REJECTED (s4, 2026-08-19, forensics modality)
 * s4 probe x4. Carrier for copy 2's address load = the pre-existing `result` local (which already carries copy 1's staged pointer/word). sandbox = 4 at 67 insns. Disassembly: copy 2's address load is not hoisted AT ALL (slots 10-12 are lw v0,12 / lw a1,16 / lw v0,0(v0), and copy 2's address load appears at slot 14) because `result` is $v0 and copy 1's loaded word is still live in $v0 until the 0x20 store, so the anti-dependence pins the second set below that store.
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
    s32 temp2;
    s32 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    result = *(s32 *)(outer + 0xC);
    result = *(s32 *)(result + 0);
    *(s32 *)(outer + 0x20) = result;
    result = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(result + 4);
    temp_a1 = *(s32 *)(outer + 0x10);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(temp_a1 + 4);
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
