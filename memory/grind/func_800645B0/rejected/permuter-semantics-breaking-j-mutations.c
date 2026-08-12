/* REJECTED — permuter finds, grind session 5 (permuter modality), 2026-08-12.
 *
 * Campaign `ia-maintained-index-chassis` (tmp/grind/func_800645B0/s4/ws2,
 * 52,757 iterations) produced 10 novel outputs.  Every one that scored below
 * the 200 base but above the 10 floor defeats the loop-entry constant fold
 * (`j == 0`, which makes GCC emit `move s0,s3` instead of the target's
 * `addu s0,s3,a0`) by DESTROYING the loop semantics.  The three shapes:
 *
 *   output-160-1 (score 160): `j = 0;` moved out of the outer loop body and
 *     into the found-slot arm, so `j` is read uninitialised on entry and is
 *     never reset for a group with no free slot.
 *   output-166-1 (score 166): `idx2 = 0;` hoisted above the outer loop and the
 *     reset written `j = idx2;` — same effect, one indirection further out.
 *   output-135-1 (score 135): `j = mask;` inside an `if (1) { }` wrapper, i.e.
 *     the loop counter overwritten with the occupancy bit, with the loop test
 *     rewritten `(j + 1) < (4 + 1)`.
 *
 * These are not candidate C.  Their value is as a measurement: they are the
 * permuter's independent confirmation of session 3's H15/LA result — the ONLY
 * mutations that defeat the entry-copy fold are ones that make `j` genuinely
 * non-constant there, and every legal way to do that costs a real instruction.
 */
