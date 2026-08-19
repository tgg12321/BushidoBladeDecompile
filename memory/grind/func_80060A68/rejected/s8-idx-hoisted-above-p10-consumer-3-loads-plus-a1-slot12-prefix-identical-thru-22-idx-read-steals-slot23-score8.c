/* s8 2026-08-19 - structural.  f3 / i3 / i4 / r1 (all byte-identical, score 8 / 66 insns).
 *
 * WHY THIS IS BANKED AS THE MOST IMPORTANT NEGATIVE OF THE SESSION: it FALSIFIES s10's
 * "conservation law".  s10 wrote that three independent `lw ?,0x10($v1)` loads and an
 * un-pinned p10 load were mutually exclusive ("3 loads <=> p10 has no early consumer <=>
 * sched2 T-30 pin; early consumer <=> one load CSE-folded <=> a nop").  This body has BOTH:
 * all three 0x10 loads AND target's `lw $a1,0x10($v1)` at slot 12 (1-based), at 66
 * instructions, and its prefix is BYTE-IDENTICAL TO TARGET THROUGH SLOT 22 - eleven slots
 * deeper than cb (which diverges at slot 11) and covering the entire copy triple.
 *
 * THE CHANGE.  cb's statement order with ONE addition: the `idx = *(u16 *)outer;` read is
 * hoisted from its late seat to a position strictly BEFORE the p10 consumer statement
 * (f3: between the copy-3 store and the 0x18 store; i3 and i4 and r1 are the neighbouring
 * seats and compile identically).
 *
 * THE MECHANISM (dump-verified, .lreg + .greg, not guessed).  `p10` is pseudo 75 and it is
 * assigned by LOCAL-ALLOC, not global-alloc (text1b.greg for this function says "1 regs to
 * allocate: 83", and the dispositions line reads `75 in 2`, i.e. $v0).  In cb, sched1 leaves
 * insn 39 (p10's load) immediately before insn 56 (its consumer), so reg75's live range spans
 * only insn 59, and $v0 is dead across that window - it is killed at insn 53, `sh $v0,0x18($v1)`.
 * local-alloc's lowest-free-hard-reg scan therefore hands reg75 $v0, and because $v0 is written
 * at slot 20 and read at slot 22 in the final schedule, sched2 cannot hoist the load out of
 * slot 23.  Hoisting the `idx` read keeps a pseudo alive across insns 39..56, $v0 is no longer
 * free, reg75 gets $a1 (dead from slot 12 to slot 28), and sched2 hoists the load to slot 12
 * exactly as in target.  This is frontier item 2 of the s7 ledger, CONFIRMED by measurement.
 *
 * WHY IT IS STILL 8 AND NOT 0.  The hoisted `idx` load has no dependences beyond $v1, so
 * sched2 schedules it into slot 23 - the load-delay slot after `lhu $v0,0x0($a0)` - which is
 * the slot target gives to the THIRD `lw $a0,0x10($v1)`.  Everything from slot 23 to slot 31
 * is then a permutation of target: f3 emits (idx load, sh, third 0x10 load, lhu a1,4(a1),
 * lhu a2,2(v0), addiu, sw gp, addiu, sh a2,26) where target emits (third 0x10 load, sh,
 * lhu a0,2(a0), addiu, sw gp, sh a0,26, lhu a1,4(a1), idx load, addiu).
 *
 * SEAT SWEEP THAT ISOLATES THE LEVER.  All twelve source seats for the `idx` read were
 * measured on the cb order (i0..i11).  `lw $a1,0x10($v1)` at slot 12 appears at seats 2,3,4,5
 * - exactly the seats strictly before the p10 consumer statement - and at no other seat.
 * Seats 0 and 1 over-shoot ($a2, score 9); seats 6 and later leave reg75 in $v0 (scores 4-8,
 * cb itself is seat 9 at score 4).
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
    idx = *(u16 *)outer;
    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    temp_a1 = *(u16 *)(p10 + 4);
    temp2 = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = temp2;
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;
    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
