/* REJECTED — session 1 (2026-08-13). Slug: while-loop-triggers-loop-rotation.
 *
 * HYPOTHESIS (killed): the function's trailing loop is written in the source
 * as a `goto check; loop: ...; check: if (cond) goto loop;` chain, which emits
 * NO NOTE_INSN_LOOP_BEG/END. flow.c weights reg_n_refs by loop_depth
 * (flow.c:2081, 2329, 2515, 2725), so a goto-loop leaves every in-loop
 * reference counted at weight 1. Rewriting the loop as a real `while` would
 * restore the loop notes, lift the loop pointer's reg_n_refs, and (the hope)
 * flip the $s0/$s1 allocation inversion that is the whole remaining residual.
 *
 * MEASURED RESULT: floor went 23 -> 38 (WORSE), and the instruction count went
 * 116 -> 121 (5 EXTRA instructions vs target). The `while` form makes loop.c
 * ROTATE the loop: it duplicates the exit test to the top
 *     lw v1,4(sN); li v0,-1; beq v1,v0,<exit>
 * and deletes the target's `j .L800787F8` entry jump. It also forces a FOURTH
 * callee-save ($s3) live and grows the frame 88 -> 96 bytes. The $s0/$s1
 * inversion was NOT fixed: the loop pointer still landed in $s1.
 *
 * CONCLUSION: the target's entry-jump-to-bottom-test shape (`j .L800787F8`
 * in the delay-slot pair at target insn 72-73) is the UNROTATED form, which
 * this GCC only produces from the goto spelling. Do not re-propose `while`,
 * `for`, or `do{}while` spellings of this loop; and note that the loop-depth
 * ref-weighting route to the $s0/$s1 flip does not work arithmetically either
 * (see hypotheses.md H3 — arg0 stays ~1.6-2x ahead at every weight).
 */

    s.cd_flag = 0;
    while (var_s0[1] != -1) {
        s.a = var_s0[0];
        s.b = s.a + 0xC;
        s.h = -D_800A3608;
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
        var_s0++;
        arg0[5] = arg0[5] + 0xC;
    }
}
