/* REJECTED (s4, INVALID - semantics): permuter proposal output-135-1 on the
 * floor-1 chassis hoisted `stptr2 + 6` into a loop-invariant local
 * (`new_var2 = stptr2 + 6;` before loop2, `*((s16 *) new_var2) = 1;` inside).
 * stptr2 advances by 0x68 every iteration, so the hoist writes the SAME
 * address 2 times instead of two distinct ones - a behaviour change, not a
 * spelling. Recorded so no later session re-proposes it from the campaign
 * artifacts. The same output also carried a `(new_var = i) < 0x12` dead
 * store, which is a dead-store-family construct with no measured payoff here.
 */
