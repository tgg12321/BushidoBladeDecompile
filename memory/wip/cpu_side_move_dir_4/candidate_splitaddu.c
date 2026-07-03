/* SPLIT-ADDU candidate (2026-07-03 session 3b) — masked 4, w=a0 SOLVED.
   Requires BOTH edits: the printf block AND the check2 copy-walker
   unification (src -> w). Residual: p_t (in-call addu temp) = v1 vs
   target a0 (masked-invisible) + 6-slot schedule permutation
   [sll4@58->55, addu4@61->59, lw5/11D5/sll-11D5/sw one-slot shifts].
   Mechanism notes in notes.md session-3b addendum. */

/* decls added to function scope: */
/*   s32 w;  (replaces src in check2; u8 *src DELETED)  */
/*   void **pp;  */

/* printf block: */
  {
    s32 arg5;
    w = idx_1494[0];
    arg5 = tbl_125c[idx_1494[1]];
    pp = (void **)&D_800F19C0; /* FAKE */
    w <<= 2;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)(w + (s32)tbl_125c), arg5);
  }

/* check2 copy loop (src unified into w): */
/*
    *idx_1494 = 2;
    dst = a1;
    w = (s32)(&D_800F19A0);
    i = 7;
    if (a1 != 0)
    {
      do
      {
        b = *(u8 *)w;
        w++;
        i--;
        *dst = b;
        dst++;
      }
      while (i != (-1));
    }
*/
