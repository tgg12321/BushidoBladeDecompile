/* func_8003F6D8 — session 1 recon — sandbox --disable all score 0 (2026-09-08).
 * Outer loop: offset `off = i*0xD0 + 8` computed from i INSIDE the loop; loop.c reduces
 * it to a DEST_REG giv (init after the entry test, +0xD0 before i++), and the address
 * arg0+off+0x1C is left as a runtime add. A hoisted user biv or a pointer giv both miss. */
typedef struct {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 *objs[5];
    /* 0x18 */ s16 pairs[3][16];
    /* 0x78 */ s32 unk78[3];
    /* 0x84 */ s32 quads[3][4];
} Func8003F6D8Inner; /* size 0xB4; sits at +0x1C of each 0xD0-byte record, records start at arg0+8 */

void func_8003F6D8(s16 *arg0) {
    s32 i;
    s32 j;

    for (i = 0; i < arg0[0]; i++) {
        s32 off = i * 0xD0 + 8;
        Func8003F6D8Inner *in = (Func8003F6D8Inner *)((u8 *)arg0 + off + 0x1C);
        for (j = 0; j < in->count; j++) {
            s32 *obj = in->objs[j] + 6;
            func_80052A20(obj, in->quads[j], in->pairs[j]);
            func_80052A20(obj, in->quads[j] + 2, in->pairs[j] + 8);
        }
    }
}

