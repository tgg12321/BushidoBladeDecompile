/* REJECTED (s1, 2026-09-08): fully structured if/else-if dispatch with the default func_8005C650 call duplicated into both arms (no gotos): 173 insns, score 19 (vs 6 = scorer-false floor for the goto/label form K2). The goto dispatch with the shared call_default block is required.
   Chassis: -mel -msoft-float, LeafThreshold pair-struct header decls (tmp/grind/func_800335D8/s1/hdr.py). No FAKE constructs. */
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
        s16 *buf = D_800A3750;

        for (i = 0; i < 4; i++, tbl += 2) {
            u8 *data = tbl + 1;
            s32 rng;
            s32 type;
            s32 rnd;

            rng = rng_Next();
            rnd = rng & 0x3FF;
            type = *tbl;

            if (type == 1) {
                if (buf[i] == 0) {
                    buf[i] = 1;
                    func_8005C650(0x7B + i, *data, *data);
                }
            } else if (type >= 2 && type < 7) {
                s32 adj = type - 2;
                s32 count;
                s32 scount;
                s32 thresh;
                s32 limit;

                count = (u16)buf[i] + 1;
                buf[i] = count;
                scount = (s16)count;
                thresh = D_8008EA44[adj].a;
                limit = thresh * 30;

                if (limit < scount) {
                    s32 ratio = ((scount - limit) << 10) / (D_8008EA44[adj].b * 30);

                    if (rnd < ratio) {
                        buf[i] = 0;
                        type = *tbl;
                        if ((u32)(type - 5) < 2) {
                            func_800325E0(0x7B + i, D_8008EA00[type]);
                        } else {
                            func_8005C650(0x7B + i, *data, *data);
                        }
                    }
                }
            }
        }
    }
}
