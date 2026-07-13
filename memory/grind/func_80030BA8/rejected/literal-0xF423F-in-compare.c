/* REJECTED — sandbox --disable all = 6 (87/87 insns, pure sched/reg diff).
 * The Judge's first prescribed remediation for `new_var`: drop the holder and
 * put the literal straight in the comparison. sched1 does NOT lift the lui/ori
 * into the mult/mflo latency window on its own — the constant needs to be a
 * distinct pseudo with its own set-insn for the list scheduler to move it.
 * KILLS the s1 frontier hypothesis "literal 0xF423F still matches".
 *
 * The WORKING replacement is a block-local named const in the same block
 * (`s32 range_sq = 0xF423F;`) -> score 0. Function-scope lifetime was never
 * load-bearing; the single-pseudo identity is.
 */
        {
            s32 dx = *(s32 *)(arg0 + 0xF4) - *(s32 *)(p + 0x2A);
            s32 dz = *(s32 *)(arg0 + 0xFC) - *(s32 *)(p + 0x32);
            i++;
            if (dx * dx + dz * dz > 0xF423F) {
                goto loop_test;
            }
        }
