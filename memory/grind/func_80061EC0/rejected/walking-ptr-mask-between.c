/* REJECTED (score 5, s1 2026-07-23) — walking pointer but mask store BETWEEN 1144 and 1148.
 * Correct register alloc (mask->$v1, arg0->$v0) but the mask chain lui->ori->sw out-prioritizes
 * the 1140 store-addr lui and grabs arg0[0]'s delay slot -> whole mask chain scheduled one load
 * too early. FIX: move the mask store to the FINAL statement (see candidate.c) -> distance 0. */
end:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800A3464 = 0xFF00FF;   /* too early: forces mask chain into arg0[0]/arg0[1] delay slots */
    D_800F1148 = *p;
}
