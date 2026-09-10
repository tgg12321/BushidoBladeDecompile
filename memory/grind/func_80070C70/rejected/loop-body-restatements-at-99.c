/* REJECTED (s3, 2026-09-10) - six loop-body / declaration restatements, ALL measured 99 -> 99
 * on the floor-99 chassis (tmp/grind/func_80070C70/s3/probe3.py):
 *   x2-no-t              drop the `s32 t` intermediate, restate both p_static stores from
 *                        prim.p_geom directly                                    -> 99 (195 insns)
 *   x3-incr-order        var_s0 += 1 moved ahead of var_s3 += 0x16C              -> 99
 *   x4-incr-s0-last      var_s0 += 1 moved after ctx_or_var_s2 += 3              -> 99
 *   y1-array-member      record member spelled `s16 v[1]`, read `...v[0]`        -> 99
 *   y2-2d-array          `extern s16 D_800A3590[][1];`, read `[var_s0][0]`       -> 99
 *   y3-shift-in-struct   explicit `(s32)` on the record read before the << 4     -> 99
 * RESULT: y1/y2/y3 confirm the s3 win is owned by the COMPONENT_REF/get_inner_reference OFFSET
 * PATH and not by the particular record spelling - every shape that reaches that path measures the
 * same 99, and the two contrived declarations (a 1-member struct and an [][1] array) are
 * interchangeable. x2/x3/x4 confirm s1's saturation finding still holds at 99: statement-level
 * spelling inside the loop moves the score by exactly 0 while the callee-saved seat rotation and
 * the extra strength-reduced induction register dominate it. Attack the allocation, not the
 * arithmetic.
 */
