/* CANDIDATE - _spu_pitch2note (src/main.c)  sandbox --disable all = 19  (s4, 2026-09-08)
 * Pure C, no FAKE constructs, 74/74 insns.  The insn MULTISET matches the target;
 * the whole residual is a register-seat permutation plus three sched1 order
 * inversions.
 *
 * s4 delta over the s3 body (20 -> 19), ONE change: the inner loop's two
 * accumulator initialisations move into the `for` init clause as a comma
 * expression,
 *     for (inner = 0, acc = 0, next = step; inner < 0x20; inner++)
 * replacing the two separate statements
 *     acc = 0;
 *     next = step;
 *     for (inner = 0; inner < 0x20; inner++)
 * This is ordinary C (a comma expression in a for-init, the standard way to
 * seed several loop-carried values).  The lever is discovered, not invented:
 * the s4 permuter campaign on the s3 chassis produced two finds (output-565-1,
 * output-565-7) that measured 19 by hoisting `acc = 0;` OUT of the outer loop
 * -- a SEMANTIC BREAK, since acc must reset every outer iteration -- which
 * showed that the emission POSITION of `move acc,zero` is worth one point.
 * The hand sweep a1..a9 then found the legal spelling that reproduces the same
 * position: `acc = 0` must sit inside the for-init AND after `inner = 0`.
 *   19: a1 (inner, acc, next), a3 (inner, next, acc), a4 (next=step outside;
 *       for (inner = 0, acc = 0)), a8 (next, inner, acc)
 *   20: a2/a6 (acc first), a5 (acc left outside the for-init),
 *       a7/a9 (acc before inner)
 *
 * The 24-permutation pre-loop statement-order sweep was re-run on THIS loop
 * shape (s4 p00..p23, range 19..23): `target; scale; curve; oct;` is still the
 * unique minimum, so the order below is unchanged from s2/s3.
 *
 * Naming `base = outer << 5` and putting it in the for-init (c1/c2/c4) is
 * byte-neutral at 19; putting it outside the for-init (c3/c5) costs a point.
 *
 * RESIDUAL (19; pairdiff tmp/grind/_spu_pitch2note/s4/a1_pairdiff.txt):
 *   seats  pitch-copy $a3 vs $a0 | shift $a0 vs $t2 | lower $t3 vs $t4 |
 *          acc $t2 vs $t3 | outer $t4 vs $t2
 *   order  ours 'andi a2,a3,0xffff' at 16 and 'oct' at 20; target 'oct' at 16
 *          and the andi at 21, after 'move outer,zero'
 *   order  ours emits 'move acc,zero' BEFORE the LICM-hoisted 'sll base,outer,5';
 *          the target emits the sll first (still one apart, insns 33/34)
 *   order  ours emits the UPPER bound's addu before the LOWER bound's; the
 *          target emits the lower first.
 *
 * Prototype s32 (u16, u16, u16); no header prototype exists and no in-EXE caller. */
s32 _spu_pitch2note(u16 cen_note, u16 cen_fine, u16 pitch) {
    u16 search;
    s32 bit;
    s32 shift;
    s32 oct;
    s32 scale;
    u32 curve;
    u32 target;
    u32 lower;
    u32 upper;
    u32 step;
    u32 acc;
    u32 next;
    u32 lo;
    u32 hi;
    s32 outer;
    s32 inner;
    s32 result;
    s32 quot;
    s32 rem;
    s32 note;
    s32 fine;

    search = ~pitch;
    bit = 0;
    for (shift = 15; shift >= 0; shift--) {
        if (!((search >> shift) & 1)) {
            bit = shift;
            break;
        }
    }
    target = pitch;
    scale = 1 << bit;
    curve = 0x1000;
    oct = bit - 12;
    for (outer = 0; outer < 0x30; outer++) {
        lower = scale * curve;
        curve *= 0x103B;
        curve >>= 12;
        upper = scale * curve;
        step = (upper - lower) >> 5;
        for (inner = 0, acc = 0, next = step; inner < 0x20; inner++) {
            lo = (lower + acc) >> 12;
            hi = lower + next;
            hi >>= 12;
            if (target >= lo && target < hi) {
                result = (outer << 5) + inner;
                goto found;
            }
            next += step;
            acc += step;
        }
    }
    result = 0x600;
found:
    quot = result / 128;
    rem = result % 128;
    note = cen_note + quot + oct * 12;
    fine = cen_fine + rem;
    return (note << 8) | fine;
}
