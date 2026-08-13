/*
 * func_80034F88 — REJECTED (grind session s6, forensics modality).
 * Honest sandbox score 24 at 51 build insns; the floor is 18 at 51.
 *
 * Ternary select with explicit (u8) casts in both arms.  Same .greg outcome
 * as ifelse (val2 loses its conflict with the condition pseudo, keeps its
 * conflict with hard reg 2, still gets $v1).  Score 24.
 *
 * RTL dumps for this form: tmp/grind/func_80034F88/s6/rtl/ternary/
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
    val2 = c ? (u8)(val | 1) : (u8)val;
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    val2 = c ? (u8)(val | 2) : (u8)val;
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    val2 = c ? (u8)(val | 4) : (u8)val;
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
