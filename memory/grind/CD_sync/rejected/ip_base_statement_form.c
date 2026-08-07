/* REJECTED s2: ip statement-form base
 * form: t0 <<= 2; t0 += (s32)tbl_125c; (as statements, not *=4 / u8*-cast)
 * score: masked 7 (baseline h5 = 2)
 * reason: RE-CONFIRMS session-4 addendum notes.md:47-48 ("t0<<=2 at same
 *   position = 7 here, 9 mar — the *=4 temp's launch is head-load-bearing").
 *   The *= 4 spelling with the (u8*)tbl+t0 addu is masked-load-bearing;
 *   swapping to plain <<=2 / t0+=(s32)tbl regresses by 5. Frontier probe #1
 *   as literally described ("statement-form ip base expecting ~6") does NOT
 *   yield ~6 masked here — the session-4 measurement is stable across two
 *   probes.
 * verdict: the ip statement-form as a REPLACEMENT for h5's *=4 addu-spelling
 *   is KILLED for csmd4's masked metric. To measure raw-6 order-perfect
 *   would need adiff / raw window diff (not built this session).
 */

/* function excerpt only — see src/system.c HEAD for surrounding context */
  {
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    t0 <<= 2;                    /* was: t0 *= 4;                  */
    t0 += (s32)tbl_125c;         /* was: t0 = (s32)((u8*)tbl+t0);  */
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
  }
