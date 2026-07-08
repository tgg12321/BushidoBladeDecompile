/* REJECTED s2: fmt-la-late via block-local staging
 * form A: single-set  `void *fmt = &D_800161C8;` at end of block, then debug_printf(fmt, ...)
 * form B: multi-set   fmt = &D_800161B8; tslTm2LoadImage_2(fmt); ...; fmt = &D_800161C8; debug_printf(fmt, ...)
 *   (form B legit-multi-set: BOTH sets have real consumers, no dead stores)
 * score: masked 2 in BOTH forms (h5 baseline = 2; no change)
 * reason: frontier probe #3 ("fmt-la-late spelling frees a0 for t0's in-place
 *   chain"). GCC's cse.c equiv-const propagation folds `void *` = constant-address
 *   back to the lui/addiu at each USE site regardless of local staging — even
 *   multi-set with legit consumers doesn't hide the two symbol constants from
 *   equiv_constant. The lui/addiu remains scheduled by GCC where it wants,
 *   independent of C source position.
 * verdict: fmt-la-late at the C source level is INERT on the masked metric.
 *   The a0-block on t0 (evidence line 120: "a0 is blocked for t0 by the call's
 *   fmt-la scheduling before t0's last use") is not a source-level dial for
 *   csmd4 — the lui/addiu constant-address home selection is decided post-CSE
 *   and post-scheduling regardless of C local staging.
 * IMPLICATION for the fmt-la-late strategy line: any workable version has to
 *   defeat equiv_constant CSE first (e.g., pass fmt through a function call
 *   or volatile write that GCC can't fold), which requires a non-inert
 *   surrounding change and takes us out of pure-structural modality.
 */
  {
    s32 arg5, t0;
    void **pp;
    void *fmt;         /* form A: single-set */
    fmt = &D_800161B8; /* form B: multi-set (with corresponding tslTm2LoadImage_2(fmt) call hoisted into the block) */
    /* ... rest of h5 body ... */
    fmt = &D_800161C8;
    debug_printf(fmt, *pp, D_800A11DC[D_800A11D5], *(s32*)t0, arg5);
  }
