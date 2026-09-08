/* CANDIDATE - _spu_pitch2note (src/main.c)  sandbox --disable all = 20  (s3, 2026-09-08)
 * Pure C, no FAKE constructs, 74/74 insns.  The insn MULTISET matches the target;
 * the whole residual is a register-seat permutation plus two sched1 order
 * inversions (see below).
 *
 * s3 delta over the s2 body (21 -> 20), ONE change:
 *   the upper inner-loop bound is spelled as a SPLIT compound assignment
 *       hi = lower + next;
 *       hi >>= 12;
 *   while the lower bound stays folded (`lo = (lower + acc) >> 12;`).
 *   Splitting BOTH bounds (h4/e2/e3 forms) or only the LOWER one (r2a) measures
 *   21; splitting only the UPPER one is the unique -1.  Same
 *   split-compound-assignment family as the s2 sibling transplant
 *   (`curve *= 0x103B; curve >>= 12;` from the COMPLETED-C sibling _spu_2pitch).
 *
 * The p21 pre-loop statement order (`target; scale; curve; oct;`) was re-swept
 * over all 24 permutations on THIS loop shape (s3 q00..q23, range 20..24) and is
 * still the unique minimum - q00 == p21 == the order below.
 *
 * RESIDUAL (20) = the same 5-seat RA permutation as s2 plus two order inversions:
 *   pitch-copy $a3 vs $a0, shift $a0 vs $t2, lower $t3 vs $t4, acc $t2 vs $t3,
 *   outer $t4 vs $t2; the (andi target / move outer,zero) pair and the
 *   (addu lo / addu hi) pair are emitted in the opposite order.
 *   ra_solver s2 status: forward model EXACT (21/21), inverse.py depth-2
 *   NEGATIVE for the 6-seat goal and for the single flip shift $v1 -> $t2.
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
        acc = 0;
        next = step;
        for (inner = 0; inner < 0x20; inner++) {
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
