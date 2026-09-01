/* s13 (2026-09-01, escalation modality) -- REFS-LIFT PROBE B: the same two extra
 * references placed EARLY (immediately after `p10 = *(s32 *)(outer + 0x10);`) to
 * try to buy refs without moving qty8's death point.
 *
 * MEASURED (local_extract QTYDBG, blk 0): qty8 reg1=75 birth=26 death=54 refs=4
 * got=5 ($a1).  Placing the references early does NOT protect the span: the added
 * instructions push the temp_a1 read (qty8's death) six index units further out,
 * and cse collapses one of the two new base references, so the body pays span
 * 18 -> 28 for a single ref (pri = 2*4*10000/28 = 2857, WORSE than baseline 1666's
 * competitor threshold gap).  Sandbox: score 6 / build 72 / target 66 -- the six
 * extra instructions are six guaranteed residual slots on a body that already
 * matches target's instruction count exactly.  DEAD.
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
    *(u16 *)(outer + 0x1E) = *(u16 *)(p10 + 6);
    *(u16 *)(outer + 0x2C) = *(u16 *)(p10 + 8);

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
