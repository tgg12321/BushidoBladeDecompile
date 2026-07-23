/* REJECTED (s2 structural) — recompute bitpos = i<<4 at loop top (drop the
 * carried `bitpos += 0x10`). score 14.
 *
 *     do { s32 idx4;
 *          bitpos = i << 4;          // fresh each iter, no back-edge carry
 *          ... } while (i < 2);
 *   (bitpos=0 init and bitpos+=0x10 tail both removed)
 *
 * INTENT: shorten bitpos's live_length (no longer live across the back-edge)
 * -> raise its allocno pri -> bitpos wins $t1. RESULT: backfired. Deriving
 * bitpos from i adds a THIRD i reference (i now feeds `i*4`, `i<<4`, i++, i<2),
 * so i's n_refs jump and i takes the top pri slot -> i=$t1, bitpos=$t2, sum=$t3.
 * Also replaces the target's `addiu $t1,$t1,0x10` with `sll $t2,$t1,4` (loop-
 * body opcode diff). KILLED — recomputing a carried induction var inflates the
 * source var's refs and steals the register.
 */
