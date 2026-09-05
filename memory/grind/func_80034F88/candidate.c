/* s66 (solver, 2026-09-05) -- FLOOR 2 -> 0.  49/49 instructions, byte-identical
 * to asm/funcs/func_80034F88.s.
 *
 * WHAT CLOSED IT.  s65 fitted GCC 2.7.2's global.c allocation priority exactly
 * (pri = floor_log2(nrefs) * nrefs * 10000 / live_length) and reduced the whole
 * residual to two arithmetic branches.  Branch (A) said: block 0's address
 * object reaches the target seating iff it carries at least SIXTEEN references,
 * because it must be allocated before block 0's value (pri 17500) and its
 * five references at live length 28 only price at 3571.  s65 measured the
 * obvious byte-neutral reference lifts (duplicated store into arms, split
 * reads) dead.  This session found the one that is free: the address object
 * and the copy loop's counter are ONE variable, so the loop's eleven counter
 * references land on the address allocno.  Measured model
 * (tmp/grind/func_80034F88/s66/z2.model.json):
 *
 *   ord0 p74 c (flag/result)      19 refs / len 21 / pri 36190 -> $v0  TARGET
 *   ord1 p76 q (address + index)  16 refs / len 21 / pri 30476 -> $v1  TARGET
 *   ord2 p75 u (block-0 value)     7 refs / len  8 / pri 17500 -> $a0  TARGET
 *   ord3 p73 v (blocks-1/2 value)  6 refs / len 10 / pri 12000 -> $v1  TARGET
 *   ord4 p80 r (blocks-1/2 addr)   6 refs / len 19 / pri  6315 -> $a0  TARGET
 *   ord5 p72 p                     6 refs / len 34 / pri  3529 -> $a1  TARGET
 *
 * The block-0 value no longer has to be blocked out of $v1 by a conflict (the
 * s64 loop-index-conflict route, which capped at score 2 because one allocno
 * gets one hard register): $v1 is simply already taken by the higher-priority
 * address/counter allocno when it is allocated, so it scans on to $a0 -- which
 * is what the target does -- and the copy loop's byte temp is a plain
 * block-local that local-alloc seats at $v0, also as the target has it.
 * Sixteen references is exactly the threshold s65 computed; the merge supplies
 * 5 + 11 = 16 with no extra instruction anywhere in the function.
 *
 * INTEGRATION HANDOFF (unchanged from s62-s65): this body needs the split
 * declaration in include/code6cac.h -- `extern u8 D_80106A70[3];` absorbing
 * D_80106A71/D_80106A72 (their two consumers in src/code6cac.c converted to
 * element form) with `extern u8 D_80106A73;` left as its own scalar in
 * src/code6cac_b.c.  Measured byte-neutral project-wide (s62, s63, and the
 * full-build oracle check this session).  One-command installer:
 * `python3 tmp/grind/func_80034F88/s63/apply.py <body.c>`.
 */
void func_80034F88(void) {
    s32 *p;
    s32 v;
    s32 c;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block-0's own address object, also carrying the copy loop's
         * counter below, mechanism: global.c allocation priority
         * floor_log2(nrefs)*nrefs*10000/live_length -- the loop's eleven
         * counter references lift this allocno from 5 refs / pri 3571 to
         * 16 refs / pri 30476, so it is seated in $v1 before block 0's value
         * allocno (pri 17500) is considered and that value scans on to $a0.
         * lever-exhaustion: hypotheses.md s53-s65 (the reference-lift branch
         * is s65's branch (A); its other spellings are banked dead). */
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

        for (q = 0; (s32)q < 3; q++) {
            c = *((u8 *)p + (s32)q + 0x17);
            D_80106A70[(s32)q] = c;
        }
    }
}
