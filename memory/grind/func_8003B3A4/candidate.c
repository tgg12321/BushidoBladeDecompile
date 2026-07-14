/* func_8003B3A4 — sandbox distance 0 (re-verified s2b, 2026-07-13, annotation in
   place in src). Judge ruling bd20027f sanctions the FAKE alias per-function, but
   layer-1 cheat-reviewer returned NEEDS_USER on whether retroactive exhaustion cures
   the 2026-03-26 first-reach taint — owner answer required before layer-2 +
   candidate-ready (see tmp/grind/func_8003B3A4/s2/layer1_review_verdicts.md). */
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    {
        /* FAKE: store-only pointer alias — direct symbolic stores expand via the
           assembler sb macro ($at), so the address never enters RA; the pointer
           local makes it an RA-visible pseudo materialized into $v1 pre-branch,
           matching target. Direct/ternary/diamond/offset forms measured 6/6/8/6. */
        u8 *p = &D_8010277D;
        if (a1 != 0) {
            *p = 0xE;
        } else {
            *p = 0x1D;
        }
    }
    D_8010277F = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102783 = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
