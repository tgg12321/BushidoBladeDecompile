/* func_80069AE4 (text1b.c) — the qbase do-while loop region (HEAD form).
 * Matches ONLY with the 3 regfix scheduling-paperwork rules (reorder 94,92,93
 * + insert_label .L_LOOP_TOP @93 + subst .L# @104) that hoist `addiu a0,sp,24`
 * (= &s.sp18, the call arg) to the LOOP TOP as the branch target.
 * No pure-C form found that makes GCC's list scheduler place the loop-invariant
 * address computation at the loop head. See notes.md. */
    q = qbase;
    i = 0;
    do {
        s32 v = *q;
        q++;
        s.sp18 = v;
        s.sp1C = v + 0xC;
        s.sp20 = arg0[5];
        arg0[5] = func_8007352C((s32)&s.sp18);
        i++;
    } while (i < 3);
