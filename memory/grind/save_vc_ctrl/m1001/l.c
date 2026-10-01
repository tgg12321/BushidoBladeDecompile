typedef struct {
    u8 pad0[0xC];
    s32 ptr;
    u8 pad10[0x58];
} SvcRec;
void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    SvcRec *rec = (SvcRec *)recs;
    s32 i = n - 1;
    if (i != -1) {
        do {
            if (rec->ptr != 0) {
                rec->ptr += delta;
            }
            rec++;
        } while (--i != -1);
    }
}
