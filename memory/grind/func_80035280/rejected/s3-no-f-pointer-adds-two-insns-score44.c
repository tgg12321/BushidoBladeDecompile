/* REJECTED (s3): dropping the `f = &D_80106A73;` local and spelling
 * `src = (u8 *)&D_80106A73 - 3;` directly costs two instructions --
 * sandbox 44 / build 111 vs 39 / 109 for the same body with `f` kept.
 * The target keeps both registers ($a1 = &D_80106A73 for the lbu, $a2 = $a1-3
 * for the loop-1 walker), so the two-pointer spelling is the right one. */
void func_80035280(void) {
    s32 *p;
    u8 *src;
    u8 *base;
    s32 i;
    s32 v;

    p = func_80077D00();
    src = (u8 *)&D_80106A73 - 3;
    v = p[8];
    v = (v & ~1) | (src[3] & 1);
    p[8] = v;
    v = (v & ~2) | (src[3] & 2);
    p[8] = v;
    v = (v & ~4) | (src[3] & 4);
    p[8] = v;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[0x17 + i] = *src;
        ((u8 *)p)[0x1D + i] = *src;
        src++;
    }
    base = (u8 *)&D_80106A58;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[i * 4 + 0x21] = *(s32 *)(base + i * 8 + 4) / 1800;
        ((u8 *)p)[i * 4 + 0x22] = (*(s32 *)(base + i * 8 + 4) / 30) % 60;
        ((u8 *)p)[i * 4 + 0x23] = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;
        ((u8 *)p)[i * 4 + 0x24] = base[i * 8];
    }
}
