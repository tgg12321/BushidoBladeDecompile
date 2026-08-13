/*
 * REJECTED (s10, synthesis) — from w2_staged_before_store.c
 *
 * s10 wave W/Z — three separate pointer LOCALS, each staged where the target
 * materialises a base, on the variable-reuse chassis: 23 at exactly 49
 * instructions.  The instruction SHAPE is the target's, but the allocator gives
 * every base the same hard register (a1) and evicts `p` from a1 into a0, which
 * cascades through the whole function; and because all three bases share one
 * register, the second base cannot be materialised before block 1's store the
 * way the target does.  Every placement and declaration order measured the same
 * 23: w1 (block-top) 23/49, w2 (staged before the previous store) 23/49,
 * z6 (extra pointers declared last) 23/49, z7 (`p` declared last) 23/49,
 * z8 23/49, w9 (block-scoped pointers) 23/49, w7 (two pointers, no
 * re-assignment) 23/47, z3/z4/z5 (one extra pointer) 23/47.
 * The form that DOES supply three bases is a single pointer variable
 * re-assigned (z1 10/49) or two variables one of which is re-assigned
 * (zz5 9/49) — see candidate.c, including the classification caveat.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    u8 *ptr3;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    ptr2 = &D_80106A73;
    *ptr = c;

    c = p[8] & 2;
    val = *ptr2;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    ptr3 = &D_80106A73;
    *ptr2 = c;

    c = p[8] & 4;
    val = *ptr3;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    *ptr3 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
