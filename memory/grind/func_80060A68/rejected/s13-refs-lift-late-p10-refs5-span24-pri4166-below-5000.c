/* s13 (2026-09-01, escalation modality) -- REFS-LIFT PROBE A: two extra references
 * to p10 placed AFTER the temp_a1 read (i.e. after qty8's previous death point).
 *
 * PURPOSE: test the s13 escape law derived from the QTYDBG ground truth --
 * local-alloc gives qty8 (the +0x10 pointer, reg1=75) $a0 only if its priority
 * fl(refs)*refs*size*10000/span reaches 5000 (the qty10 group; qty 8 < qty 10 wins
 * a tie).  Baseline is refs=3 span=18 -> 1666.  refs=5 needs span <= 20.
 *
 * MEASURED (local_extract QTYDBG, blk 0): qty8 reg1=75 birth=26 death=50 refs=5
 * got=6 ($a2).  The two extra references cost SIX index units of span (18 -> 24),
 * so pri = 2*5*10000/24 = 4166 -- still below 5000, and qty8 is still allocated
 * after qty10.  The lift is self-cancelling: every added reference is an added
 * instruction, and an added instruction inflates the same span it is meant to
 * outrun.  DEAD.
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
    p10 = *(s32 *)(outer + 0x10);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    temp_a1 = *(u16 *)(p10 + 4);
    *(u16 *)(outer + 0x1E) = *(u16 *)(p10 + 6);
    *(u16 *)(outer + 0x2C) = *(u16 *)(p10 + 8);
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
