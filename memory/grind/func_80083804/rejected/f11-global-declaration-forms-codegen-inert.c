/*
 * REJECTED — frontier F11 (session 9): the DECLARATION of the three globals
 * motion_Close touches is codegen-inert. KILLED.
 *
 * F11 was the successor to F10 and the last structural input to cc1 that no
 * session had varied: sessions 1-7 measured the body exhaustively and session 8
 * measured everything above the body (signature, inlining, TU order, helper
 * parameterisation) inert, leaving only how D_800A2668 / D_8008D070 /
 * D_00000000 are DECLARED. The hope was that a different declared type could
 * (a) change the addressing GCC emits, or (b) let two address materialisations
 * collapse into one, either of which would change the instruction count and
 * force the residual table to be re-derived.
 *
 * PROBE. tmp/grind/motion_Close/s9/declsweep.py — ten declaration forms, each
 * applied to the extern lines AND to both users of the symbols (motion_Close
 * and its sibling func_80083794), with the floor-13 body held fixed, each
 * measured with `sandbox motion_Close --disable all` AND with the instrumented
 * cc1 (BB2_ALLOC_DEBUG=1) reading the per-allocno table. NO `asm("Sym")` alias
 * rename was measured — that is a forbidden family
 * (.claude/rules/inline-asm-injection.md § alias renames) and an aliased
 * variant must not even be scored.
 *
 * The ten forms (log: tmp/grind/motion_Close/s9/declsweep.log):
 *   e0_base                 extern s32 D_800A2668; extern void (*D_8008D070)(void);
 *                           extern s32 D_00000000;                       [reference]
 *   e1_tbl_array            extern void (*D_8008D070[])(void);  p = D_8008D070
 *   e2_cnt_array            extern s32 D_00000000[];            count = (s32)D_00000000
 *   e3_both_array           both of the above
 *   e4_cnt_char             extern char D_00000000[];           (linker-provided count
 *                                                                symbol as a char array)
 *   e5_tbl_const            extern void (*const D_8008D070)(void); p is void (*const *)
 *   e6_guard_u32            extern u32 D_800A2668;
 *   e7_decl_order           the three externs emitted in reverse order
 *   e8_cnt_voidp            extern void *D_00000000;
 *   e9_tbl_fnarray_cnt_char e1 + e4 together
 *
 * RESULT. All ten score 13, all ten emit build_insns 25, and all ten produce
 * the byte-identical allocno table:
 *     ord=0 pseudo=73 hardreg=16 nrefs=10 livelen=8 pri=37500   (p    -> $s0)
 *     ord=1 pseudo=72 hardreg=17 nrefs=8  livelen=7 pri=34285   (count -> $s1)
 * Not one form moved a register, an instruction, or a reference count.
 *
 * MECHANISM (why this was always going to be inert, stated so no session
 * re-opens it). The two address materialisations are `lui/addiu` of a %hi/%lo
 * symbol pair. Under -G0 an extern object of ANY type is addressed that way,
 * and the declared type only affects (i) the pointer arithmetic scale factor
 * and (ii) whether an array name decays. Here p is incremented by the size of
 * a function pointer either way, and the count symbol's VALUE is never loaded —
 * only its ADDRESS is taken — so the count declaration's type cannot even
 * influence a load width. The two symbols live at unrelated addresses
 * (0x8008D070 and 0x00000000), so no struct or array wrapping can make one
 * address materialisation serve both. And the frame residual is
 * signature-independent (H1) while the $v0-vs-$t0 residual is allocation-order-
 * independent (F5), so neither of the two largest residual buckets has a
 * declaration-shaped input at all.
 *
 * CONSEQUENCE. With F10 (session 8) and F11 (session 9) both dead, EVERY input
 * to cc1 outside the function body proper is now measured codegen-inert for
 * this function: the signature, the callee/inlining structure, the TU order,
 * and the declarations. The only remaining variable is the body, and the body
 * is bounded below at 13 by three backend-level disproofs (H1, F5, F7).
 */

/* The reference form; the other nine differ only in the three extern lines and
   the two initialiser expressions, and are reproduced by declsweep.py. */
extern s32 D_800A2668;
extern void (*D_8008D070[])(void);   /* e1: array-typed constructor table */
extern char D_00000000[];            /* e4: linker-provided count symbol    */

void motion_Close(void) {
    s32 count;
    void (**p)(void);

    if (D_800A2668 != 0) {
        do { do { do { p = D_8008D070; } while (0); } while (0); } while (0);
        count = (s32)D_00000000;
        if (count != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
