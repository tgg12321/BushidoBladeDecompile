/* REJECTED -- s18 (structural).  `idx = idx * 12;` as the byte-offset computation (the
 * data at D_800F0D78 / D_800F0D7C / videoDec is a 12-byte-stride struct array,
 * so this is the most natural spelling of the offset).  **3 / 78, 78 insns.**
 * Index 20 is EXACT -- the *3 addu comes out `addu $s0,$s1,$s0`, the target's
 * operand order -- because expand_mult routes the add through a TEMP, so
 * optabs.c:412-419's `target == op1` commutative swap never fires.  The whole
 * residual is the WD loop-head signature (11/12 swapped, 65).
 * DUMP-PROVEN CAUSE (tmp/grind/func_800645B0/dumps/text1b.sched, insns 38/63/64
 * of func_800645B0): pseudo 74 (`idx`) is SET ONCE -- insn 38 `74 = 72 + 73`;
 * the sum and the shift land in temps 86 and 87 and are propagated into the
 * three stores, so reg_n_sets[74] == 1, sched.c:2526 birthing_insn_p lifts the
 * loop-top addu to max_priority, and the const-1 `li` takes the loop head.
 * `idx = (idx2 + idx) * 4;` (a5) measures identically for the same reason.
 * Equivalent to the WD chassis but WITHOUT the extra named local.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx * 12;
                *((s32 *)(((s32)(&D_800F0D78)) + idx)) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + idx)) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + idx)) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}