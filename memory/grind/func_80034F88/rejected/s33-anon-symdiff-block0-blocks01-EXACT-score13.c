/* s33 (synthesis) -- score 13 at 49 build insns.  BLOCKS 0 AND 1 ARE BYTE-EXACT
 * WITH THE TARGET, the first body in 33 sessions for which that is true, and the
 * first single-declared-pointer body ever to seat the address object in $v1.
 *
 * WHY IT WORKS (measured, tmp/grind/func_80034F88/s33/greg_c1.txt):
 *   The mask statement addresses the byte through an ANONYMOUS symbol-difference
 *   expression.  (minus (symbol_ref D_80106A73) (symbol_ref D_80106A70)) is not
 *   CONSTANT_ADDRESS_P at expand time, so the address is forced into a compiler
 *   temp (pseudo 75) that is born and dies inside block 0's basic block.  That
 *   makes it a LOCAL-ALLOC quantity, and since $v0 holds the live func_80077D00
 *   return, mips.h's absent REG_ALLOC_ORDER hands it $v1 in raw register order.
 *   cse then folds the difference to a plain symbol_ref and rewrites block 1's
 *   `q = &D_80106A73;` into a REGISTER COPY from that temp, which global.c:1709-
 *   1713 converts into a hard-reg PREFERENCE for $v1 on q's allocno (74).  The
 *   hard-reg conflict on register 3 that every previous body printed disappears:
 *     base .greg   `74 conflicts: 72 74 77 78 81 82 85 86 2 3 29`   74 in 4 ($a0)
 *     c1   .greg   `74 conflicts: 72 74 79 80 83 84 87 88 2 29`
 *                  `74 preferences: 3`                             74 in 3 ($v1)
 *   The fold costs zero instructions and zero frame bytes (prologue stays
 *   `addiu sp,sp,-24`, unlike the s30 chain-extender's phantom slot).
 *
 * WHY IT IS ONLY 13:  the seat swap MOVED rather than closed.  q is one C object,
 * hence one pseudo, hence one allocno, hence ONE hard register (global.c:426), so
 * blocks 2 and 3 -- which were register-exact on candidate.c -- now also take $v1
 * where the target uses $a0.  The 3-insn lui/addiu-vs-sb rotation at the block-1
 * join is a consequence, not a separate defect: with block 2 sharing $v1 its
 * address materialisation cannot be hoisted above block 1's store.
 *
 * NOT A SUBMISSION.  The symbol-difference is a FAKE-family construct (the F1
 * combine-foldable chain-extender, owner ruling 2026-07-01) and it measures WORSE
 * than the admissible floor of 10, so no family claim is being made for it here.
 * It is banked as the positive control that the $v1 seat is reachable without a
 * second named pointer object.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) &= 0xF8;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
