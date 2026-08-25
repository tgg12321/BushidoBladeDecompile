/* [s10 2026-08-25 - escalation/disposition]  MIRRORED PARTITION OF candidate.c.  score 2 /
 * build 66 / target 66 - the SAME floor as candidate.c, but the two differing instructions
 * move: here p10's hoisted load (slot 11, $a1) serves the +2 read and the +4 read, and the
 * +0 read gets the fresh load (slot 19, $a0).  Result vs target: line 22 `lhu v0,0(a0)` is
 * now EXACTLY target (candidate.c emits `lhu v0,0(a1)` there), and the defect moves to line
 * 26, `lhu a0,2(a1)` where target has `lhu a0,2(a0)`, plus the same missing third
 * `lw a0,16(v1)` at line 23 (a nop in both bodies).  w2 (the +2 read spelled inline as
 * *(u16 *)(*(s32 *)(outer + 0x10) + 2) instead of *(u16 *)(p10 + 2)) is BYTE-IDENTICAL to
 * this body - cse folds the inline re-read onto p10 either way.
 *
 * WHY IT IS BANKED AS REJECTED, NOT AS THE CANDIDATE: it does not lower the floor (2), and it
 * demonstrates the s10 conservation argument - whichever of the three 0x10 reads is chosen as
 * p10's second consumer, that read loses its own load and takes p10's register, so exactly two
 * instructions are always wrong.  Measured with tmp/grind/func_80060A68/s10/run2.ps1,
 * diffed with tmp/grind/func_80060A68/s10/cmp.py.
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
    u16 temp0;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    temp0 = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    *(u16 *)(outer + 0x18) = temp0;
    p10 = *(s32 *)(outer + 0x10);
    temp2 = *(u16 *)(p10 + 2);
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
