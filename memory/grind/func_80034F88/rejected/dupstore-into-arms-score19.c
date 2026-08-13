/* s8 (rederive). score 19 / 46 insns, sb 7.  Store DUPLICATED INTO BOTH ARMS
 * (the sanctioned duplicated-statement-into-arms family): it does put the
 * store inside a cse-skipped block and does recover the reloads (lbu 4), but
 * jump2's find_cross_jump merges only ONE of the three duplicate pairs, so
 * the build carries 7 sb where the target has 5.  Worse than the floor.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        val2 = val | 1;
        *ptr = val2;
    } else {
        val2 = val;
        *ptr = val2;
    }

    c = p[8] & 2;
    val = *ptr;
    if (c) {
        val2 = val | 2;
        *ptr = val2;
    } else {
        val2 = val;
        *ptr = val2;
    }

    c = p[8] & 4;
    val = *ptr;
    if (c) {
        val2 = val | 4;
        *ptr = val2;
    } else {
        val2 = val;
        *ptr = val2;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
