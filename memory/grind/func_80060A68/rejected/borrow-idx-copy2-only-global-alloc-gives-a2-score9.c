/* REJECTED (s4, 2026-08-19, forensics modality)
 * s4 probe x1. Carrier for copy 2's address load = the pre-existing `idx` local, copy 2 ONLY (K11's v44/v46 shared idx with copy 3 as well; this isolates the copy-2 borrow).
   sandbox --disable all = 9, 67 insns.
   FORENSICS (tmp/grind/func_80060A68/s4/x1/text1b.lreg,.greg): the borrow WORKS at the scheduler -- slots 10-13 emit lw v0,12 / lw a2,12 / lw a1,16 / lw a0,12, i.e. copy 2's address load is unbumped and lands ahead of the staged 0x10 read exactly as the birthing_insn_p model predicts. It fails in REGISTER ALLOCATION: merging copy 2's pointer into idx makes pseudo 73 live from the copy block to the call, local-alloc declines it (6 uses across 17 insns, dies in 2 places), and global-alloc records `73 conflicts: 73 2 3 4 5 29` -- a conflict with hard reg 4 ($a0) -- so it is assigned reg 6 ($a2) and the body costs an extra insn. In the score-0 candidate the same slot's pseudo 73 is local-alloc'd straight to $a0 (`73 in 4`).
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
    idx = *(s32 *)(outer + 0xC);
    *(s32 *)(outer + 0x24) = *(s32 *)(idx + 4);
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
