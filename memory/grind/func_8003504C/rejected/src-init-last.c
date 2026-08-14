/* REJECTED — floor 24 -> 26, no allocation flip.
 * H4: priority is reg_n_refs/live_length, so shortening src's live range
 * (assign it last, after base/ptr) raises its ratio above i's 7/L.
 * Killed s1 — the achievable shortening is a handful of insns out of a ~40-insn
 * range, nowhere near enough to overcome a 7-vs-5 ref gap, and the reordered
 * init costs 2. Live-range shortening of src is a dead axis; the working
 * direction is LENGTHENING i's range (H3, confirmed).
 */
    p = func_80077D00();
    i = 0;
    base = &D_80102785;
    ptr = (u8 *)(base - 9);
    src = (u8 *)p;      /* <-- moved to last init */
