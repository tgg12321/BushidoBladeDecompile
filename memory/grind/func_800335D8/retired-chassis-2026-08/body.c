void func_800335D8(void) {
    u8 *slot = &D_800A3918;
    u8 *pos = (u8 *)&D_80107850;
    u8 *tbl = &D_8008E914 + D_800A36A4 * 8;
    int new_var;

    do {
        if (*slot != 0) {
            s32 cat = func_80033498();
            u8 cur = *slot;
            s32 idx = cat * 2;
            s32 a0_val;

            if (cur == 1) {
                a0_val = (&D_8008EBF4)[cat];
                goto do_call;
            }
            if (cur == (&D_8008EBFC)[idx]) {
                a0_val = (&D_8008EBF4)[cat] + 1;
                goto do_call;
            }
            if (cur == (&D_8008EBFD)[idx]) {
                a0_val = (&D_8008EBF4)[cat] + 2;
            do_call:
                func_800325E0(a0_val, (s32 *)pos);
            }

            {
                s32 val = *slot + 1;
                *slot = val;
                idx = 0;
                new_var = cat * 2;
                if ((u32)(&D_8008EBFD)[new_var] < (u32)(val & 0xFF)) {
                    *slot = idx;
                }
            }
        }
        slot++;
        pos += 12;
    } while ((s32)slot < (s32)&D_800A391E);

    if (func_8001DB58() != 0) {
        u8 *data = tbl + 1;
        s32 counter = 0x7B;
        s16 *base = &D_800A3750;
        s16 *p = base;

        do {
            s32 rng;
            s32 type;
            s32 rnd;

            rng = rng_Next();
            rnd = rng & 0x3FF;
            type = *tbl;

            if (type == 1) goto handle_type_1;
            if (type < 2) goto skip;
            if (type < 7) goto handle_type_2_6;
            counter++;
            goto loop_bottom;

        handle_type_1:
            if (*p != 0) goto skip;
            *p = type;
            goto call_default;

        handle_type_2_6:
            {
                s32 adj = type - 2;
                s32 adj2 = adj * 2;
                s32 count;
                s32 scount;
                s32 thresh;
                s32 limit;

                count = *(u16 *)p + 1;
                *p = count;
                scount = (s16)count;
                thresh = (&D_8008EA44)[adj2];
                limit = thresh * 30;

                if (limit >= scount) goto skip;

                {
                    s32 diff = scount - limit;
                    s32 thresh2 = (&D_8008EA45)[adj2];
                    s32 limit2 = thresh2 * 30;
                    s32 ratio = (diff << 10) / limit2;

                    if (rnd >= ratio) goto skip;
                }

                *p = 0;
                type = *tbl;
                if ((u32)(type - 5) < 2) {
                    func_800325E0(counter, (s32 *)(&D_8008EA00 + type * 16));
                    counter++;
                    goto loop_bottom;
                }
                goto call_default;
            }

        call_default:
            {
                u8 val = *data;
                func_8005C650(counter, val, val);
            }
        skip:
            counter++;
        loop_bottom:
            p++;
            data += 2;
            tbl += 2;
        } while ((s32)p < (s32)(base + 4));
    }
}
