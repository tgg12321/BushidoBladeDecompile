/* s21 PROBE D: fully inline arg5, no v0 = idx_1494[1] staging.
   masked=4 (+2 vs h5 baseline). NOVEL DATA POINT:
   Removing the fn-scope v0 SET from the arg5 chain REGRESSES the h5 basin by +2.
   The v0 pseudo's presence in the arg5 chain (with `v0 = idx_1494[1]` set +
   subsequent `v0 <<= 2` in-place SLL) is empirically load-bearing for h5's
   masked=2 alignment.

   Mechanism: without `v0 = idx_1494[1]` as a first SET, the arg5 index-load
   becomes a single-set fresh dest from expand — this new pseudo takes on
   LAUNCH via birthing_insn_p, but its LUID/qty differ from the staged-v0
   form. The result is a distinct sched2 outcome tipping into a new tiebreak.

   This EXPLAINS why the sanctioned staged-value-reused-variable pattern
   (staging idx into currently-dead v0) is the specific mechanism that
   reaches masked=2 — the v0 SET count and LUID at the arg5 chain head are
   both required.
*/
    t0 = idx_1494[0];
    pp = (void **)&D_800F19C0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    arg5 = *(s32 *)((idx_1494[1] << 2) + (s32)tbl_125c);
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
