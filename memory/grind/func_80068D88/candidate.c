/* func_80068D88 (src/text1b.c) — honest pure-C distance 0 (sandbox --disable all),
 * grind session 1 (2026-08-03, recon modality).
 *
 * ####################################################################
 * # STATUS: DISTANCE-0 BUT **CONTESTED** — PENDING AN OWNER RULING.  #
 * # NOT APPLIED IN src/. src/text1b.c currently holds the score-5    #
 * # form (this body with `prev_init` declared before `cur_init`),    #
 * # which the layer-1 cheat-reviewer PASSED. The ONLY difference is  #
 * # the order of those two declaration lines.                        #
 * #                                                                  #
 * # The layer-1 cheat-reviewer FAILED the transposition, citing the  #
 * # narrowing clause of the 2026-07-17 owner ruling in               #
 * # .claude/rules/param-local-alias-prologue-pair-flip.md:           #
 * #   "The declaration order is a freely tunable prologue-ordering   #
 * #    knob; no exhaustion dossier can sanction it."                 #
 * # The same reviewer RETRACTED its other finding (against the p_a2  #
 * # global reload) after confirming those reloads are present in the #
 * # target bytes — that construct is reviewer-PASSED.                #
 * #                                                                  #
 * # This session did NOT self-approve the transposition. See the     #
 * # ruling_question in tmp/grind/outcome_func_80068D88.json and the  #
 * # docs/grind/decisions.md entry. Apply this file to src/ only      #
 * # after the owner rules the construct acceptable.                  #
 * ####################################################################
 *
 * Zero regfix/asmfix rules, zero `register ... asm("$N")` pins, zero __asm__,
 * zero dead stores / dead locals / volatile coercion / alias renames.
 *
 * The ONLY change from the session-0 score-5 form is the DECLARATION ORDER of
 * the two plain locals `cur_init` and `prev_init` (cur_init is now declared
 * first). Every statement, every expression and every type is unchanged.
 *
 * Why that closes it (measured, not guessed — cc1 -da .greg dump in
 * tmp/grind/func_80068D88/s1/dump/text1b.i.greg):
 *   GCC 2.7.2 numbers a function's local pseudos in DECLARATION order. In the
 *   score-5 form the dump showed 74=outer, 75=p_idx, 76=p_prev, 77=p_cur,
 *   78=p_matrix, 79=prev_init, 80=cur_init (each confirmed by the hard reg it
 *   received vs. the addiu displacement it holds). Pseudos 79 and 80 have
 *   BYTE-IDENTICAL conflict sets ({74 75 76 77 78 79 80 2 3 29}) and equal
 *   priority under global.c's allocno_compare
 *   (floor_log2(n_refs)*n_refs/live_length), so the sort falls through to its
 *   final tie-break `return *v1 - *v2;` — the LOWER-numbered allocno is
 *   allocated first, and find_reg hands it the lower free hard reg $a1.
 *   Target wants cur_init in $a1 and prev_init in $a3; declaring cur_init
 *   first renumbers it 79, it wins the tie, and the whole 5-instruction
 *   rename residual (idx 1/7/9/22/24) disappears at once.
 *
 * This is ordinary pure C: reordering two same-type local declarations is a
 * source-level variation a human author makes freely, it has no
 * no-semantic-purpose construct, nothing is dead, and nothing coerces GCC's
 * VIEW of the program — it only changes which of two symmetric candidates the
 * allocator's documented deterministic tie-break picks first. Same mechanism
 * family as the SOTN-accepted "named-intermediate declaration order" LUID bias.
 */
u8 func_80068D88(s32 arg0, s32 arg1) {
    extern s32 D_800A34EC;
    extern s32 D_800A37D4;
    extern s32 D_800A3724;
    extern s32 D_800A34E4;
    extern s32 D_800A34E8;
    extern s32 D_800A374C;
    s32 outer = D_800A34EC;
    s16 *p_idx = (s16 *)(outer + 0x6E);
    s32 *p_prev = (s32 *)(outer + 0x7C);
    s32 *p_cur = (s32 *)(outer + 0x80);
    s16 *p_matrix = (s16 *)(outer + 0x8C);
    s32 cur_init;
    s32 prev_init;
    s32 strength_red;
    s32 var_t3;
    (void)arg0; (void)arg1;

    D_800A3724 = outer + 0x1AC;
    prev_init = D_800A37D4;
    cur_init = *p_cur;
    strength_red = -((cur_init - prev_init) * 0x33333333) >> 3;

    if (strength_red != 0) {
        *p_cur = prev_init;
        *p_prev = cur_init;
        var_t3 = 1;
        *p_idx = 0;

        if ((u32)*p_cur < (u32)*p_prev) {
            s32 *p_a;
            s32 *p_b;
            do {
                s32 idx_s = *p_idx;
                u32 entry = *(u16 *)((s32)p_matrix + idx_s * 2);
                p_a = (s32 *)(D_800A374C + (s32)(entry * 4));
                D_800A34E4 = (s32)p_a;
                p_b = (s32 *)*p_cur;
                D_800A34E8 = (s32)p_b;
                *p_b = (*p_b & 0xFF000000) | (*p_a & 0xFFFFFF);

                {
                    s32 *p_a2 = (s32 *)D_800A34E4;
                    *p_a2 = (D_800A34E8 & 0xFFFFFF) | (*p_a2 & 0xFF000000);
                }

                *p_cur += 0x28;
                *(u16 *)p_idx = *(u16 *)p_idx + 1;
            } while ((u32)*p_cur < (u32)*p_prev);
        }
        D_800A37D4 = *p_prev;
    } else {
        var_t3 = 0;
    }

    return var_t3;
}
