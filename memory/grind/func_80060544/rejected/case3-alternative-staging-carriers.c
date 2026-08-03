/* func_80060544 — REJECTED: every alternative carrier for the Case3 staged
 * address except `end_off`.  (grind session s4, 2026-08-03, modality permuter)
 *
 * Context: the closing lever (see candidate.c) stages the Case3 static-table
 * address through the finished `end_off` local:
 *
 *     Case3:
 *         stat = (s32)(&D_8009B7D0);
 *         end_off = stat;                 <== the lever
 *         s.p_static = (s32 *)end_off;
 *
 * These are the carriers that DO NOT work.  Measured in the reduced-TU
 * workspace tmp/perm_60544 (validated instruction-identical to the full-file
 * cheat-free sandbox build: 133 insns and the same single `move a1,zero`
 * displacement), reported as raw differing instructions vs target:
 *
 *   base — no staging at all ........................ 3   (133 insns)
 *   a FRESH local `s32 stat3;`  ..................... 3   (133)
 *   `geom`   (dead here; the address carrier) ....... 3   (133)
 *   `idx`    (dead here? no — live across the loop) . 4   (133)
 *   `mid_off` (LIVE — semantics change) ............ 123  (129)
 *   `j`      (loop-2 counter, dead here) ........... 136  (136)
 *   `p0`     (loop-2 walking pointer, dead here) ... 136  (136)
 *   `p1`     (loop-2 walking pointer, dead here) ... 136  (136)
 *   `end_off` ....................................... 0   (133)  <== the match
 *
 * What this kills:
 *   - "the lever is just an extra pseudo hop" — a fresh single-set local is
 *     exactly inert (3), i.e. the hop by itself does nothing.
 *   - "any dead local will do" — geom, j, p0 and p1 are all dead at that
 *     point and all fail (j/p0/p1 catastrophically: borrowing a loop-2
 *     variable extends its live range across loop 1 and grows the frame).
 *   - "it is a multi-set / reg_n_sets effect" in the generic sense — `geom`
 *     and `idx` are already multi-set and both fail.
 *
 * What survives: the operative property is WHICH allocno carries the staged
 * value.  `end_off` is the end-offset carrier that s2 identified as one of
 * the two pseudos in the $s7/$s8 callee-save race, and only extending THAT
 * one into the Case3 block reproduces target's instruction order.
 *
 * Do not re-propose any of the above carriers.  The measured forms are the
 * bodies below (only the two staging lines differ between them).
 *
 * CORROBORATION IN THE REAL TREE (s4, second half of the session): the same
 * sweep re-run through `sandbox func_80060544 --disable all` on src/text1b.c,
 * ten existing locals, one sandbox run each (scores are the engine's honest
 * cheat-free distance, so they are NOT comparable to the reduced-TU raw
 * instruction counts above — only the ordering is):
 *
 *   end_off ..... 0  (insns 133)  <== the match, reproduced full-file
 *   geom ........ 2  (133)   — i.e. exactly the unstaged base, inert
 *   idx ......... 4  (133)
 *   prev ........ 4  (133)
 *   i ........... 4  (133)
 *   new_var3 ... 18  (133)
 *   last ....... 22  (133)
 *   mid_off .... 22  (129)   — mid_off is LIVE; this form is also incorrect C
 *   end_ptr .... 23  (133)   — staging through the *other* end-offset pseudo
 *   j .......... 56  (136)
 *
 * The `end_ptr` result (23) is the sharpest of these: the two ends of the
 * end-offset carrier chain are NOT interchangeable, so the lever is not "touch
 * the end-offset value again", it is specifically the first pseudo of that
 * chain — the one whose live range s2 measured as the loser of the $s7/$s8
 * callee-save race.
 */

/* --- fresh local (inert) ------------------------------------------------ */
/*  s32 stat3;                          (extra declaration)
    Case3:
        stat = (s32)(&D_8009B7D0);
        stat3 = stat;
        s.p_static = (s32 *)stat3;
*/

/* --- geom (inert) ------------------------------------------------------- */
/*  Case3:
        stat = (s32)(&D_8009B7D0);
        geom = stat;
        s.p_static = (s32 *)geom;
*/

/* --- idx (worse) -------------------------------------------------------- */
/*  Case3:
        stat = (s32)(&D_8009B7D0);
        idx = stat;
        s.p_static = (s32 *)idx;
*/

/* --- mid_off (WRONG — mid_off is live, this changes behaviour) ---------- */
/*  Case3:
        stat = (s32)(&D_8009B7D0);
        mid_off = stat;
        s.p_static = (s32 *)mid_off;
*/

/* --- j / p0 / p1 (catastrophic: +3 insns) ------------------------------- */
/*  Case3:
        stat = (s32)(&D_8009B7D0);
        p0 = (s32 *)stat;
        s.p_static = p0;
*/
