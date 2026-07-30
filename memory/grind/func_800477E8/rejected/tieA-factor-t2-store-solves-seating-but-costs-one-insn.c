/* REJECTED-AS-WRITTEN (s3, structural) — but this file is the PROOF of tie A's
 * mechanism, so read it before attacking tie A again.
 *
 * WHAT IT PROVES.  Lifting the shared `*s0 = t2;` store out of loop1's two arms
 * gives t2 ONE in-loop reference instead of one per arm, and that alone flips tie
 * A to target: the .greg dispositions become t1val->$t1(9) and t2->$t2(10) with
 * EVERY other disposition still equal to target.  So tie A is a reference-count
 * problem exactly as s2's F5 predicted, and it is reachable — s2's K8 ("no
 * structural lever moves tie A") is overturned on mechanism.
 *
 * WHY IT IS STILL REJECTED.  Factoring needs a second test of `a3 >= 5`, and cc1
 * emits one instruction more than target: build_insns 171 vs 170, score 10-13
 * (worse than the accepted form's 5 despite the better allocation).  Caching the
 * condition in a local first (`v0 = (a3 >= 5); if (v0) ... if (v0) ...`) does not
 * help — same 171.
 *
 * THE ARITHMETIC, so a successor does not have to re-derive it.  Priority is
 * floor_log2(n_refs)*n_refs/live_length; references are weighted by loop_depth.
 *   as written in the candidate: t1val 3/76 = 0.0395   t2 5/150 = 0.0667  -> t2
 *   with t2 factored          : t1val 3/76 = 0.0395    t2 3/150 = 0.020   -> t1val
 * t2's live_length is double t1val's because t2 is used in BOTH arms and so stays
 * live on both paths, while t1val is dead along the `a3 >= 5` path.  Equalising
 * from the other side needs a FOURTH weighted reference to t1val (2*4/76 = 0.105
 * would beat 0.0667); no honest one exists in this body, and an extra reference
 * placed after the loop lengthens t1val to ~150 and loses again (0.053).
 *
 * SO THE OPEN QUESTION IS NARROW: a spelling of "t2 is read once per iteration"
 * that keeps 170 instructions.  Measured and dead so far:
 *   - factored with two ifs                      : 171, score 10
 *   - factored with the condition cached in v0    : 171, score 13
 *   - literal 0x2C00 in BOTH arms (no t2 local)   : 170, score 6 — CSE rebuilds a
 *     single pseudo with the same 2-use profile, seating unchanged
 *   - literal in the a3>=5 arm, t2 in the else arm: 171, score 3 (best score seen
 *     anywhere, but it materialises a second `li` — the +1 insn is real)
 *   - both arms storing a v0 copy of t2           : 171, score 39
 *   - `s0[1] = t2;` hoisted ahead of the branch with `s0 += 2` in the arms:
 *     167 insns, score 10
 */

        v0 = (a3 >= 5);
        if (v0) {
            *s0 = s3val;
        } else {
            *s0 = s2val;
        }
        s0 += 1;
        *s0 = t2;          /* <- the single in-loop reference that flips tie A */
        s0 += 1;
        if (v0) {
            *s0 = s1val;
            /* ... the rest of the a3 >= 5 arm, unchanged ... */
        } else {
            *s0 = t1val;
            /* ... the rest of the else arm, unchanged ... */
        }
