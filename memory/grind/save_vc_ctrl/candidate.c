/* One 0x68-byte record of the three arrays func_80041430 rebases (obj+0x2C x 0x15, +0x8B4 x 0x14,
 * +0x10D4 x 0x14; text1a_pre_tu2.c): +0xC holds a pointer into the moved block, or 0. */
typedef struct VcCtrl {
    u8 unk0[0xC];
    s32 ptr;
    u8 unk10[0x58];
} VcCtrl;

/* Adds delta to every record's non-null pointer (the block it points into moved by delta). */
void save_vc_ctrl(s32 delta, VcCtrl *rec, s32 n) {
    s32 i;

    for (i = n - 1; i != -1; i--) {
        if (rec->ptr != 0) {
            rec->ptr += delta;
        }
        rec++;
    }
}
