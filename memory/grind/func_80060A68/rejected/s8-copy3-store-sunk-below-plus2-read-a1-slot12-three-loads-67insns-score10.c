/* s8 2026-08-19 - structural.  n2 (score 10 / 67 insns).
 *
 * Banked as the CLEANEST demonstration of the register-pressure lever, independent of any
 * named local.  Statement order is copy 1, the `p10` read, copy 2, the 0x18 halfword store,
 * the p10 consumer, the +2 read, THEN copy 3.  Simply sinking copy 3's store below the +2
 * read is enough to keep a value live in $v0 across p10's sched1 live range, so local-alloc
 * gives p10's pseudo $a1 and sched2 hoists its load to slot 12 - `lw $a1,0x10($v1)` - with all
 * three 0x10 loads present.  The prefix is byte-identical to target through slot 16.
 *
 * It is banked as REJECTED because the price is the copy triple: with copy 3 sunk, target's
 * slots 17..21 (`lw $a0,0xC($v1)` / `sw $v0,0x24($v1)` / `lw $v0,0x8($a0)` / `lw $a0,0x10($v1)`
 * / `sw $v0,0x28($v1)`) are re-ordered and a 67th instruction appears.  Sibling measurements:
 * m1 (copy 3 sunk one statement less far) = 9 at 66 insns but only two 0x10 loads; o4 and o5
 * (copy 3 via a `c3` local with no separator between the p10 read and the 0x18 read) = 10 at
 * 66 insns, also two loads.
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
    p10 = *(s32 *)(outer + 0x10);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp_a1 = *(u16 *)(p10 + 4);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
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
