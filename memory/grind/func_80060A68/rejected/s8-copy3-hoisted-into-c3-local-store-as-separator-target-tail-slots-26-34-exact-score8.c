/* s8 2026-08-19 - structural.  u5 (score 8 / 66 insns).
 *
 * THE FIRST BODY IN THE CAMPAIGN THAT REPRODUCES TARGET'S TAIL.  Slots 26 through 34 are
 * byte-identical to target, INCLUDING the two instructions the whole campaign has been
 * chasing at the far end: `lhu $a1,0x4($a1)` at slot 29 (p10's consumer, in $a1, seventeen
 * slots after its load) and `lhu $a0,0x0($v1)` at slot 30 (the late `idx` read).  It also
 * has `lw $a1,0x10($v1)` at slot 12 and three 0x10 loads.  Together with
 * s8-idx-hoisted-...-score8.c (byte-identical PREFIX through slot 22) it brackets the answer
 * from both ends: the two bodies' correct regions are complementary and their union is target.
 *
 * THE CHANGE.  Copy 3's VALUE is hoisted into a named `c3` local read ABOVE copy 2, and
 * `*(s32 *)(outer + 0x28) = c3;` is then used as the cse separator between the `p10` read and
 * the 0x18 halfword read.  This is the third independent way found this session to keep a
 * pseudo alive in $v0 across insns 39..56 and so push p10's pseudo off $v0 onto $a1; the other
 * two are the `idx` hoist (f3 family) and simply sinking copy 3's store below the +2 read
 * (m1 9/66, n2 10/67, o4 and o5 10/66).
 *
 * WHY IT IS 8.  All eight wrong slots are in 17..25 and they are copy-triple damage: hoisting
 * the c3 read above copy 2 lets cse keep copy 2's base in $a0, so copy 3 is emitted as
 * `lw $a0,0x8($a0)` / `sw $a0,0x28($v1)` instead of target's `lw $a0,0xC($v1)` /
 * `lw $v0,0x8($a0)` / `sw $v0,0x28($v1)`, and one extra `lw $a1,0x10($v1)` appears at slot 24.
 * Keeping the c3 read BELOW copy 2 (u4 / u7 / u8 / ub) repairs the triple but costs a 67th
 * instruction and moves the $a1 load back to slot 24 (scores 5 to 7).
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
    s32 c3;
    u16 temp2;
    s32 p10;
    u16 temp_a1;
    s32 result;

    outer = D_800A3468;
    *(s32 *)((s32)&D_800F10D0 + *(u16 *)outer * 4) = 0;
    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    c3 = *(s32 *)(*(s32 *)(outer + 0xC) + 8);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    p10 = *(s32 *)(outer + 0x10);
    *(s32 *)(outer + 0x28) = c3;
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
