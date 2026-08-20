/* func_80044098 - s5 MATCH form (F6): sandbox --disable all = 0 at 26/26 insns,
 * zero regfix/asmfix rules, zero register pins, zero cheat-asm.
 *
 * Route: the owner ruling of 2026-08-18 (F6 survey) added the
 * "semantically-null fabricated statement pairs (cancellation-pair /
 * redundant-condition class)" family to the SOTN-accepted list
 * (.claude/rules/no-new-park-categories.md:370-380). That is exactly the
 * reopening path the 2026-07-27 Judge constraint named: the gate-(ii) exhibit
 * it demanded now exists on the frozen list, so the `v1++; v1--;` spelling the
 * s4 permuter found (tmp/grind/func_80044098/s4/perm_base/output-0-2) is
 * sanctioned. The DIFFERENT `v1 += 2; v1 -= 1;` fabricated-decomposition
 * spelling (rejected/judge-fail-0727-1613.c) remains banned by name, both by
 * the F6 text itself and by state.json judge_constraints[0]; it is NOT used
 * here.
 *
 * Mechanism (why the pair is not decoration): the pointer and the counter are
 * near-tied allocnos (s1: counter 14 refs/livelen 16 = pri 26250 -> $v1;
 * pointer 12/17 = 21176 -> $a0) and the whole 13-diff residual of the honest
 * form was that one swap. The cancellation pair adds loop-weighted refs to the
 * pointer pseudo at flow time; combine.c re-merges the +1/-1/+1 chain into the
 * single target `addiu v1,v1,4`, and combine.c:52-57 documents that
 * reg_n_refs is never adjusted afterwards, so global.c allocates with the
 * lifted count and the pointer takes $v1 / the counter takes $a0 as target has
 * them. Byte-neutral: 26 build insns == 26 target insns, score 0.
 *
 * Superseded forms kept for the record: pY (peel + hdr-borrowed constant
 * holder) reached 3 at 26/26 and is proven 3-locked by the s3 theorem; it is
 * archived at rejected/pY-peel-hdr-borrow-floor3.c. */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            do {
                *v1 -= (s32)a6;
                /* FAKE: semantically-null cancellation pair `v1++; v1--;`
                   adjacent to the real `v1++` (owner ruling 2026-08-18, F6
                   survey; .claude/rules/no-new-park-categories.md, SOTN-
                   accepted techniques). what: the pair nets zero and emits no
                   bytes. mechanism: flow.c reg_n_refs counts the extra
                   loop-weighted pointer refs before combine.c re-merges the
                   chain into the single target addiu (combine.c:52-57 -
                   reg_n_refs is never adjusted afterwards), so global.c's
                   allocno priority for the pointer overtakes the counter's
                   and the pointer lands $v1 / the counter $a0 as target has
                   them. lever-exhaustion: memory/grind/func_80044098
                   evidence.md + hypotheses.md s1-s4 (counter-split guard-fold,
                   8/8 same-path decorations cse-folded pre-flow, peel+holder
                   family proven 3-locked from sched.c/flow.c source, ~105k
                   permuter iterations over 4 basins). */
                v1++;
                v1--;
                v1++;
                a4--;
            } while (a4 != -1);
        }
    }
}
