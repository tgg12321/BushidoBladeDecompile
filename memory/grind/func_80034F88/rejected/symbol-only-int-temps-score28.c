/* REJECTED (measured, s1): symbol-only accesses with s32-typed temporaries.
 * sandbox --disable all = 28.  47 build insns vs target 49.
 *
 * HYPOTHESIS THIS KILLS: "the target's `lbu` reloads exist because the source
 * reads the byte into an int-typed intermediate, so the read is a
 * (zero_extend:SI (mem:QI ...)) rtx that cse.c cannot satisfy from the
 * QImode value it just stored."  Measured false — with s32 temps the read is
 * still forwarded (objdump shows a single `lbu` for the whole flag section,
 * exactly as with u8 temps), and the score gets WORSE than the u8 version
 * because promoting the temps costs an extra copy in each arm.
 *
 * Integer-vs-byte typing of the temporaries is therefore NOT the lever.
 */
void func_80034F88(void) {
    s32 *p;
    s32 val;
    s32 val2;
    s32 i;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    val = D_80106A73;
    val2 = val | 1;
    if (!(p[8] & 1)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    val2 = val | 2;
    if (!(p[8] & 2)) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    val2 = val | 4;
    if (!(p[8] & 4)) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
