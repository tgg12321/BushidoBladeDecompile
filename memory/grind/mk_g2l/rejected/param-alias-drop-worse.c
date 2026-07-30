/* REJECTED (mk_g2l, session 1) — dropping the param-alias locals is WORSE.
 *
 * Floor 17 -> 18 (measured, sandbox --disable all).
 *
 * Form: delete `u8 *arg_a2 = stack_a2;` and `s32 arg_v1 = stack_v1;`, test
 * `stack_v1` directly and pass `stack_a2` directly to func_80032854.
 * Rationale tried: [[drop-param-alias-local]] — an alias local can hold a
 * register hostage, so dropping it frees one.  Here it does the opposite.
 * NB both aliases were dropped together; the single-alias variants are still
 * unmeasured (hypotheses.md H3).
 */
void mk_g2l(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1) {
    s32 v1_obj;

    if (a0 == 1) {
        return;
    }

    if (stack_v1 != 0) {          /* was: arg_v1 != 0 */
        v1_obj = *ptr;
        cmd = 0x2B;
        goto call_with_field4;
    }

    /* ... body unchanged ... */

call_with_field4:
    a0 = *(s16 *)(v1_obj + 0x4);

do_call:
    func_80032854(a0, cmd, stack_a2, (s16 *)0);   /* was: arg_a2 */
}
