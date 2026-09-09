/* REJECTED (s1, 2026-09-08): for (i = 0; i < 4; i++, p++, data += 2, tbl += 2) with *p uses: biv i has no giv, so i survives as a live counter (addu s2,zero); score 33 vs 20 for the do-while pointer form.
   Chassis: -mel -msoft-float, LeafThreshold pair-struct header decls applied (tmp/grind/func_800335D8/s1/hdr.py) unless noted. No FAKE constructs. */
void func_800335D8(void) {
    s32 i;
    u8 *tbl = D_8008E914[D_800A36A4];

    for (i = 0; i < 6; i++) {
        if (D_800A3918[i] != 0) {
            s32 cat = func_80033498();
            u8 cur = D_800A3918[i];

            if (cur == 1) {
                func_800325E0(D_8008EBF4[cat], (s32 *)&D_80107850[i]);
            } else if (cur == D_8008EBFC[cat].a) {
                func_800325E0(D_8008EBF4[cat] + 1, (s32 *)&D_80107850[i]);
            } else if (cur == D_8008EBFC[cat].b) {
                func_800325E0(D_8008EBF4[cat] + 2, (s32 *)&D_80107850[i]);
            }

            {
                s32 val = D_800A3918[i] + 1;
                D_800A3918[i] = val;
                if ((u32)D_8008EBFC[cat].b < (u32)(val & 0xFF)) {
                    D_800A3918[i] = 0;
                }
            }
        }
    }

    if (func_8001DB58() != 0) {
        u8 *data = tbl + 1;
        s32 counter = 0x7B;
        s16 *p = D_800A3750;

        for (i = 0; i < 4; i++, p++, data += 2, tbl += 2) {
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
            *p = 1;
            goto call_default;

        handle_type_2_6:
            {
                s32 adj = type - 2;
                s32 count;
                s32 scount;
                s32 thresh;
                s32 limit;

                count = *(u16 *)p + 1;
                *p = count;
                scount = (s16)count;
                thresh = D_8008EA44[adj].a;
                limit = thresh * 30;

                if (limit >= scount) goto skip;

                {
                    s32 diff = scount - limit;
                    s32 thresh2 = D_8008EA44[adj].b;
                    s32 limit2 = thresh2 * 30;
                    s32 ratio = (diff << 10) / limit2;

                    if (rnd >= ratio) goto skip;
                }

                *p = 0;
                type = *tbl;
                if ((u32)(type - 5) < 2) {
                    func_800325E0(counter, D_8008EA00[type]);
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
        loop_bottom:;
        }
    }
}
