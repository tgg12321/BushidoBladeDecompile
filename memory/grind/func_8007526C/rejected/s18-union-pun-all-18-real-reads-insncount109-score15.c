void func_8007526C(void) {
    union hw { u16 u; s16 s; };
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            { union hw a = { *(u16 *)(p + 8) };
              *(u16 *)(p + 8) = a.u + 0xA; }
            { union hw b = { *(u16 *)(p + 0xC) };
              *(u16 *)(p + 0xC) = b.u + 0xA; }
            { union hw c = { *(u16 *)(p + 0xC) };
            if (c.s >= 0xC8) {
                { union hw d = { *(u16 *)(p + 0x10) };
                if ((d.u >> 8) == 0) {
                    union hw e = { *(u16 *)(p + 0x10) };
                    *(u16 *)(p + 0x10) = e.u + 1;
                } }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                { union hw f = { *(u16 *)(p + 0x18) };
                  *(u16 *)(p + 0x14) = f.u; }
                { union hw g = { *(u16 *)(p + 0x38) };
                  *(u16 *)(p + 0x3C) = g.u; }
            } }
            break;
        case 3:
            { union hw b = { *(u16 *)(p + 0xC) };
              *(u16 *)(p + 0xC) = b.u + 0xA; }
            { union hw c = { *(u16 *)(p + 0xC) };
            if (c.s >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                { union hw f = { *(u16 *)(p + 0x18) };
                  *(u16 *)(p + 0x14) = f.u; }
                { union hw g = { *(u16 *)(p + 0x38) };
                  *(u16 *)(p + 0x3C) = g.u; }
                { union hw d = { *(u16 *)(p + 0x10) };
                if ((d.u >> 8) == 0) {
                    union hw e = { *(u16 *)(p + 0x10) };
                    *(u16 *)(p + 0x10) = e.u + 1;
                } }
            } }
            break;
        case 2:
            { union hw b = { *(u16 *)(p + 0xC) };
              *(u16 *)(p + 0xC) = b.u - 0xA; }
            { union hw c = { *(u16 *)(p + 0xC) };
            if (c.s <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            } }
            break;
        case 4:
            { union hw a = { *(u16 *)(p + 8) };
              *(u16 *)(p + 8) = a.u - 0xA; }
            { union hw b = { *(u16 *)(p + 0xC) };
              *(u16 *)(p + 0xC) = b.u - 0xA; }
            { union hw c = { *(u16 *)(p + 0xC) };
            if (c.s <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            } }
            break;
        }
        i++;
    } while (i < 2);
}
