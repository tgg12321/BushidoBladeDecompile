/* REJECTED — s4, 2026-07-13.  THE AGGREGATE FAMILY IS STRUCTURALLY DEAD.
 *
 * This supersedes s3's candidate.c (the Blk16/Blk12 aggregate-typing form, 125/127
 * insns, sandbox 73), which s3 described as "semantically correct and 2 mechanical
 * deltas short", and it KILLS all three of the s3 frontier items at once.
 *
 * =========================================================================
 * THE PROOF (isolated compiler probes, no src/ involvement — reproduce with
 * tmp/grind/func_8001C624/s4/probe_{offset0,array,ptrview,baseptr,arrcast}.c)
 * =========================================================================
 *
 * The target simultaneously contains, for EACH of the three copied regions:
 *     (a) an at-form offset-0 store:  lui $at,%hi(SYM) ; sw $zero,%lo(SYM)($at)
 *     (b) an INDEPENDENT copy base:   lui $a1,%hi(SYM) ; addiu $a1,$a1,%lo(SYM)
 *     (c) a num_regs=2 block copy:    lw,lw,sw,sw / lw,sw
 * i.e. GCC did NOT common (a)'s address with (b)'s register.
 *
 * Every spelling that gives the block copy a SHARED BASE SYMBOL (the dependence
 * that pins it below the stores that feed it) forces GCC to materialise that base
 * in a register at the offset-0 access — CSE then commons it with the copy's base,
 * so (a) degrades to `sw $0,0($3)` (-1 lui x 3 groups) AND (c) degrades to
 * num_regs=3 (3x lw + 3x sw).  s3 recorded these as two INDEPENDENT deltas; they
 * are ONE phenomenon — the shared base register — and neither can be fixed alone.
 *
 * MEASURED, all identical degradation (`la $3,BASE ; sw $0,0($3)` + num_regs=3):
 *   - struct VAR_DECL at offset 0            (probe_offset0.c  f_agg0)
 *   - struct at a NON-ZERO offset in a wider
 *     enclosing object (BIG+184)             (probe_offset0.c  f_aggN)   <-- kills s3 frontier #1
 *   - array VAR_DECL, `ARR[k]`               (probe_array.c    g_arr_copy)
 *   - array VAR_DECL via a cast, `((int*)ARR)[k]`
 *                                            (probe_arrcast.c  m_arr_cast)
 *   - typed struct view through a constant
 *     pointer, `((Work*)&SYM)->a`            (probe_ptrview.c  h_ptrview)
 *   - const local pointer, `Work *w = ...`   (probe_ptrview.c  h_constptr)
 *   - base pointer into a real array,
 *     `int *p = ARR; p[k] = ...`             (probe_baseptr.c  k_baseptr)
 *
 * s3 FRONTIER #1 IS FALSE.  It predicted that putting the copied region at a
 * NON-ZERO offset inside a wider enclosing object would stop the collision.  It does
 * not: the collision is between the copy's base address and the address of the store
 * to the FIRST COPIED WORD, and those are the SAME rtx at ANY enclosing offset,
 * because the copy's base IS the address of the first copied word.  All three copied
 * regions have their first word stored in this function (D_80101F80, D_80101FB0,
 * D_80101FCC — visible in the target asm), so no enclosing object can help.
 *
 * s3 FRONTIER #2 IS FALSE.  It is not a use-count problem to be engineered around:
 * `k_baseptr` degrades with only ONE other use of the base.
 *
 * s3 FRONTIER #3 IS FALSE.  num_regs 3-vs-2 is not downstream RA fallout that "will
 * fall out once #1 lands" — it is the same shared base register, and it reaches 2
 * only when the addresses are compile-time constants.
 *
 * =========================================================================
 * NOT A TOOLCHAIN ARTIFACT
 * =========================================================================
 * The ORIGINAL PsyQ cc1psx (tools/cc1psx_wrapper.sh, GCC 2.7.2.SN.1 — the compiler
 * that actually built the game) produces BYTE-IDENTICAL output to our GCC 2.7.2 port
 * on every probe above.  Zero divergence.  So the original Bushido Blade 2 source
 * genuinely did NOT declare this region as an aggregate — the target's at-form
 * offset-0 stores prove it, and the proof now holds under the real compiler.
 *
 * =========================================================================
 * WHAT THIS LEAVES
 * =========================================================================
 * The unique C form with BOTH properties (constant-address MEMs so the stores keep
 * at-form, AND a shared base symbol so the copies are pinned) is the base+offset
 * re-view over a scalar symbol — `((s32 *)&D_80101F80)[k]` — because a scalar
 * VAR_DECL never engages GCC's aggregate machinery.  That is s2's v13: byte-exact,
 * link-identical, self-rejected as a cheat.  Note `((int*)ARR)[k]` on a REAL array
 * folds back to `ARR[k]` and degrades — so the property comes specifically from
 * declaring the base a SCALAR and indexing past it, i.e. from a declaration that is
 * FALSE about the object.  See hypotheses.md §s4 and the s4 ruling question.
 */

/* The dead form, for the record (s3's candidate.c body — 125/127, sandbox 73). */
void func_8001C624(void) {
    s32 local[3];
    s32 x;
    s32 y;
    s32 z;
    s32 i;

    i = 0x36;
    func_80021D10(0, &((s32 *)&D_80101EC8)[i], (s32)D_800A38E0);
    func_80021D10(1, local, (s32)D_800A38E0);
    D_80101FB0.a = 0;     /* <-- ALWAYS `sw $0,0($reg)`: loses its at-form lui, and
                           *     drags the block copy to num_regs=3.  Unfixable. */
    x = ((s32 *)&D_80101EC8)[0x36];
    y = D_80101FA4;
    z = D_80101FA8;
    D_80101FB0.b = -0x384;
    D_80101FB0.c = 0;
    D_80101FBC = x;
    D_80101FC0 = y - 0x384;
    D_80101FC4 = z;
    D_80101F80.a = x;
    D_80101F80.b = y;
    D_80101F80.c = z;
    D_80101F90 = D_80101F80;
    D_801020C0 = D_80101FB0;
    D_80101FCC.a = 0;
    D_80101FCC.b = 0;
    D_80101FCC.c = 0;
    D_80102114 = D_80101FCC;
    local[0] = local[0], local[1] = local[1] - 0x384, local[2] = local[2];
    D_80101FDC = 0;
    D_80101FE0 = 0;
    D_80101FE4 = 0;
    D_80101FEC = 0;
    D_80101FF0 = 0;
    D_80101FF4 = 0;
    D_80101FFC = 0;
    D_80102000 = 0;
    D_80102004 = 0;
    D_8010200C = 0;
    D_80102014 = 0;
    D_80102018 = 0;
    D_8010201A = 0;
    D_80102016 = 0;
    D_80102010 = D_80101F80.b;
    func_8003FFE0(0);
}
