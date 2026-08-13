/* func_800645B0 -- REJECTED chassis "OA" (grind session [s8], rederive, 2026-08-12).
 * Measured 3 / 79 with `sandbox func_800645B0 --disable all` (target 78 insns).
 *
 * WHY IT IS INTERESTING (and why it is banked rather than merely discarded).
 * This is the FIRST body in the whole grind that gets the inner loop's top
 * region exactly right WITHOUT any reg_n_sets / scheduling lever at all:
 * unmasked indices 11, 12 and 65 -- the three points that are the entire
 * residual of the JD and CA chassis -- are all GONE, and the *3 sum's
 * commutative operand order is correct too.  It does that structurally:
 *
 *   - the slot index is MAINTAINED: computed once per group before the inner
 *     loop (`idx = i + j;`) and updated at the inner loop's bottom after
 *     `j += 1;`, so `reg_n_sets[idx] == 2` by construction and sched.c's
 *     birthing_insn_p lift can never fire on it;
 *   - the bottom update is the insn reorg.c moves into the back-edge delay
 *     slot, which is exactly what the target has at 0x800646B4;
 *   - and the group-top `idx = i + j;` is NOT constant-folded to
 *     `move $s0,$s3` (the defect that killed every earlier maintained-index
 *     shape -- session 3's H15) because `j`'s reset lives in the outer loop's
 *     UPDATE block rather than in the group-top block, so `j` has two reaching
 *     definitions where the sum is expanded.
 *
 * WHY IT IS DEAD.  Exactly one instruction over target, and the extra insn is
 * structural, not incidental: `j = 0` is emitted TWICE (once in the prologue
 * from the initialiser, once in the loop-tail block from the update), where the
 * target emits `addu $a0,$zero,$zero` exactly ONCE, at the top of the outer
 * loop body.  Six placements were measured (sweep28: OA/OB/OC/OD/OE/OF, plus
 * PA with the reset in the loop TEST and PB with it in the update and no
 * separate initialiser) and every one of them pays that duplicate: any
 * placement outside the group-top block is duplicated by GCC's loop-exit-test
 * duplication or needs a pre-loop initialiser, and the only placement that is
 * NOT duplicated -- the natural `for (j = 0; j < 4; j++)` init, i.e. inside the
 * group-top block itself -- is precisely the one that lets cse fold `i + j`
 * back to `move $s0,$s3`.
 *
 * The consequence for the search is recorded as [s8] in hypotheses.md: the
 * target's `addu $s0,$s3,$a0` at 0x800645DC is NOT a source-level group-top
 * statement (no C spelling can produce it in a block that also zeroes `j`), so
 * it is reorg.c's non-own-thread COPY of the inner loop's first insn -- and the
 * original therefore did carry `reg_n_sets[idx] >= 2` from a set that costs no
 * instruction.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    j = 0;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4, j = 0) {
        idx = i + j;
        do {
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                last = rand();
                *((s32 *)(((s32)(&D_800F0D78)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + (idx << 1))) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
            j += 1;
            idx = i + j;
        } while (j < 4);
    }
    return 1;
}
