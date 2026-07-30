/* REJECTED (s2, structural) — giving loop2 its own inner counter is the single
 * worst measured change to this function.
 *
 * Delta (from the s2 candidate base, which already splits loop2's r/w/val):
 *     s32 n;
 *     ...
 *     n = 0x10;
 * inner2:
 *     *p = val;
 *     n--;
 *     p--;
 *     if (n >= 0) goto inner2;
 *
 * Measured: floor 17 -> 40 (build_insns still 170).
 * With loop3 also given its own pointer: 17 -> 36.
 *
 * MECHANISM. `a0` must stay ONE allocno shared by loop1's column counter,
 * loop2's inner counter and loop3's index. That sharing is what gives `a0` a
 * high enough reference count to be allocated early and take $a0(4) -- which is
 * exactly where target has it, in ALL THREE loops. Peeling loop2's counter out
 * creates a fresh tight-range induction pseudo whose priority is high enough to
 * grab $v1(3) before anything else in loop2, displacing the whole loop2 seating
 * (measured: n->$v1(3), a0->$a0 only in loop1/loop3, ptr->$a0(4)).
 *
 * This is the mirror image of the loop2 value variable: `val`/`w`/`r` MUST be
 * split off (that is what frees loop1's `v1`), but `a0` and `ptr` MUST NOT be.
 * The correct partition is asymmetric; do not "finish the job" by splitting the
 * remaining shared locals.
 */
