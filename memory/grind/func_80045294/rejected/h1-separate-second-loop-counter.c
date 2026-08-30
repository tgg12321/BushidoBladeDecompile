/* REJECTED s48: score 2 -> 37, build_insns 80 (target 83). H1 chassis
 * (i = a0 BEFORE v1 = a0 << 4) PLUS a distinct second-loop counter `j`
 * instead of reusing `i`. Motivated by cse.c:844-857 (make_regs_eqv): the
 * copy destination displaces the source as qty_first_reg -- and therefore
 * becomes canon_reg's substitution target -- only when BOTH
 *   (1) the destination's live range leaves the current cse EBB, AND
 *   (2) uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]].
 * Splitting the two loop counters was the one C-level way to falsify (2)
 * without emitting an instruction (i's last use moves back to the end of
 * loop 1, a0's stays at the loop-2 init). It DOES falsify (2), but the
 * shortened live range also re-rotates the whole callee-save allocation
 * (a0 lands in $s1, the loop counter in $s2) and lets loop 2's
 * `lw D_800A33AC` hoist out of the loop, DELETING 3 instructions. The
 * 83-insn target shape depends on the SINGLE reused counter variable.
 * KILLS the "fourth reference to a0" / "i is not a value-copy of a0"
 * frontier axis at the source level: every falsification of cse.c's
 * condition (2) that costs no instruction also shortens i's range, and
 * shortening i's range is what costs the 3 instructions. */
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
        s32 j;

        gpu_DrawSync(0);
        func_800520B8(s4, s5, sum);

        j = a0;
        if (j < D_800A33AC) {
            v1 = j << 4;
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
                j += 1;
            } while (j < D_800A33AC);
        }
    }

    D_800A33A0 += a1;
    D_800A33A4 -= a1;
}
