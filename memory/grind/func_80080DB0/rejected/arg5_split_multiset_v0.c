/* REJECTED s2: arg5 split-load through multi-set v0
 * form: v0 = *(s32*)(v0 + (s32)tbl_125c); arg5 = v0;
 *   (instead of h5's arg5 = *(s32*)(v0 + (s32)tbl_125c);)
 * score: masked 12 (h5 baseline = 2)
 * reason: literal reading of frontier probe #2 ("keep the lw dest split:
 *   load into the existing multi-set v0, then arg5 = v0") REGRESSES by 10.
 *   The lw's dest becoming v0 (already multi-set from prior idx_1494[1]
 *   staging) re-times the arg5 load far from its target slot — a very
 *   different allocation web forms. Evidence-bank line 130 records
 *   "k2=6=g3" for this shape in a prior session, but the sandbox
 *   metric here in HEAD's WIP-imported base is 12 — likely because the
 *   surrounding h5 chain (v0=idx[1]; v0<<=2; then v0=lw) triple-sets v0
 *   over the window, which the k-session measured differently.
 * verdict: the LITERAL "load into multi-set v0 then arg5=v0" spelling is
 *   KILLED on the h5 base in this session. Any k2 revival would need a
 *   different v0 usage pattern (fewer prior sets in the block) — which
 *   collapses back to a fresh single-set carrier (WIP-recorded as folding).
 */
  {
    s32 arg5, t0;
    void **pp;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    v0 = *(s32 *)(v0 + (s32)tbl_125c);
    arg5 = v0;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
  }
