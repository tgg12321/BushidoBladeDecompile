/* SESSION 16 (2026-09-01, forensics) -- 12 / 78 at 78 insns, and every single
 * difference is a REGISTER NAME.  Second real write to idx = the byte-offset
 * scaling (`idx = wid << 2;`), which is a non-copy insn and so survives to
 * sched (reg_n_sets[idx] = 2, no birthing lift).  The emitted stream has the
 * target's instruction sequence exactly -- 78 insns, same opcodes, same order,
 * loop head and delay slot correct -- under a seat permutation:
 *   11 addu s1,s3,a0 | TGT addu s0,s3,a0      19 sll s0,s1,1 | TGT sll s1,s0,1
 *   20 addu v1,s0,s1 | TGT addu s0,s1,s0      22 sll s1,v1,2 | TGT sll s0,s0,2
 *   plus the three `addu at,at,s1` / one `addu at,at,s0` consumers and 65.
 * Cause: keeping idx live past the sum (it is rewritten immediately after)
 * denies wid the seat idx vacates in the target, so idx/idx2/wid rotate through
 * s1/s0/v1.  Kept as the RA-seat exhibit for a solver session.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
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
                wid = idx2 + idx;
                idx = wid << 2;
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
