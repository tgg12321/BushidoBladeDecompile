#define PDER ((u8 *)((s32)p + ((s32)&D_80106A73 - (s32)p)))
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            c = m | 1;
        } else {
            c = m;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 2;
        v = *PDER;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *PDER = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 4;
        v = *PDER;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *PDER = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
#undef PDER
