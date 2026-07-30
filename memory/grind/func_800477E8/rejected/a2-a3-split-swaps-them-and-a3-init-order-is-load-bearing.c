/* REJECTED (s2, structural) — partial splits of loop2's locals, and the
 * `a3 = 0;` statement-position sensitivity that any successor WILL trip over.
 *
 * All measured with sandbox --disable all + the cc1 -da `.greg` dump.
 *
 * (1) split-a2 only  (loop2 gets `w`, a3/v1 stay shared)      : 17 -> 23
 * (2) split-v1 only  (K2's form, re-measured)                 : 17 -> 29
 *     seating: v1->$v1(3) CORRECT, but a2->$a1(5) a3->$a2(6) a1->$a3(7)
 * (3) split-v1+split-a2                                       : 17 -> 25
 *     seating: v1->$v1(3) a1->$a1(5) a0->$a0(4) t0->$t0(8) all CORRECT,
 *              but a2 and a3 swap ($a2<->$a3) and loop2 is wrong
 * (4) split-a2 + declare a1 before a2                         : 17 -> 23
 * (5) split-loop2-all + split-loop3 (a0 split too)            : 17 -> 25..36
 * (6) full loop1 for/for rewrite (nested real for loops)      : creates two
 *     EXTRA induction pseudos (t2 pushed to $t4(12), t1val to $t3(11)); do not
 *     use a for() over the column counter without re-checking the dump.
 *
 * (7) THE STATEMENT-ORDER TRAP. From the accepted s2 candidate base, the three
 *     pre-loop initialisations must be ordered
 *         t2 = 0x2C00;  a3 = 0;  a0 = 0;
 *     Any other order costs points:
 *         a3 = 0; t2 = 0x2C00; a0 = 0;   (HEAD's order) -> a3 and t0 swap, 25
 *         t2 = 0x2C00; a0 = 0; a3 = 0;                  -> 26
 *     MECHANISM: allocno priority is
 *     floor_log2(n_refs)*n_refs*size/live_length; a3 (6 refs) only out-ranks
 *     t0 (5 refs) while a3's live range stays short, and each statement placed
 *     between `a3 = 0;` and the loop head lengthens it. This is ordinary
 *     statement order, not a coercion, but it is arbitrary-looking and a
 *     successor should try to reach the same seating from a NATURAL loop shape
 *     before proposing it to the Judge.
 */
