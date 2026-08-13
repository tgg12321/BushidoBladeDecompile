/*
 * s9. The cheapest cse basic-block BOUNDARY that works with the target's
 * UNCONDITIONAL store: `if (!c && ptrN)` gives the join label LABEL_NUSES 2
 * (expand_expr for TRUTH_ANDIF_EXPR emits jumpifnot on each operand to the
 * same false-label), so cse_end_of_basic_block's extension test at
 * cse.c:8112 declines and the block ends AT the join.  Two boundaries buy
 * back the target's three addend-0 lui+addiu bases (lui 4, target 4) and the
 * block-2/3 reloads (lbu 4).  Cost: one conditional branch per boundary.
 * score 31 at 53 insns vs the floor's 18 at 51.  `&& ptrN` is also an
 * INSTRUMENT, never a candidate: ptrN is never null, so the operand has no
 * semantic purpose (cheat-checklist T1/T2).
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr1;
    u8 *ptr2;
    u8 *ptr3;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr1 = &D_80106A73;
    *ptr1 &= 0xF8;

    c = p[8] & 1;
    val = *ptr1;
    val2 = val | 1;
    if (!c && ptr1) {
        val2 = val;
    }
    ptr2 = &D_80106A73;
    *ptr1 = val2;

    c = p[8] & 2;
    val = *ptr2;
    val2 = val | 2;
    if (!c && ptr2) {
        val2 = val;
    }
    *ptr2 = val2;
    ptr3 = &D_80106A73;

    c = p[8] & 4;
    val = *ptr3;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    *ptr3 = val2;


    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
