/* s60 (rederive, 2026-09-05) -- SAME OBJECT MODEL AS s59, NEW VALUE MODEL.
 * Honest floor is still 10 (49 target insns / 49 build insns, measured this
 * session on HEAD), but this body carries ZERO FAKE-annotated constructs, ZERO
 * declaration puns, ZERO alias handles and ZERO dead stores -- and it is the
 * first body in 60 sessions in which the address object `q` is NOT hard-reg
 * blocked out of $v1.  The previous (s59) body, which buys the target's
 * block-0 reload with one FAKE dead store at the same score, is preserved at
 * memory/grind/func_80034F88/candidate_s59_fake_reload_variant.c.
 *
 * ============================ REQUIRED DECLARATION =========================
 * Unchanged from s59.  THIS BODY DOES NOT COMPILE AGAINST HEAD's HEADER; it
 * requires one header edit plus two element-form use-site edits in the other
 * TU (an integration handoff, reverted at the end of the session so the tree
 * is clean):
 *
 *     include/code6cac.h:472
 *       -  extern u8 D_80106A70;
 *       +  extern u8 D_80106A70[4];
 *     src/code6cac.c:340   D_80106A70 = 0x11;  ->  D_80106A70[0] = 0x11;
 *     src/code6cac.c:345   (D_80106A70 & 0xF)  ->  (D_80106A70[0] & 0xF)
 *
 * s59 measured the declaration byte-neutral project-wide (func_8001945C 0/11,
 * func_80019488 0/14, func_80037F40 0/51); s60 re-used it unchanged.
 *
 * ============================ WHAT s60 CHANGED =============================
 * The value model.  s59's block 0 declared four separate s32 locals
 * (raw / mv / v / c).  `raw` and `mv` never crossed a basic-block boundary, so
 * cc1 handed them to LOCAL-ALLOC as block-local quantities -- and local-alloc
 * seated BOTH of them in $v1.  That is what put a hard-register-3 conflict on
 * the address allocno.  Measured directly this session from the instrumented
 * cc1 dumps (tmp/grind/func_80034F88/s60/, dumps regenerated per variant):
 *
 *   s59 body (.lreg):  Register 75 used 2 times across 2 insns in block 0
 *                      Register 76 used 2 times across 2 insns in block 0
 *                      ;; Register 75 in 3.   ;; Register 76 in 3.
 *   s59 body (.greg):  ;; 74 conflicts: 72 74 77 78 82 83 86 87 2 3 29
 *                                                                 ^ hard $v1
 *
 * Reusing ONE value variable (`v`) for the mask value and for every flag
 * block's value makes every block-0 value pseudo live across the if, so block
 * 0 contains no block-local quantity at all, and the conflict disappears:
 *
 *   this body (.greg): ;; 74 conflicts: 72 74 75 76 2 29        (no 3)
 *
 * $v1 is now legal for the address object for the first time in 60 sessions.
 *
 * ============================ THE RESIDUAL =================================
 * What remains is ONE inequality in global.c's allocno_compare, and it is now
 * fully priced.  Priority is floor_log2(n_refs) * n_refs / live_length; the
 * measured allocnos of this body are
 *
 *   73  i (loop index)   11 refs / 7 insns  -> 4.71   seated $v1
 *   76  c (result/cond)  15 refs / 18 insns -> 2.50   seated $v0 (prefers 2)
 *   75  v (values)       14 refs / 20 insns -> 2.10   seated $v1
 *   74  q (address)      10 refs / 31 insns -> 0.97   seated $a0
 *   72  p                 6 refs / 34 insns -> 0.35   seated $a1
 *
 * and the emitted order ";; 5 regs to allocate: 73 76 75 74 72" matches that
 * ranking exactly.  The target wants 74 in $v1 and 75 in $a0, i.e. 74 must
 * sort ABOVE 75.  Splitting `v` (s60l/s60m: v for the mask + block 0, w for
 * blocks 1-2) lowers 75 to 2.40 and adds 76=w at 1.20 -- still both above
 * 0.97, so the order does not move.
 *
 * ============================ THE STRUCTURAL LIMIT =========================
 * s60 also established WHY the single-pointer chassis cannot finish, whatever
 * the priorities do.  In the target the flag address occupies TWO different
 * hard registers: $v1 in block 0 (la at 80034F98, last read by the store at
 * 80034FD0) and $a0 in blocks 1 and 2 (la at 80034FC8 and 80034FF0).  A single
 * C pointer object is a single allocno, and global.c:1275 writes exactly one
 * hard register per allocno (reg_renumber[allocno_reg[allocno]] = best_reg);
 * GCC 2.7.2 does no live-range splitting.  So the target's address geometry is
 * only reachable if the address is carried by more than one RA object -- which
 * on this chassis means either the pointer-free array spelling (three
 * rematerialised address pseudos: rejected/s59b-..., 50 insns, block 0
 * register-EXACT) or the Judge-granted two-object form, which has never been
 * measured on the aggregate declaration.
 *
 * FAMILY.  `v` reused for the mask value and the flag values is variable reuse
 * of an ordinary local -- no annotation, no coercion, no dead store.  `q` is a
 * pointer into a declared aggregate.  See evidence.md E60.* and hypotheses.md
 * s60.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;
    s32 v;
    s32 c;

    p = func_80077D00();
    q = &D_80106A70[3];

    v = *q;
    v = v & 0xF8;
    *q = v;
    v = *q;
    c = p[8] & 1;
    if (c) {
        c = v | 1;
    } else {
        c = v;
    }
    *q = c;

    q = &D_80106A70[3];
    v = *q;
    c = p[8] & 2;
    if (c) {
        c = v | 2;
    } else {
        c = v;
    }
    *q = c;

    q = &D_80106A70[3];
    v = *q;
    c = p[8] & 4;
    if (c) {
        c = v | 4;
    } else {
        c = v;
    }
    *q = c;

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
