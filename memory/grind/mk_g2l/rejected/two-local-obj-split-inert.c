/* REJECTED (mk_g2l, session 1) — the two-local obj split is INERT.
 *
 * Measured twice:
 *   - at floor 17 (pre-lever): output BYTE-IDENTICAL to the single-variable
 *     form.  Not merely score-equal — the same instructions.
 *   - at floor 7 (post-lever): score still 7.  It only changes WHICH wrong
 *     register the call-path obj lands in: split -> $a0, single -> $v1.
 *     Target wants $v0, so neither spelling is right.
 *
 * Form: a second local for the field-check region's `*ptr`, on the theory that
 * target's use of $v1 for the field-check obj and $v0 for the four
 * call_with_field4 paths implied two distinct source variables.  It does imply
 * two distinct PSEUDOS in target, but splitting the C variable is not what
 * creates them, and the split does not touch the real blocker (the hard-reg
 * preference merge — evidence.md / hypotheses.md H2).
 *
 * Keep the single `v1_obj` variable: simpler, more natural, same score.
 */
void mk_g2l(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1) {
    s32 v1_obj;      /* the four call_with_field4 paths */
    s32 chk_obj;     /* the field-check region  <-- the inert split */
    u8 *arg_a2 = stack_a2;
    s32 arg_v1 = stack_v1;

    if (a0 == 1) {
        return;
    }

    if (arg_v1 != 0) {
        v1_obj = *ptr;
        cmd = 0x2B;
        goto call_with_field4;
    }

    chk_obj = *ptr;              /* was v1_obj */
    {
        s16 f8E;

        a0 = *(s16 *)(chk_obj + 0x86);
        f8E = *(s16 *)(chk_obj + 0x8E);

        if (a0 == f8E) {
            a0 = *(s16 *)(chk_obj + 0x4);
            cmd = 0x28;
            goto do_call;
        }

        {
            s16 f88 = *(s16 *)(chk_obj + 0x88);
            if (a0 == f88 && a3 != 0) {
                a0 = *(s16 *)(chk_obj + 0x4);
                cmd = 0x27;
                goto do_call;
            }
        }
    }

    /* ... dispatch tail unchanged (all four paths use v1_obj) ... */

call_with_field4:
    a0 = *(s16 *)(v1_obj + 0x4);

do_call:
    func_80032854(a0, cmd, arg_a2, (s16 *)0);
}
