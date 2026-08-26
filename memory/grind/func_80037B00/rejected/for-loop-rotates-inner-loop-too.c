s32 func_80037B00(u8 *arg0) {
    s32 i;
    s32 count;
    s32 mismatch;
    s8 *ent;
    s8 *p;
    s8 *q;
    s8 *end;
    s32 c;
    s32 d;

    count = D_800A38C8;
    ent = (s8 *)&D_80102810;
    for (i = 0; i < count; i++) {
        mismatch = 0;
        p = ent;
        q = (s8 *)arg0;
        end = ent + 0x15;
        do {
            c = (u8)*q;
            if (c == 0) {
                break;
            }
            d = (u8)*p;
            if (c != d) {
                mismatch = 1;
                break;
            }
            p += 1;
            q += 1;
        } while ((s32)p < (s32)end);
        if (mismatch == 0) {
            return 1;
        }
        ent += 0x28;
    }
    return 0;
}
