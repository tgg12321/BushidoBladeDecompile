/* s64 (synthesis, 2026-09-05) -- FLOOR 9 -> 2.  49/49 instructions; the ENTIRE
 * residual is TWO instructions inside the copy loop, and it is one register
 * field in each of them:
 *
 *      ours                     target
 *      lbu  $a0, 0x17($v0)      lbu  $v0, 0x17($v0)
 *      sb   $a0, %lo(D_..)($at) sb   $v0, %lo(D_..)($at)
 *
 * EVERY other instruction in the function -- including all three la pairs, the
 * block-0 reload in the load-delay slot of the flag lw, and all seven register
 * seats that 63 sessions could not reach ($v1 for block 0's address, $a0 for
 * block 0's value, $a1 for p) -- is byte-identical to the target.
 *
 * INTEGRATION HANDOFF (unchanged from s62/s63): this body needs the split
 * declaration in include/code6cac.h -- `extern u8 D_80106A70[3];` absorbing
 * D_80106A71/D_80106A72 (their two consumers in src/code6cac.c converted to
 * element form) with `extern u8 D_80106A73;` left as its own scalar.  Measured
 * byte-neutral project-wide (s62, s63).  One-command installer:
 * `python3 tmp/grind/func_80034F88/s63/apply.py <body.c>`.
 *
 * HOW THE SEAT WAS FINALLY REACHED (the s64 result).  tools/ra_solver/
 * inverse.py run on the chassis-(a) model (mask+reload in ONE local) with the
 * goal {block-0 address: $v1, block-0 value: $a0, p: $a1} returns
 * "minimal solution size: 1 atom" and its cheapest vector is
 *   [conflict_add] pseudo 73 (the loop index): conflict +77 (block 0's value)
 * with the named C lever "(variable identity) reuse one variable across both
 * regions".  Spelled: the block-0 value local `u` is hoisted to function scope
 * and REUSED as the copy loop's byte temp.  Measured allocation afterwards
 * (tmp/grind/func_80034F88/s64/a1.model.json):
 *   ord0 p73 index   pri 47142 -> $v1
 *   ord1 p76 value+loop temp (11 refs / livelen 10) pri 33000 -> $a0   (TARGET)
 *   ord2 p75 c       pri 23684 -> $v0
 *   ord3 p74 v       pri 12000 -> $v1   (TARGET)
 *   ord4 p81 r       pri  6315 -> $a0   (TARGET)
 *   ord5 p77 q       pri  3571 -> $v1   (TARGET -- block 0's address, the seat
 *                                        the whole grind has been chasing)
 *   ord6 p72 p       pri  3529 -> $a1   (TARGET)
 *
 * WHY THIS SPELLING IS CAPPED AT 2, AND WHAT THE NEXT SESSION MUST CHANGE.
 * The conflict is bought by making block 0's value live across the loop, and
 * the only byte-free way to be live there is to BE the loop's byte temp.  But
 * GCC 2.7.2 gives one hard register per allocno (global.c:1275) and the target
 * needs that value in $a0 in block 0 and in $v0 in the loop.  So the
 * loop-index conflict route cannot go below 2 by any spelling.  The next
 * session needs a DIFFERENT source for the same conflict: an allocno seated at
 * $v1 that block 0's value can overlap without borrowing its register.  The
 * only other $v1 allocno in the function is p74 (blocks 1/2's value); s64
 * measured that reusing `u` in block 2 (rejected/s64-u-reused-block2-score10.c)
 * does NOT create that conflict, because `u` is dead throughout block 1.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 v;
    s32 c;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block-0's own address object, mechanism: global.c:1275 assigns
         * exactly one hard register per allocno and GCC 2.7.2 does no
         * live-range splitting. lever-exhaustion: hypotheses.md s53-s60. */
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
    }
    {
        /* FAKE: the address object for flag blocks 1 and 2, mechanism: as
         * above. lever-exhaustion: as above. */
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

    for (i = 0; i < 3; i++) {
        u = *((u8 *)p + i + 0x17);
        c = u;
        D_80106A70[i] = c;
    }
}
