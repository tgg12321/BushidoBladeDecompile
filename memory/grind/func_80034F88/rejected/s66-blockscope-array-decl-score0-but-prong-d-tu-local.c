/* s66 (solver, 2026-09-05) -- FLOOR 2 -> 0, and the whole body is confined to
 * src/code6cac_b.c.  49/49 instructions, byte-identical to
 * asm/funcs/func_80034F88.s; full clean-driver build SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle with ONLY this file edited.
 *
 * WHAT CLOSED THE BYTES.  s65 fitted GCC 2.7.2's global.c allocation priority
 * exactly (pri = floor_log2(nrefs) * nrefs * 10000 / live_length) and reduced
 * the residual to two arithmetic branches.  Branch (A): block 0's address
 * object reaches the target seating iff it is allocated before block 0's value
 * (pri 17500), i.e. iff floor_log2(n)*n > 49, i.e. n >= 16 references; its own
 * five references at live length 28 price at 3571.  s65 measured every obvious
 * byte-neutral reference lift dead (duplicated store into arms, split reads,
 * merged mask).  The lift that is free is a variable reuse: block 0's address
 * object and the copy loop's counter are ONE variable, so the loop's eleven
 * counter references (flow.c weights by loop depth) land on the address
 * allocno, AFTER its last pointer use, so the live length rises only 14 -> 21.
 * Measured model (tmp/grind/func_80034F88/s66/z2.model.json):
 *
 *   ord0 p74 c (flag/result)      19 refs / len 21 / pri 36190 -> $v0  TARGET
 *   ord1 p76 q (address + index)  16 refs / len 21 / pri 30476 -> $v1  TARGET
 *   ord2 p75 u (block-0 value)     7 refs / len  8 / pri 17500 -> $a0  TARGET
 *   ord3 p73 v (blocks-1/2 value)  6 refs / len 10 / pri 12000 -> $v1  TARGET
 *   ord4 p80 r (blocks-1/2 addr)   6 refs / len 19 / pri  6315 -> $a0  TARGET
 *   ord5 p72 p                     6 refs / len 34 / pri  3529 -> $a1  TARGET
 *
 * Block 0's value is no longer blocked out of $v1 by a conflict (the s64 route,
 * capped at score 2 because global.c:1275 gives one allocno one hard register):
 * $v1 is simply already held by the higher-priority address/counter allocno, so
 * find_reg scans on to $a0 -- the target register -- and the loop's byte temp
 * stays a plain block-local that local-alloc seats at $v0, also as the target.
 *
 * WHY THE DECLARATION IS LOCAL.  The target's copy loop stores through
 * `lui $at,%hi(D_80106A70); addu $at,$at,$v1; sb $v0,%lo(D_80106A70)($at)` --
 * an indexed store into a three-byte array whose elements the census names
 * D_80106A70/71/72.  s59-s65 carried that shape as a split declaration in
 * include/code6cac.h, which the driver rules out of scope for this function.
 * A block-scope `extern u8 D_80106A70[3];` states the same true object shape
 * inside the only function that indexes it, needs no edit outside this file,
 * and is not a per-use pun: there is ONE declaration, and every access in this
 * function goes through it.  src/code6cac.c keeps its scalar element names.
 * The whole-project oracle build is the proof that the two views agree.
 */
void func_80034F88(void) {
    /* The real shape of this object: the target indexes it with a computed
     * register, so it is a three-byte array, not the scalar the shared header
     * declares.  Declared here because candidates for this function may only
     * edit src/code6cac_b.c. */
    extern u8 D_80106A70[3];
    s32 *p;
    s32 v;
    s32 c;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block 0's own address object (a second C handle on
         * D_80106A73), mechanism: global.c allocation priority
         * floor_log2(nrefs)*nrefs*10000/live_length -- blocks 1 and 2 cannot be
         * reached from this handle because global.c:1275 assigns exactly one
         * hard register per allocno and GCC 2.7.2 does no live-range splitting.
         * lever-exhaustion: memory/grind/func_80034F88/hypotheses.md s53-s65. */
        u8 *q = &D_80106A73;

        u = *q;
        u = u & 0xF8;
        *q = u;
        u = 0; /* FAKE: cse2 value invalidator, mechanism: cse2 (cse.c) forwards
                * the sb into the following lbu only while the stored value's
                * pseudo still holds it. lever-exhaustion: hypotheses.md s57-s62. */
        u = *q;
        c = p[8] & 1;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
        {
            /* FAKE: the address object for flag blocks 1 and 2, mechanism:
             * global.c:1275 assigns exactly one hard register per allocno and
             * GCC 2.7.2 does no live-range splitting, so blocks 1/2 cannot be
             * reached from the block-0 object. lever-exhaustion: as above. */
            u8 *r = &D_80106A73;

            v = *r;
            c = p[8] & 2;
            if (c) {
                c = v | 2;
            } else {
                c = v;
            }
            *r = c;

            r = &D_80106A73;
            v = *r;
            c = p[8] & 4;
            if (c) {
                c = v | 4;
            } else {
                c = v;
            }
            *r = c;
        }

        /* FAKE: the copy loop's counter is staged through q, whose pointer
         * value is dead from block 0's store above and is never read again,
         * mechanism: flow.c counts REG_N_REFS per RTL insn weighted by loop
         * depth, so the loop's eleven counter references lift this allocno from
         * 5 refs / pri 3571 to 16 refs / pri 30476 and global.c seats it in $v1
         * before block 0's value allocno (pri 17500) is considered, which sends
         * that value to $a0 as the target has it.  Both values are real and
         * used; the loop adds no instruction anywhere in the function.
         * lever-exhaustion: hypotheses.md s53-s65 -- s65's branch (A), whose
         * other byte-neutral spellings (duplicated store into arms, split
         * reads, merged mask) are all banked dead. */
        for (q = 0; (s32)q < 3; q++) {
            c = *((u8 *)p + (s32)q + 0x17);
            D_80106A70[(s32)q] = c;
        }
    }
}
