/*
 * s16 DIAGNOSTIC ONLY -- NEVER COMMITTABLE (register-asm pins are a forbidden
 * family; this file exists solely as the recorded measurement).
 * `register u8 *q asm("$3")` on the candidate chassis. GCC 2.7.2 IGNORES the
 * pin for a local that never appears as an asm operand: the emitted stream is
 * INSTRUCTION-IDENTICAL to the unpinned candidate (score 10, same census). So
 * even the illegitimate route cannot move the single pointer allocno onto $v1.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    register u8 *q asm("$3");

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

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
