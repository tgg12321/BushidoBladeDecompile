/* REJECTED (superseded, not a cheat-blocker but non-natural) — walking pointer
 * PLUS a staging local t3 with the mask store spliced between the third load and
 * its store:
 *     t3 = *p;
 *     D_800A3464 = 0xC06013;
 *     D_800F1148 = t3;
 * Scored 0. But layer-1 cheat-reviewer flagged the t3 staging + mid-sequence
 * reorder as a codegen-only construct (no semantic purpose; only steers the
 * scheduler). It turned out UNNECESSARY: the plain natural sibling form (mask
 * store after all three loads, no t3) also reaches 0. Use the candidate instead. */
end:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    t3 = *p;
    D_800A3464 = 0xC06013;
    D_800F1148 = t3;
}
