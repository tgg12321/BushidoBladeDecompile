/* REJECTED (s1, 2026-09-08): for-i with buf[i], tbl[i*2], data[i*2] (data = tbl+1 local): 179 insns, score 41; the data/tbl givs get separate copies and inits in the wrong order. Pointer bivs for tbl/data are required.
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
        s16 *buf = D_800A3750;

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
            if (buf[i] != 0) goto skip;
            buf[i] = 1;
            goto call_default;

        handle_type_2_6:
            {
                s32 adj = type - 2;
                s32 count;
                s32 scount;
                s32 thresh;
                s32 limit;

                count = *(u16 *)&buf[i] + 1;
                buf[i] = count;
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

                buf[i] = 0;
                type = tbl[i * 2];
                if ((u32)(type - 5) < 2) {
                    func_800325E0(counter, D_8008EA00[type]);
                    counter++;
                    continue;
                }
                goto call_default;
            }

        call_default:
            {
                u8 val = data[i * 2];
                func_8005C650(counter, val, val);
            }
        skip:
            counter++;
        loop_bottom:;
        }
    }
}
