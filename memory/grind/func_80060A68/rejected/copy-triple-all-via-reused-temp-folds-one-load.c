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
    s32 temp_a1;
    s32 result;
    s32 cp;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    /* FAKE: pointer ctx->unk10 staged through temp_a1 (dead until its real
       assignment below reads through and overwrites it), mechanism: sched.c
       load-late launch priority for reg_n_sets==1 destinations sinks a fresh
       single-set local's load to slot 23/26 (measured) or floats it into the
       slot-5 delay slot when hoisted above the D_800F10D0 store; target has it
       at slot 12, lever-exhaustion: memory/grind/func_80060A68/evidence.md s1 */
    temp_a1 = *(s32 *)(outer + 0x10);
    cp = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x20) = *(s32 *)(cp + 0);
    cp = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(cp + 4);
    cp = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x28) = *(s32 *)(cp + 8);

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
