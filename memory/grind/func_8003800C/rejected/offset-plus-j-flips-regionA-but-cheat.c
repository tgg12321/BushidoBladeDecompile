/* REJECTED — offset += j (score 7, from floor 8). NOT match-viable + cheat-by-spelling.
 *
 * WHAT IT DOES (measured s3, 2026-07-22): replacing `offset += 0x24;` with
 * `offset += j;` in the outer-loop tail DROPS the sandbox score 8 -> 7 and
 * FLIPS Region A completely: qty_order among the inner pseudos goes
 *   78 79 77  (bp=v1, j=a0, sum=a1)   -->   78 77 79  (bp=v1, sum=a0, j=a1)
 * i.e. sum=a0 / j=a1 EXACTLY matching target. dispositions: 77 in 4 (a0),
 * 78 in 3 (v1), 79 in 5 (a1). Region A is a pure global-allocno priority/range
 * effect, now CONFIRMED by measurement (was only theoretical in the frontier).
 *
 * MECHANISM: at loop exit j == 0x24, so `offset += j` is value-identical to
 * `offset += 0x24`, but j now lives PAST the inner loop into the tail addu,
 * lengthening j's live_length. global.c priority = numerator/live_length, so
 * j's priority drops below sum's (was j 20000 > sum 13333); sum then allocates
 * first (or ties and wins by pseudo 77<79) and takes a0.
 *
 * WHY REJECTED (two independent reasons):
 *  1) NOT match-viable. `offset += j` emits `addu $a3,$a3,$a1` (register add);
 *     target has `addiu $a3,$a3,0x24` (literal). This instruction can NEVER
 *     byte-match target with this form, so it cannot reach distance 0.
 *  2) Cheat-by-spelling (no-new-park-categories test #4). The offset value is
 *     IDENTICAL to the literal; the ONLY reason to write `+= j` instead of
 *     `+= 0x24` is to extend j's live range to steer register allocation. The
 *     justification references GCC's allocno priority, not program logic.
 *     Target itself uses the literal 0x24 and achieves sum=a0 via a different
 *     (whole-function LUID context) route, so this is not the original source.
 *
 * VALUE: strong EVIDENCE that Region A's sum/j swap is a pure j-vs-sum
 * live-range/priority effect — upgrades the permuter hypothesis (whole-function
 * LUID sweep) from theoretical to mechanism-confirmed. The permuter must find a
 * context that lifts sum's priority WITHOUT an emitted j-consumer (target's
 * sum range == build's sum range, so the discriminator is the conflict graph /
 * absolute LUID numbering, not the isolated inner-loop range).
 *
 * Only the tail line differs from candidate.c:
 *     ...
 *     chkptr++;
 *     i++;
 *     offset += j;        // <-- vs candidate's `offset += 0x24;`
 *     } while (i < 3);
 */
