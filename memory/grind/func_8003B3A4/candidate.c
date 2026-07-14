/* func_8003B3A4 — sandbox distance 0 (re-verified s2d, 2026-07-14, annotation
   restored in src after THIRD hygiene drop — must land in a COMMIT to stick, per
   the 23:22 Judge ruling). s2d added the final family kill: multi-use pointer
   over the cluster (p[0]+p[2]) scores 2 — target stores D_8010277F via the $at
   macro, so the alias is irreducibly write-only single-target. Exhaustion now:
   direct 6 / ternary 6 / diamond-local 8 / offset-fold 6 / multi-use 2.
   BLOCKED ON OWNER ONLY: layer-2 FAIL on process ground (Judge self-answered the
   s2b layer-1 NEEDS_USER reserved for the human); question escalated verbatim to
   Trenton in docs/grind/decisions.md "2026-07-13 23:22". Do NOT re-run layer-2
   until Trenton answers (HOLD constraint). */
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
