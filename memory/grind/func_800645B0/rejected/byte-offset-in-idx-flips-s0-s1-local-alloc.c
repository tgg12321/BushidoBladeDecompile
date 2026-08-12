/* REJECTED — session 2 (structural), sweep 9 variant DA.  Score 12 / 78 insns
 * (worse than the session-2 floor of 1).
 *
 * WHAT IT GETS RIGHT: this form closes BOTH of the residuals session 2 started
 * from.  `wid = idx2 + idx;` has a destination distinct from `idx`, so
 * expand_binop does not swap the commutative pair and the sum emits
 * `addu ?,s1,s0` (target order); and `idx = wid << 2;` is a real second set of
 * `idx`, so the birthing_insn_p bonus does not fire and the loop top emits
 * `addu` first.  The unmasked diff confirms both (no li/addu swap, no operand
 * swap).
 *
 * WHY IT IS DEAD: routing the BYTE OFFSET through `idx` makes that value part
 * of a multi-block pseudo, and that hands the $s0/$s1 assignment to the wrong
 * variable — the build emits idx=$s1 / idx2=$s0 where the target has
 * idx=$s0 / idx2=$s1, renaming 12 points.  Mechanism, from the .lreg/.greg
 * dumps (tmp/grind/func_800645B0/s2/dump_DA/):
 *   - local_alloc runs BEFORE global_alloc and allocates block-local pseudos.
 *   - In the winning shape the byte offset is a BLOCK-LOCAL CSE temp with a
 *     high allocno priority (floor_log2(n_refs)*n_refs/live_length,
 *     global.c allocno_compare), so it claims $s0 first; the block-local
 *     `idx2`, which overlaps it, is pushed to $s1; and the multi-block `idx`
 *     then re-uses $s0 because its live range does not overlap the temp's.
 *   - With the byte offset living in `idx`, that early claimant disappears,
 *     `idx2` takes $s0, and `idx` conflicts with it and takes $s1.
 * Declaration order does not move this (sweep 10: EA/EB/EC/ED/EE all 12/78),
 * confirming session 1's H7 kill in a second shape.
 */
extern s32 rand(void);
extern void *D_800A347C;
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
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            j += 1;
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
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
