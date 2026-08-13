#define BB2_SET_FLAG(cond, bit)          {                                        u8 *q = &D_80106A73;                 s32 v;                               s32 c;                                                                    c = (cond);                          v = *q;                              if (c) {                                 c = v | (bit);                   } else {                                 c = v;                           }                                    *q = c;                          }

void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    {
        u8 *q = &D_80106A73;

        *q &= 0xF8;
    }

    BB2_SET_FLAG(p[8] & 1, 1)

    BB2_SET_FLAG(p[8] & 2, 2)

    BB2_SET_FLAG(p[8] & 4, 4)

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
