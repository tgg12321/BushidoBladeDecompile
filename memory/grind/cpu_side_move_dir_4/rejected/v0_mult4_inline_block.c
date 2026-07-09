  {
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    v0 = idx_1494[1]; /* FAKE: index staged through the (dead-here) v0 var per staged-value-reused-variable (owner-sanctioned 2026-07-03); mechanism: sched.c adjust_priority/birthing_insn_p; prior v0 value dead (re-set below before any read); lever-exhaustion: twin session-4, mirrors marionation */
    pp = (void **)&D_800F19C0; /* FAKE: pointer-alias staging the D_800F19C0 load early; mechanism: local-alloc.c update_equiv_regs refs-2 sink defeat; lever-exhaustion in WIP history */
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 = v0 * 4; /* PROBE A: route through expand_mult for symmetric LAUNCH */
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
