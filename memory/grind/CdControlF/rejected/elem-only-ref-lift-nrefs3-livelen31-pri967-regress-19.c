/* REJECTED (session 3). Session 2's F0 frontier as literally specified: give
 * `elem` a byte-neutral THIRD reference. It does not work, and the reason kills
 * the whole "lift one value" class.
 *
 * Measured (tmp/grind/CdControlF/s3/variants_a.json, S1_dw0_deref_only):
 *   elem nrefs 2 -> 3, livelen UNCHANGED at 31, pri 645 -> 967.
 * 967 is ABOVE idx's 882, so elem is allocated third and takes s2, not s5.
 * Score 9 -> 19.
 *
 * The general lesson: a loop-note region multiplies reg_n_refs but leaves
 * reg_live_length alone, so ref-lifting can never place a value INTO a narrow
 * priority window. s2's "elem needs livelen ~38" is unreachable by ref-lifting,
 * and reaching it any other way needs an `elem` reference in the post-loop tail
 * — which asm/funcs/CdControlF.s shows the ORIGINAL does not have either (the
 * target tail never touches s5).
 *
 * Sibling kills from the same batch, same class:
 *   S2_dw0_a1_block  -> a1 7 refs, pri 3783 > count 2285, a1 takes s0. Score 28.
 *   S7_dw0_idx_guard -> idx 4 refs, pri 2352 > count 2285, idx takes s0. Score 22.
 *
 * The closing form instead sizes ONE region to the whole loop body so that
 * every row moves together; see memory/grind/CdControlF/candidate.c.
 */
s32 CdControlF(u8 a0, s32 a1) {
    s32 result;
    s32 idx;
    s32 saved;
    s32 count;
    s32 *base;
    s32 *elem;
    s32 nparam;

    result = 0;
    idx = a0;
    saved = g_cd_callback_a;
    count = 3;
    base = g_cd_sector_buf;
    elem = base + idx;

loop:
    g_cd_callback_a = 0;

    if (idx != 1) {
        if (g_cd_mode & 0x10) {
            CD_cw(1, 0, 0, 0);
        }
    }
    if (a1 != 0) {
        do { nparam = *elem; } while (0);
        if (nparam != 0) {
            if (CD_cw(2, a1, 0, 0) != 0) {
                goto next;
            }
        }
    }
    g_cd_callback_a = saved;
    if (CD_cw(a0, a1, 0, 1) == 0) {
        goto done;
    }
next:
    count--;

    if (count != (-1)) {
        goto loop;
    }
    g_cd_callback_a = saved;
    result = -1;
done:
    return result + 1;
}
