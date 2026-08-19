/* s8 REJECTED: score 16 at 137 insns (baseline 11 at 135). Moving `idx++` from the
 * head of the inner-loop body to after DrawSync()/`tbl += 2` extends idx's live
 * range across the LoadImage and DrawSync calls and costs two instructions. */
    s32 off = idx << 5;
    ... LoadImage ... DrawSync(0); tbl += 2; idx++;
