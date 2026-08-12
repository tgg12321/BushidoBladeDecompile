/* func_800645B0 -- CHASSIS "JD" (grind session 7, rederive modality, 2026-08-12).
 * NOT the shipped candidate: this scores 3 / 78, against the standing floor of
 * 1 held by candidate.c (the "SB" body).  It is banked because it is the most
 * structurally promising body the grind has produced, and the next session
 * should start its search here rather than on SB.
 *
 * WHAT IT IS.  The three word-stride destinations are consecutive words at a
 * 12-byte stride (D_800F0D78 / +4 / +8 == videoDec, undefined_syms_auto.txt
 * :467-469) and D_800F0BCC is a parallel s16 array at a 2-byte stride: the data
 * model is an array of 3-word structs plus an s16 array, and the target's
 * `sll $s1,$s0,1 / addu $s0,$s1,$s0 / sll $s0,$s0,2` is nothing but GCC's own
 * synth_mult expansion of `k * 12` sharing its `k << 1` with the halfword
 * index.  So the index arithmetic is written INLINE as expressions and there
 * are NO `idx2` / `wid` locals at all -- every intermediate is an unnamed
 * pseudo.  (The stores keep the per-symbol hand-built cast form rather than a
 * struct subscript: subscripting relocates against %lo(D_800F0D78) with
 * addends 4 and 8 and engine/score.py counts that as two differing
 * instructions even though the linked words are identical -- see
 * rejected/struct-array-subscript-lo16-addend-false-distance.c.)
 *
 * WHY IT MATTERS.  JD's ENTIRE unmasked residual is the three loop-top points
 * (11, 12, 65): every register, the *3 sum's commutative operand order and the
 * instruction count are already the target's.  It therefore reaches the CA
 * position without CA's `wid` variable and without the SB chassis' expand_binop
 * wall (H24) -- and it does so with a completely different pseudo set, which is
 * what session 6's H32 said the answer needs.  The remaining 3 points are
 * sched.c's birthing_insn_p lift on `addu idx,i,j` (reg_n_sets[idx] == 1).
 *
 * The measured way to deny that lift at ZERO instruction cost is a second set
 * of `idx` OUTSIDE the loops (sweep25 KD, 4/78, loop top exact) -- see the
 * session-8 frontier item 0 in hypotheses.md.
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
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
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
        }
    }
    return 1;
}
