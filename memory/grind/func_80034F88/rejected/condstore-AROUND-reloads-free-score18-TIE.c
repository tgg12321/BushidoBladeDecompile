/* s8 (rederive). score 18 / 42 insns — TIES the floor, does NOT beat it.
 * THE FINDING: this non-volatile, label-free form reproduces ALL FOUR of the
 * target lbu reloads at ZERO instruction cost, via cse.c invalidate_skipped_
 * block (7843-7867) -> invalidate_skipped_set (7810-7836): a MEM store inside
 * a conditionally-SKIPPED block is INVALIDATED, never recorded, so the next
 * read of the byte cannot be forwarded.  It is NOT usable for this target,
 * because the mechanism requires the store to sit INSIDE the arm while the
 * target's store is unconditional (0x80034FC4 'addu $v0,$a0,$zero' on the
 * fall-through path + a single 'sb' after the join label).  42 vs 49 insns:
 * 3 missing value-select moves + 4 missing address-materialisation insns.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        *ptr = val | 1;
    }

    c = p[8] & 2;
    val = *ptr;
    if (c) {
        *ptr = val | 2;
    }

    c = p[8] & 4;
    val = *ptr;
    if (c) {
        *ptr = val | 4;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
