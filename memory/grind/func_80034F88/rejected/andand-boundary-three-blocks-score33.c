/*
 * s9. Same boundary construct on all three blocks: the third boundary is
 * pure cost (there is no fourth base to rematerialise).  33 at 55 insns.
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
    if (!c && ptr3) {
        val2 = val;
    }
    *ptr3 = val2;


    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
