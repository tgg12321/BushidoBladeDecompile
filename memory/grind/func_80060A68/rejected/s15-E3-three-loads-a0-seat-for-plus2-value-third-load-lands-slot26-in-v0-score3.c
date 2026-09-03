/* [s15 2026-09-03 - forensics]  score 3 / build 66 / target 66, THREE lw ?,0x10($v1)
 * loads.  BEST 3-LOAD BODY IN THE CAMPAIGN (s14's W5 was 5/66).
 *
 * WHAT IT BUYS.  Placing `D_800A3478 = outer + 0x18;` BETWEEN the +2 halfword read and
 * the 0x1A store makes the addiu quantity (refs 2, span 2, local-alloc priority 10000)
 * live INSIDE the +2 value pseudo's range, so the addiu is allocated first, takes $v0,
 * and the +2 value falls through first-fit to $a0 - target's register.  Slots 20-27 are
 * then target's own stream one slot early: lhu v0,0(a0) / lw a0,16(v1) / sh v0,24(v1) /
 * lhu a0,2(a0) / addiu v0,v1,24 / sw v0,gp / sh a0,26(v1).
 *
 * WHY IT IS STILL 3.  The third 0x10 load (p10's) is emitted at slot 26 in $v0 instead
 * of at slot 11 in $a1, so slots 11-13 shift and `lhu a1,4(v0)` replaces
 * `lhu a1,4(a1)`.  Getting that load early needs the p10 statement placed before a
 * store that invalidates cse (see the s15 evidence entry) - and every such placement
 * measured 67 instructions (A/B/C families, 30 bodies).
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
    p10 = *(s32 *)(outer + 0x10);
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
