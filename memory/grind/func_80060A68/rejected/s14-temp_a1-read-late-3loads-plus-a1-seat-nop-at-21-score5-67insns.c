/* [s14 2026-09-03 - solver modality]  score 5 / build 67 / target 66, THREE
 * lw ?,0x10($v1) loads, and the +4 pointer seated in $a1 -- target's register.
 * This is the s11 "x2" class re-derived from ordinary statement order alone (no
 * value local, no invented carrier): the ONLY difference from the 4/66 d8 body
 * is that `temp_a1 = *(u16 *)(p10 + 4);` sits AFTER the 0x1A store instead of
 * right after the 0x18 store.
 *
 * WHY THE SEAT FLIPS (measured, QTYDBG/FFR, tmp/grind/func_80060A68/s14/):
 *   d8   (temp_a1 read early): reg75 = qty10, birth 34 death 38, refs 2,
 *        span 4  -> pri 20000/4  = 5000 -> allocation call #19 of 25 -> the
 *        only overlapping already-allocated quantity is qty0 ($v1), so
 *        first-fit hands it $v0.
 *   P2A1 (temp_a1 read late) : reg75 = qty11, birth 36 death 46, refs 2,
 *        span 10 -> pri 20000/10 = 2000 -> allocation call #24 of 25 -> by
 *        then qty9/qty10 hold $v0 and qty8/qty12 hold $a0 across [36,46], so
 *        first-fit is forced past {$v0,$v1,$a0} and hands it $a1.
 * The s13 escape arithmetic was run in the wrong direction: it asked how to
 * RAISE pri(p10) to 5000 to win $a0 (and every reference lift self-cancels
 * against the span it inflates).  The seat this function actually needs is
 * $a1, which wants pri LOW, and lowering it is free -- a statement move, not
 * an added reference.
 *
 * WHAT IT STILL COSTS.  67 insns: slot 21 is a `nop` where target has the
 * third `lw $a0,0x10($v1)` filling the +0 read's load-delay slot, and the +4
 * pointer load lands at slot 24 instead of target's slot 11.
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
