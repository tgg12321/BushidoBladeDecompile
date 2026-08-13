/*
 * func_80034F88 — REJECTED (grind session s6, forensics modality).
 * Honest sandbox score 35 at 61 build insns; the floor is 18 at 51.
 *
 * The store duplicated into both arms of the if/else.  Score 35 at 61
 * insns — GCC emits a `j` around the else arm and a second `lui at`
 * per arm, i.e. two full symbol-addressed stores per block.  This is
 * the sanctioned duplicated-statement-into-arms shape and it is
 * measured DEAD here (worst result of the session).
 *
 * RTL dumps for this form: tmp/grind/func_80034F88/s6/rtl/ifelse_dupstore/
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
    val = D_80106A73;
    if (c) {
        D_80106A73 = val | 1;
    } else {
        D_80106A73 = val;
    }

    c = p[8] & 2;
    val = D_80106A73;
    if (c) {
        D_80106A73 = val | 2;
    } else {
        D_80106A73 = val;
    }

    c = p[8] & 4;
    val = D_80106A73;
    if (c) {
        D_80106A73 = val | 4;
    } else {
        D_80106A73 = val;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
