/* [s17 2026-09-03 - rederive modality.  score 5 / build 67 / target 66.
 * THE CLOSEST-BY-STRUCTURE BODY THE CAMPAIGN HAS PRODUCED, and the reason this file is
 * worth reading before anything else in rejected/.
 *
 * H3 = C1,C2,C3,S1,S2,S5,S3,P,S4,S7,S6,S8.  Its disassembly is TARGET'S ENTIRE 66-INSTRUCTION
 * STREAM with exactly ONE instruction relocated: `lw $a1,0x10($v1)` is emitted at slot 26
 * instead of target's slot 11, everything between shifts one slot earlier, and a nop appears
 * at slot 32 (the lhu $a1,0x4($a1) at 31 no longer has three slots before the sh $a1,0x1C).
 * That nop is the 67th instruction.  Both register seats are TARGET'S - the +2 halfword value
 * in $a0 (reg74 got=4) and the p10 pointer in $a1 (reg75 got=5) - and all three
 * `lw ?,0x10($v1)` loads are present.  No prior body in the campaign held all of that at once
 * (s16's L1 held both seats but scheduled a moved copy triple nine points out of place).
 *
 * WHY THE LOAD SITS AT 26.  H3's p10 read is textually AFTER `D_800A3478 = outer + 0x18;`.
 * That statement's store is `sw $v0,%gp_rel(D_800A3478)($gp)` - a gp-based store, which
 * sched2 cannot disambiguate from `mem(v1+0x10)` the way it disambiguates the v1-based
 * 0x18 / 0x1A / 0x20.. stores.  The load is therefore chained behind it and is emitted in the
 * very next slot.  Every s17 body whose p10 read follows either gp store puts its load at
 * slot 26-28 (E3, G1, G3, G4, H1..H8, HC..HE, and every A/B/C body); every body whose p10
 * read precedes both gp stores and which also wins $a1 puts it at slot 11 (K7, J8, M2, M6, M7).
 *
 * DEAD AS SPELLED.  Moving P before S5 requires a non-gp store between the +2 read and the
 * p10 read to stop cse folding them, and the only such statement the function owns is a copy
 * (M1/M2/M3/M5/M6/M7, 67 insns, 10-15) - the price H-s16-4 already recorded.
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
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    p10 = *(s32 *)(outer + 0x10);
    idx = *(u16 *)outer;
    D_800A347C = outer + 0x20;
    temp_a1 = *(u16 *)(p10 + 4);
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
