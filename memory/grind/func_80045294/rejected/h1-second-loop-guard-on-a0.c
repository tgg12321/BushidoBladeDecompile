/* REJECTED s49: score 5, build_insns 84 vs target 83.  BUT the most informative
 * near-miss in the ledger, and it CORRECTS the s48 record.
 *
 * Form: H1 chassis (i = a0 BEFORE v1 = a0 << 4, which is the only source order
 * that reaches target's sched2 prologue-cluster order) PLUS the second loop's
 * guard spelled on the parameter, `if (a0 < D_800A33AC) { i = a0; ... }`,
 * instead of `i = a0; if (i < D_800A33AC) { ... }`.  Because that a0 read is
 * NOT dominated by a live `i = a0` copy (i was clobbered by loop 1), cse.c
 * cannot substitute it, so it is a genuine fourth reference to pseudo 72.
 *
 * MEASURED (tools/ra_solver/extract.py, tmp/grind/func_80045294/s49/v1.model.json):
 *   pseudo 72 nrefs_flow = 4  (H1 baseline is 3) — the exact ra_solver inverse
 *   atom [refs_up] pseudo 72: 3 -> 4 that s47 named as the one non-foreclosed
 *   RA lever.  And it WORKS: the disassembly (s49/v1_disasm.txt) shows block 0
 *   emitted as  sw $s0,0x10($sp) ; move $s0,$s2 ; sll $v1,$s0,0x4  — target's
 *   INSTRUCTION ORDER and target's REGISTER ALLOCATION exactly (a0 -> $s2,
 *   i -> $s0).  The whole callee-save rotation that defines the score-11 H1
 *   basin is gone.
 *
 * WHY IT STILL LOSES (two residuals, both newly isolated):
 *   (1) block 0's shift reads $s0 (i) where target reads $s2 (a0).  reg_n_refs
 *       is a global.c input; it does not touch cse.c's choice of which register
 *       of an equivalence quantity is canonical.  That choice is made in
 *       make_regs_eqv (tools/gcc-2.7.2/cse.c:842-857): reg 75 (i) displaces
 *       reg 72 (a0) as qty_first_reg iff uid_cuid[regno_last_uid[75]] >
 *       uid_cuid[regno_last_uid[72]].  Here i's last reference is inside loop 2
 *       and a0's last reference is this guard, which precedes loop 2 — so the
 *       inequality still holds and the ashift operand is still rewritten.
 *   (2) hoisting `i = a0` into the arm costs one instruction (84 vs 83): target
 *       emits `move $s0,$s2` BEFORE the slt and puts `sll $v1,$s0,4` in the
 *       delay slot, i.e. target's guard reads i, not a0.
 *
 * Control measured the same session (rejected/h1-i-then-guard-on-a0.c): moving
 * `i = a0;` back in front of the guard restores 83 insns but drops nrefs_flow(72)
 * to 3 — cse steals the guard's a0 as well.  The fourth reference and the
 * 83-instruction shape are mutually exclusive at this site.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 i = a0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }

    if (sum != 0) {
        s32 *ptr;
        s32 idx;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        if (a0 < D_800A33AC) {
            i = a0;
            v1 = i << 4;
            ptr = (s32 *)((u8 *)&D_800EED14 + v1);
            idx = v1;
            do {
                *ptr += a1;
                {
                    void (*fn)(s16, s32) = (void (*)(s16, s32)) *(s32 *)((u8 *)&D_800EED1C + idx);
                    if (fn != 0) {
                        fn(*(s16 *)((u8 *)&D_800EED10 + idx), a1);
                    }
                }
                ptr = (s32 *)((u8 *)ptr + 0x10);
                idx += 0x10;
                i += 1;
            } while (i < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
