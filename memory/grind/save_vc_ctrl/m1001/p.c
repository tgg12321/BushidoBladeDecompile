typedef struct {
    u8 pad0[0xC];
    s32 ptr;
    u8 pad10[0x58];
} SvcRec;
void save_vc_ctrl(s32 delta, s16 *recs, s32 n) {
    s32 i;
    for (i = n - 1; i != -1; i--) {
        if (((SvcRec *)recs)[0].ptr != 0) {
            ((SvcRec *)recs)[0].ptr += delta;
        }
        recs = (s16 *)((SvcRec *)recs + 1);
    }
}
