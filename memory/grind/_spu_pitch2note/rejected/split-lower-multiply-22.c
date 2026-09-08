/* CANDIDATE - _spu_pitch2note (src/main.c)  sandbox --disable all = 21  (s2, 2026-09-08)
 * Pure C, no FAKE constructs, 74/74 insns - the insn MULTISET now matches the
 * target exactly (inverse_compose classify: FIRST DIVERGENCE = RA).
 *
 * Three s2 deltas over the s1 body (36):
 *  1. SIBLING TRANSPLANT (-12): `curve *= 0x103B; curve >>= 12;` instead of
 *     `curve = (curve * 0x103B) >> 12;` - the split compound-assignment spelling
 *     the COMPLETED-C sibling `_spu_2pitch` (src/main.c, `ratio *= 0x103B;
 *     ratio >>= 12;`) uses for the same 0x103B curve step.  36 -> 24.
 *  2. HIT ARM FIRST (-1, and it is what removes the last STRUCTURAL residual):
 *     `if (target >= lo && target < hi) { result = ...; goto found; }` followed
 *     by the two increments, instead of s1's `if (miss) {incs} else {hit}`.
 *     The label now sits BEFORE the increments, so nothing separates
 *     `acc += step` from `inner++` and reorg.c:2969 fill_simple_delay_slots
 *     takes `acc += step` into the back-edge slot exactly as the target does
 *     (s1's F1 frontier item - closed).  24 -> 23.
 *  3. PRE-LOOP STATEMENT ORDER (-2): `target; scale; curve; oct;` (permutation
 *     p21 of the four pre-loop assignments) instead of `oct; scale; curve;
 *     target;`.  All 24 permutations were measured (21..25); p21 is the unique
 *     minimum and it seats `bit` in $v1 as the target does.  23 -> 21.
 *
 * RESIDUAL (21) = a 5-seat RA permutation + 2 sched1 order inversions:
 *   shift $a0 vs $t2, pitch-copy $a3 vs $a0, lower $t3 vs $t4, acc $t2 vs $t3,
 *   outer $t4 vs $t2; and the pairs (andi target / move outer=0) and
 *   (srl v0 / addu v1) emitted in the opposite order.
 *   ra_solver: forward model EXACT (21/21 dispositions); inverse.py depth-2
 *   NEGATIVE for the 6-seat goal AND for the single flip shift $v1->$t2.
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
        lower = curve;
        lower *= scale;
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
