/* REJECTED — operand-axis reassociation for codegen control.
 * Violated rule: sibling family of [[or-tree-shape-shift]] (FORBIDDEN)
 * + [[no-new-park-categories]] §"cheats by any spelling".
 *
 * Score: 2 (vs candidate's 6) — flips RA so sum->$v1, i->$a3 to match
 * target. But the parenthesisation `(sum + arg2) - arg1` reorders the
 * epilogue to `addu+subu` instead of target's `subu+addu`; the 2 diffs
 * are the epilogue insns.
 *
 * Why it's a cheat: a programmer writing this function would naturally
 * write `sum + (arg2 - arg1)` (sum + offset) — the reassociation only
 * exists to shorten sum's live range and bump its GCC allocno priority
 * above i's. Justification refers to GCC's allocator → cheat by intent
 * (no-new-park-categories §"You can describe what it does without
 * referencing GCC's allocator…").
 *
 * Preserved so future agents don't re-derive this.
 */
s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        sum += arg0;
        arg0--;
    }
    return (sum + arg2) - arg1;
}
