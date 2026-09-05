/* S59-BEGIN */
/* s59 rederive vA: ARRAY OBJECT MODEL.  D_80106A70 is declared `u8[4]` and the
 * flag byte is its element [3] -- no C pointer object aliases &D_80106A73 at
 * all, and the trailing loop's `*(&D_80106A70 + i)` declaration pun is gone. */
void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();

    {
        s32 v;
        s32 c;

        D_80106A70[3] = D_80106A70[3] & 0xF8;
        v = D_80106A70[3];
        c = p[8] & 1;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        D_80106A70[3] = c;
    }

    {
        s32 v;
        s32 c;

        v = D_80106A70[3];
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        D_80106A70[3] = c;
    }

    {
        s32 v;
        s32 c;

        v = D_80106A70[3];
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        D_80106A70[3] = c;
    }

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
/* S59-END */
