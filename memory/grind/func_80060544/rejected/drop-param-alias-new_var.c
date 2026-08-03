/* REJECTED — dropping the `new_var = arg1;` carrier is a REGRESSION, twice measured.
 *
 * The symptom looks exactly like the documented [[drop-param-alias-local]] recipe:
 * our build emits an extra `move s5,s7` after the first loop and then
 * `sll a0,s5,0x2` where target has `sll a0,s7,0x2` — i.e. a redundant copy of the
 * arg1 param into a second callee-save, followed by the rename.  The rule says
 * "drop the alias so the param register frees up for reuse."  It does not apply
 * here: `new_var` is load-bearing.
 *
 * Measurements (engine sandbox func_80060544 --disable all, session s1 2026-08-03):
 *   from the session-start base (floor 18): 18 -> 48, build_insns 134 -> 136.
 *   from the s1 improved base   (floor  4):  4 -> 10, build_insns unchanged 134.
 *
 * Note the second measurement: the insn count did NOT drop, so removing the alias
 * does not even remove the `move` — it just re-shuffles the allocation and breaks
 * the $v0 assignments the geom/stat levers had won.
 *
 * A related variant is also dead: HOISTING the alias's init to the top of the
 * function (`prev = arg0; new_var = arg1; ...`) scored 8 -> 14, and additionally
 * routing the struct field through the carrier (`s.arg2_field = new_var;` with the
 * hoisted init) also scored 14.  See rejected/hoist-new_var-init.c.
 *
 * Do NOT re-propose either form.  The remaining $s5/$s7 pair must be closed by a
 * different mechanism (see hypotheses.md H4).
 */

/* the rejected edit, in full: */
/*   - delete the declaration      `s32 new_var;`                              */
/*   - delete the assignment       `new_var = arg1;`  (between p1 and p0 init) */
/*   - rewrite the ot_Link call:                                               */
        ot_Link(D_800A374C + (arg1 * 4), new_var3);
/*     instead of                                                              */
/*      ot_Link(D_800A374C + (new_var * 4), new_var3);                         */
