/* s45: block 0 spelled through the bare symbol (no pointer object at all), q declared and first assigned at block 1's head. 16 at 51. .greg: q (74) still in $a0 with hard reg 3 in its conflict row and NO preference for 3 -- the address is re-materialised at the top of the function instead of being copied from block 0's anonymous temp, so global.c:1725 set_preference never fires. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    D_80106A73 = D_80106A73 & 0xF8;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
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
