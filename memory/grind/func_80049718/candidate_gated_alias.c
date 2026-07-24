/* func_80049718 — GATED close to floor 0 (sandbox distance 0, genuine/not-masked,
 * re-confirmed s2 for BOTH spellings). The ONLY delta vs the clean floor-4
 * candidate.c is the prologue address computation. Two equivalent spellings:
 *
 *   POINTER (preferred — matches the tslPolyF4Init sanctioned lever verbatim):
 *     s16 *tbl = D_800EF980;
 *     p_anim = tbl + arg0;        // replaces p_anim = &D_800EF980[arg0];
 *
 *   INTEGER (same mechanism, less natural):
 *     s32 base = (s32)D_800EF980;
 *     p_anim = (s16 *)(base + arg0 * 2);
 *
 * WHY IT CLOSES CLUSTER A (named GCC-pass mechanism):
 *   `fold` canonicalizes `&D_800EF980[arg0]` = PLUS(mult(arg0,2), symbol_ref)
 *   with the symbol_ref (address constant) as operand-2, so the INDEX ashift is
 *   expanded/born first (-> $v0) and local-alloc coalesces the BASE symbol_ref
 *   directly into the destination $s0 (built in place: lui $s0; addiu $s0;
 *   addu $s0,$v0,$s0), and the whole addr comp is hoisted early. Target keeps
 *   the base in its own temp: lui $v0; addiu $v0; sll $v1,$s6,1; addu $s0,$v1,$v0
 *   (base born first, its own pseudo). Staging the base into a live pseudo
 *   (`tbl`/`base`) gives the base its OWN register born before the index ->
 *   base->$v0, index->$v1, `addu s0,v1,v0` == target, placed late.
 *
 * EXHAUSTION (s2 — clean pure-C forms without a base-holding pseudo, all dead):
 *   &D_800EF980[arg0]                          -> 4 (baseline)
 *   (s16*)((s32)D_800EF980 + arg0*2)           -> 4
 *   { s32 idx=arg0*2; ...(s32)D_800EF980+idx } -> 4
 *   p_anim = D_800EF980; p_anim += arg0;       -> 9 (cse re-folds)
 *   p_anim = D_800EF980 + arg0                 -> 11 (s1)
 *   (s16*)((s32)&D_800EF980[0]+arg0*2)         -> no change (s1)
 *
 * CLASSIFICATION (the s2 ruling-request): this base-pointer local is EITHER
 *  (a) ordinary pointer arithmetic — register-alloc-pure-c documents the
 *      IDENTICAL lever `s32 *base = arr; elem = base + idx;` as tslPolyF4Init's
 *      Confirmed COMPLETED-C closure ("Ordinary pointer arithmetic", NO FAKE),
 *      OR
 *  (b) a pointer-alias-fake-exception (owner ruling 2026-07-01) — single-use
 *      local, second handle to a global, purpose is to steer base-register
 *      birth order; requires /* FAKE */ annotation + exhaustion + layer-1/2.
 * s1's layer-1 cheat-reviewer FAILed it under (b). The prerequisites for (b)
 * are now ALL satisfiable (exhaustion above + named mechanism above + the
 * annotation below). Owner ruling requested before committing either way. */

/* If ruled (b), the committable form carries this annotation:
 * FAKE: base-pointer local forces the D_800EF980 table address into its own
 *       pseudo so local-alloc births base before index (fold puts symbol_ref
 *       as PLUS operand-2 -> index-first + base-coalesced-into-$s0); yields
 *       target's addu $s0,$v1,$v0. Clean respellings measured dead at floor 4. */
