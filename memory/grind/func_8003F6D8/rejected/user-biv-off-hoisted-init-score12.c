/* REJECTED — score 12: user biv off=8 before loop -> init before entry test, incr after i++ */
typedef struct {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 *objs[5];
    /* 0x18 */ s16 pairs[3][16];
    /* 0x78 */ s32 unk78[3];
    /* 0x84 */ s32 quads[3][4];
} Func8003F6D8Inner; /* size 0xB4 */

typedef struct {
    /* 0x00 */ u8 unk0[0x1C];
    /* 0x1C */ Func8003F6D8Inner inner;
} Func8003F6D8Rec; /* size 0xD0 */

void func_8003F6D8(s16 *arg0) {
    s32 i;
    s32 j;
    s32 off = 8;

    for (i = 0; i < arg0[0]; i++, off += 0xD0) {
        Func8003F6D8Inner *in = (Func8003F6D8Inner *)((u8 *)arg0 + off + 0x1C);
        for (j = 0; j < in->count; j++) {
            s32 *obj = in->objs[j] + 6;
            func_80052A20(obj, in->quads[j], in->pairs[j]);
            func_80052A20(obj, in->quads[j] + 2, in->pairs[j] + 8);
        }
    }
}

