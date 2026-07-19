/* KILLED s4 (2026-07-19): frontier #1 variant — `s32 v1;` declared before
 * other decls, s4's init inlines a0<<4 anonymously, then v1 = a0<<4 assigned
 * late (after i, s4, count, s5). Was: hoped decl-first + late assign would
 * decouple RA/LUID; simultaneously test target's schedule via the inline
 * anon shift.
 *
 * Result: sandbox --disable all -> score=2, build_insns=84 (target 83).
 * OBJDUMP prologue (target-like schedule order + CSE-move copy):
 *   sw   $16,16($sp)
 *   move $16,$18
 *   sll  $2,$18,0x4      <-- ANON shift into v0 (from s4's init)
 *   move $3,$2           <-- CSE-move copy propagating v0 -> v1
 *
 * VERDICT KILLED: same-shape near-hit as cse-fold-anon-shift.c. Confirms
 * the LUID lever WORKS (schedule order matches target) FROM A DIFFERENT
 * ANGLE (decl-early + assign-late v1 + inline anon a0<<4 in s4). The
 * CSE-move copy is inherent to any spelling that has TWO tree-level
 * a0<<4 expressions — GCC's CSE reuses the earlier anon pseudo but must
 * copy into v1's separate declared pseudo.
 *
 * CROSS-ANGLE PROOF (compared to cse-fold-anon-shift.c):
 * - cse-fold-anon-shift declared v1 AFTER s4's init (v1's assignment
 *   already late) — schedule right, +1 copy.
 * - this form declares v1 BEFORE all other decls but ASSIGNS it after s5
 *   — schedule right, +1 copy.
 * Same closing insn+1 diff from two different C shapes. The blocker is
 * NOT statement position; it is the presence of two tree-level a0<<4
 * expressions.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 v1;
    s32 sum = 0;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + (a0 << 4));
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    v1 = a0 << 4;
    /* ... body unchanged ... */
}
