/* [s17 2026-09-03 - rederive modality.  score 3 / build 66 / target 66.  Representative of
 * the A/B/C families: fifteen bodies that REDECOMPOSE the three 0x10 reads rather than
 * permuting the existing twelve statements.
 *
 * A-family: no p10 local at all - the +4 read is written inline as
 * `temp_a1 = *(u16 *)(*(s32 *)(outer + 0x10) + 4);` (S6d).
 * B-family: p10 is retained but feeds the +0 read instead of the +4 read.
 * C-family: p10 feeds the +2 read instead of the +4 read.
 *
 * RESULT (all fifteen): 3-8 at 66-68 instructions, and in NONE of them does any
 * `lw ?,0x10($v1)` reach slot 11.  The third load lands at 18/21/26-28 with the late one in
 * $v0 or $a0.  Target's slot-11 load is a LONG-LIVED pointer pseudo (born at slot 11, dying
 * at the `lhu $a1,0x4($a1)` at slot 28), and only a source-level pointer local read well
 * before its halfword use creates a pseudo with that shape.  The +4 read written inline
 * creates a two-insn pointer pseudo instead, which local-alloc seats in $v0/$a0 and sched2
 * never hoists.  The p10 local is therefore LOAD-BEARING, not a spelling choice.
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
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
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
