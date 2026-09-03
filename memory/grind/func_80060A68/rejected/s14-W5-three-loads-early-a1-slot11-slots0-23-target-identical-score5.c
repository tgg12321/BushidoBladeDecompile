/* [s14 2026-09-03 - solver modality]  score 5 / build 66 / target 66, THREE
 * lw ?,0x10($v1) loads, and SLOTS 0-23 BYTE-IDENTICAL TO TARGET -- including
 * `lw $a1,0x10($v1)` at slot 11, the early $a1 pointer load that thirteen
 * prior sessions recorded as unreachable in pure C without a multiply-set
 * carrier.  It is reached here by ordinary statement order alone: no invented
 * local, no second write to any variable, no dead code, no volatile, no asm.
 *
 * THE TWO MOVES THAT DO IT (from the 4/66 d8 body):
 *   1. `temp_a1 = *(u16 *)(p10 + 4);` AFTER the 0x1A store, not before it.
 *      This lengthens p10's live range from span 4 to span 10, dropping its
 *      local-alloc priority from 5000 to 2000 so it is allocated LAST, and
 *      first-fit is then forced past {$v0,$v1,$a0} into $a1 (QTYDBG/FFR,
 *      tmp/grind/func_80060A68/s14/qtydbg_P2A1.txt).
 *   2. `idx = *(u16 *)outer;` BEFORE the D_800A3478 store, not after the +4
 *      read.  Without this the same body is 67 insns (a nop at slot 21) or
 *      loses the seat back to $a0 (V1, score 6).
 *
 * THE REMAINING RESIDUAL IS SLOTS 24-29 ONLY (5 insns):
 *   ours   24 lhu v0,2(a0)  25 lhu a0,0(v1)  26 sh v0,26(v1)
 *          27 addiu v0,v1,24  28 sw v0,0(gp)  29 lhu a1,4(a1)
 *   target 24 lhu a0,2(a0)  25 addiu v0,v1,24  26 sw v0,%gp(D_800A3478)
 *          27 sh a0,26(v1)  28 lhu a1,4(a1)  29 lhu a0,0(v1)
 * i.e. the +2 read must land in $a0 (not $v0), its 0x1A store must be
 * deferred past the D_800A3478 gp store, and the idx read must fall to slot 29
 * instead of being hoisted into the +2 read's load-delay slot at 25.
 *
 * MEASURED NEIGHBOURS (all 3-load, this session): X2/X3/X5/X6 = 5/66 (same
 * class, idx read one statement earlier / temp2 written inline / p10 before
 * copy 2); V1,V3 = 6/66; W1,P2A1 = 5/67; Y1,Y2,Y7 = 8/67; Y3 = 7/67;
 * Y5 = 11/68; Y6 = 10/68.  Every attempt to put the D_800A3478 store back
 * between the +2 read and the 0x1A store (target's apparent order) costs the
 * instruction back.
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
    *(u16 *)(outer + 0x1A) = temp2;
    idx = *(u16 *)outer;
    D_800A3478 = outer + 0x18;
    temp_a1 = *(u16 *)(p10 + 4);
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
