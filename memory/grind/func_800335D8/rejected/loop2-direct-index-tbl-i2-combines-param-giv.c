/* REJECTED (s1, 2026-09-08): loop 2 as for-i over D_800A3750[i], tbl[i*2], tbl[i*2+1] (pre-struct chassis, s16 D_800A3750[4] decl): the param address becomes lbu 1($tblgiv) (giv expressed off the type giv), no separate $18 biv; score 62.
   Chassis: -mel -msoft-float, LeafThreshold pair-struct header decls applied (tmp/grind/func_800335D8/s1/hdr.py) unless noted. No FAKE constructs. */
void func_800335D8(void) {
    u8 *slot = D_800A3918;
    LeafPos *pos = D_80107850;
    u8 *tbl = &D_8008E914 + D_800A36A4 * 8;

    do {
        if (*slot != 0) {
            s32 cat = func_80033498();
            u8 cur = *slot;
            s32 idx = cat * 2;

            if (cur == 1) {
                func_800325E0((&D_8008EBF4)[cat], (s32 *)pos);
            } else if (cur == (&D_8008EBFC)[idx]) {
                func_800325E0((&D_8008EBF4)[cat] + 1, (s32 *)pos);
            } else if (cur == (&D_8008EBFD)[idx]) {
                func_800325E0((&D_8008EBF4)[cat] + 2, (s32 *)pos);
            }

            {
                s32 val = *slot + 1;
                *slot = val;
                if ((u32)(&D_8008EBFD)[idx] < (u32)(val & 0xFF)) {
                    *slot = 0;
                }
            }
        }
        slot++;
        pos++;
    } while (slot < &D_800A391E);

    if (func_8001DB58() != 0) {
        s32 counter = 0x7B;
        s32 i;

        for (i = 0; i < 4; i++) {
            s32 rng;
            s32 type;
            s32 rnd;

            rng = rng_Next();
            rnd = rng & 0x3FF;
            type = tbl[i * 2];

            if (type == 1) goto handle_type_1;
            if (type < 2) goto skip;
            if (type < 7) goto handle_type_2_6;
            counter++;
            continue;

        handle_type_1:
            if (D_800A3750[i] != 0) goto skip;
            D_800A3750[i] = type;
            goto call_default;

        handle_type_2_6:
            {
                s32 adj = type - 2;
                s32 adj2 = adj * 2;
                s32 count;
                s32 scount;
                s32 thresh;
                s32 limit;

                count = *(u16 *)&D_800A3750[i] + 1;
                D_800A3750[i] = count;
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

                D_800A3750[i] = 0;
                type = tbl[i * 2];
                if ((u32)(type - 5) < 2) {
                    func_800325E0(counter, (s32 *)(&D_8008EA00 + type * 16));
                    counter++;
                    continue;
                }
                goto call_default;
            }

        call_default:
            {
                u8 val = tbl[i * 2 + 1];
                func_8005C650(counter, val, val);
            }
        skip:
            counter++;
        }
    }
}
