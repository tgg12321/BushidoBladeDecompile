/* CANDIDATE - _spu_pitch2note (src/main.c)   MATCH.  sandbox --disable all = 0,
 * 74/74 insns, and `verify-oracle` reports build_matches: true (full-link SHA1
 * == 62efab4f73f992798c43e8c730aa43baa10bb4fa) with this body in src/main.c.
 * s5 (2026-09-08, permuter modality).  Pure C: zero asm, zero pins, zero FAKE
 * constructs, zero volatile, zero dead code.
 *
 * s5 took the floor 19 -> 18 -> 7 -> 6 -> 4 -> 2 -> 0 in six measured steps.
 * The chain, in order:
 *  1. 19 -> 18.  A campaign find on the x7 chassis (tmp/perm_p2n_x7/
 *     output-570-3) named the scan-loop test's shifted value into the (dead,
 *     not yet initialised) outer-loop counter.  Reproduced by hand as k1 on
 *     four chassis (a1/x1/x7/x8): all 18.  A FRESH local for the same
 *     intermediate (k2) or any other dead local (k5..k8) is 19 or worse, so the
 *     point belonged to the outer counter's quantity, not to the split.
 *  2. 18 -> 7.  The pairdiff of the 18 showed the target holding `shift` and
 *     `outer` in the SAME register ($t2: `li t2,15` ... `addiu t2,t2,-1` ...
 *     `move t2,zero`).  Spelling that in C - ONE counter variable driving both
 *     the 16-bit scan loop and the 0x30-iteration outer loop, declared in the
 *     outer counter's slot (m2; the mirror decl slot m1 is 19) - collapsed the
 *     whole 5-seat residual: every register in the function now matches.
 *  3. 7 -> 6.  Pre-loop statement order re-swept on the new shape (q00..q23,
 *     range 6..8): `oct` must come FIRST now (q04/q18/q20/q21 all 6); the
 *     `target; scale; curve; oct;` order that won three sessions running on the
 *     old shape is 7.
 *  4. 6 -> 4.  `target = pitch` moved into the outer loop's for-init
 *     (t1: `for (outer = 0, target = pitch; ...)`), which puts the `andi
 *     a2,a0,0xffff` after `move outer,zero` exactly as the target does.
 *  5. 4 -> 2.  Both inner-loop bounds split into add-then-shift with the LOW
 *     bound's add first (L6: `lo = lower + acc; hi = lower + next; lo >>= 12;
 *     hi >>= 12;`), which fixes the (addu upper / addu lower) inversion that
 *     three sessions had recorded as immune to source order.
 *  6. 2 -> 0.  Naming the inner loop's base index in the for-init (C1:
 *     `base = i * 32`, `result = base + inner`) puts the sll before the
 *     accumulator's `move ...,zero` - the last inversion, the one s4's frontier
 *     had priced as the cheapest remaining residual.
 *
 * Cleanups measured byte-neutral at 0 afterwards, so the SIMPLEST known form is
 * the one that lands (ordinary-c-judge-decidable Ruling 4): the shared counter
 * renamed `i` (E2), `base = i * 32` instead of `i << 5` (E3), and `target`
 * dropped entirely in favour of comparing `pitch` directly (E4, this body).
 * `target = pitch` as a plain pre-loop statement instead is 2 (E1), so the copy
 * had to either sit in the outer for-init or not exist at all.
 *
 * Every construct here has a truthful semantic reading: one counter variable
 * reused by two sequential loops (idiomatic C, and one variable FEWER than the
 * s4 body), a loop-invariant base index computed in the inner loop's init
 * clause next to the other three per-entry seed values, split add/shift
 * statements, and the pre-existing `goto found` mixed exit form.  No FAKE
 * annotation is claimed because no construct is a no-semantic-purpose device.
 *
 * Prototype s32 (u16, u16, u16); no header prototype exists and no in-EXE
 * caller (unchanged from s1). */
s32 _spu_pitch2note(u16 cen_note, u16 cen_fine, u16 pitch) {
    u16 search;
    s32 bit;
    s32 oct;
    s32 scale;
    u32 curve;
    u32 lower;
    u32 upper;
    u32 step;
    u32 acc;
    u32 next;
    u32 lo;
    u32 hi;
    s32 base;
    s32 i;
    s32 inner;
    s32 result;
    s32 quot;
    s32 rem;
    s32 note;
    s32 fine;

    search = ~pitch;
    bit = 0;
    for (i = 15; i >= 0; i--) {
        if (!((search >> i) & 1)) {
            bit = i;
            break;
        }
    }
    oct = bit - 12;
    scale = 1 << bit;
    curve = 0x1000;
    for (i = 0; i < 0x30; i++) {
        lower = scale * curve;
        curve *= 0x103B;
        curve >>= 12;
        upper = scale * curve;
        step = (upper - lower) >> 5;
        for (inner = 0, base = i * 32, acc = 0, next = step; inner < 0x20; inner++) {
            lo = lower + acc;
            hi = lower + next;
            lo >>= 12;
            hi >>= 12;
            if (pitch >= lo && pitch < hi) {
                result = base + inner;
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
