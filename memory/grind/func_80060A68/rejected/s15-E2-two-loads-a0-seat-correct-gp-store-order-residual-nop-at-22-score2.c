/* [s15 2026-09-03 - forensics]  score 2 / build 66 / target 66, TWO lw ?,0x10($v1) loads.
 * A NEW score-2 CLASS, structurally closer to target than the old candidate.c body:
 * its ONLY two differing slots are
 *     22  ours nop                vs  target lw $a0,0x10($v1)
 *     24  ours lhu $a0,2($a1)     vs  target lhu $a0,2($a0)
 * i.e. everything else - including the $a0 seat for the +2 halfword value and target's
 * slot 25/26/27 order `addiu v0,v1,24 / sw v0,%gp(D_800A3478) / sh a0,0x1A(v1)` - is
 * byte-identical.  The whole residual is the MISSING THIRD 0x10 load: p10 and the +2
 * read's pointer are cse-folded into one $a1 load, so the +2 read addresses $a1 and
 * slot 22 has nothing to fill the +0 read's load-delay slot.
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
    p10 = *(s32 *)(outer + 0x10);
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
