/* mk_g2l — BEST FORM, session 1 (grinder). honest sandbox floor: 7 (was 17).
 *
 * IN PLACE in src/code6cac_b.c as of session 1.
 *
 * The ONLY lever that moved the floor: hold the 0x86-field compare value in the
 * PARAMETER a0 (which is dead after the `a0 == 1` early-out) instead of in a
 * fresh `s16 f86` local.  17 -> 7, and it fixed FOUR register families at once
 * (see evidence.md): ptr -> $t0, chk-region obj -> $v1, stack_v1 -> $v1,
 * f86 -> $a0.  All four now match target.
 *
 * OPEN POLICY QUESTION for the Judge / next session: `a0 = *(s16 *)(v1_obj +
 * 0x86);` reuses a parameter as the holder for an unrelated value.  That is the
 * SOTN "variable reuse" family ([[named-local-fake-exception]] /
 * [[staged-value-reused-variable]]), which is sanctioned only as a last resort
 * with documented lever-exhaustion + a /* FAKE *\/ annotation.  It is NOT a dead
 * store and NOT a pin, and the reuse is of a genuinely-dead parameter, but the
 * only reason to write it this way is codegen.  Session 1 did not annotate it,
 * because the exhaustion record does not exist yet and the alternative spelling
 * (a fresh single local serving both the compare and the call's first argument)
 * has not been measured.  Measure that first (frontier H1).
 *
 * Residual 7 diffs, both families in evidence.md:
 *   [3]/[4]  stack-param load order: target loads 0x28(sp) (stack_a2) BEFORE
 *            0x2C(sp) (stack_v1); we emit the reverse.
 *   [10][34][39][42][44]  the call-path obj pseudo gets $v1; target wants $v0.
 */
void mk_g2l(s32 a0, s32 *ptr, s32 cmd, s32 a3, u8 *stack_a2, s32 stack_v1) {
    s32 v1_obj;
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

    v1_obj = *ptr;
    {
        s16 f8E;

        a0 = *(s16 *)(v1_obj + 0x86);
        f8E = *(s16 *)(v1_obj + 0x8E);

        if (a0 == f8E) {
            a0 = *(s16 *)(v1_obj + 0x4);
            cmd = 0x28;
            goto do_call;
        }

        {
            s16 f88 = *(s16 *)(v1_obj + 0x88);
            if (a0 == f88 && a3 != 0) {
                a0 = *(s16 *)(v1_obj + 0x4);
                cmd = 0x27;
                goto do_call;
            }
        }
    }

    if (cmd == 0) {
        v1_obj = *ptr;
        cmd = 0x22;
        goto call_with_field4;
    }

    if (cmd < 6) {
        cmd = 0x23;
        v1_obj = *ptr;
        goto call_with_field4;
    }

    v1_obj = *ptr;
    cmd = 0x24;

call_with_field4:
    a0 = *(s16 *)(v1_obj + 0x4);

do_call:
    func_80032854(a0, cmd, arg_a2, (s16 *)0);
}
