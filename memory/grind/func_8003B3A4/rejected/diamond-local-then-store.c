/* REJECTED s2 (2026-07-13): sandbox distance 8 (53 vs 56 insns) — WORSE.
   Staging the value through a local collapses the diamond further (GCC folds
   the const arms / drops the j-over shape); still $at macro store. */
    {
        u8 v;
        if (a1 != 0) {
            v = 0xE;
        } else {
            v = 0x1D;
        }
        D_8010277D = v;
    }
