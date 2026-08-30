/* s12 2026-08-30: THE THIRD AND LAST PARTITION of the three 0x10 reads, never measured before this session. p10 serves the +0 and +2 reads (both spelled *(u16 *)(p10 + k), so no cse fold is involved at all) and the +4 read gets its own load because the 0x18/0x1A stores separate it. 4 / 66 -- and the disassembly (tmp/grind/func_80060A68/s12/c1.dis) shows the SAME pathology as the 3-load body d8: the +4 address load is emitted at slot 27 in $v0, not at slot 12 in $a1. Variants: c2 (no temp2 local) 7/66, c3 (idx read hoisted above the +4 read) 5/66. This CLOSES the partition axis: {+0,+4} = base 2/66, {+2,+4} = s10 w1/w2 2/66, {+0,+2} = 4/66; in every partition the load that is NOT shared with an early consumer is emitted late in $v0. */
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
    p10 = *(s32 *)(outer + 0x10);

    *(u16 *)(outer + 0x18) = *(u16 *)(p10 + 0);
    temp2 = *(u16 *)(p10 + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(*(s32 *)(outer + 0x10) + 4);
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
