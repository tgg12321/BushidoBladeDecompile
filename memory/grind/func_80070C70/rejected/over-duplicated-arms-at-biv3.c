/* REJECTED - func_80070C70 s4. WHY DEAD: these all reach biv_count 3 and DO kill the giv
 * reduction, but they duplicate MORE than `var_s0 += 1` and pay for it in insns and in a new
 * LICM hoist. Superseded by memory/grind/func_80070C70/candidate.c (61 at 192 insns).
 * Measured on the floor-99 chassis, no FAKE constructs, `sandbox --disable all`, 2026-09-10.
 *   B-dup-tail-3sites     : whole call tail + all three increments in both mode arms -> 88 (200 insns).
 *                           Cost: `prim.code = 1` now appears twice in the loop, so move_movables
 *                           hoists the constant 1 into a SEVENTH callee-saved register (`li s4,1`
 *                           in the preheader) that the target does not spend.
 *   B4-B-ifelse           : the same with the skip arm as an `else`                   -> 88 (200 insns)
 *   B2/B5-inc-in-arms     : all three increments duplicated into the mode arms        -> 93 (195 insns)
 *   B3-tail-dup-2sites    : call tail duplicated, increments once (biv_count 2)       -> 97 (196 insns)
 *   B6-code1-shared       : `prim.code = 1` shared, link+call+increments duplicated   -> 77 (198 insns)
 *   B7/B8-call-dup-only   : only the call duplicated                                  -> 98 (197 insns)
 *   B10-only-s0-dup       : only `var_s0 += 1` duplicated, skip arm still `continue`
 *                           so var_s3/ctx increments are duplicated too               -> 65 (195 insns)
 * The winning discriminator is that the skip arm must be a plain `else` with NO increments of its
 * own, so `var_s3 += 0x16C;` and `ctx_or_var_s2 += 3;` remain a single shared fall-through block -
 * the target's .L80070EC4.
 * Scripts: tmp/grind/func_80070C70/s4/probe2.py, probe3.py, probe4.py
 */
