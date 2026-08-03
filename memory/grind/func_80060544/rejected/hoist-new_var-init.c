/* REJECTED — hoisting the `new_var = arg1;` carrier's init to the top of the
 * function does NOT coalesce the carrier with the arg1 param register.
 *
 * Hypothesis under test: our build's extra `move s5,s7` exists because the
 * carrier pseudo's live range starts mid-function (after the first loop) and so
 * never gets coalesced with the incoming-arg pseudo that owns $s7.  Starting the
 * carrier's live range in the prologue, alongside `prev = arg0;`, should let the
 * two coalesce and let the late `sll` read $s7 directly, as target does.
 *
 * Measurements (engine sandbox func_80060544 --disable all, session s1 2026-08-03),
 * both from the s1 improved base of floor 8 (geom + stat levers in, `last` not yet):
 *
 *   variant A — hoist the init only:                    8 -> 14
 *       prev = arg0;
 *       new_var = arg1;          <-- moved up from just before `p0 = &D_8009B3B0;`
 *       mid_off = arg0 + 0x4EC;
 *
 *   variant B — hoist the init AND route the struct field through the carrier,
 *   so a single C name is the only reader of arg1 in the whole body:  8 -> 14
 *       s.arg2_field = new_var;  <-- instead of `s.arg2_field = arg1;`
 *
 * Both regress by the same 6.  The carrier's position in source order is
 * therefore load-bearing in the OPPOSITE direction from the hypothesis: it must
 * stay late (between the `p1` and `p0` initialisations) for the $v0/$v1
 * assignments won by the geom/stat levers to survive.
 *
 * Do NOT re-propose.  See also rejected/drop-param-alias-new_var.c.
 */
