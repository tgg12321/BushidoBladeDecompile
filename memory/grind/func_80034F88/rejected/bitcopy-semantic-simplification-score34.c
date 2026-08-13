/*
 * s17 (rederive) -- the whole-shape semantic re-derivation: the three flag
 * blocks ARE "copy bits 0..2 of p[8] into D_80106A73", so the spec-level C is
 * one statement, `*q = (*q & 0xF8) | (p[8] & 7);`.
 *
 * Sandbox score 34 at 27 insns (target 49).  The branchless form is 22
 * instructions short.  The target BRANCHES per bit (`andi $v0,$v0,1 / bnez /
 * ori $v0,$a0,1 / addu $v0,$a0,$zero` three times), which no branchless
 * spelling can produce, so the original source really did write three
 * if/else read-modify-write blocks rather than a mask-and-merge.
 * The per-bit OR-accumulation spelling (r2_accum) is 30 at 35 insns -- same
 * kill for the same reason: one read and one store instead of four of each.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q = (*q & 0xF8) | (p[8] & 7);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
