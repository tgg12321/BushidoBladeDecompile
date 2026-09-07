/* REJECTED (s3): initialising the loop-1 counter in its DECLARATION
 * (`s32 i = 0;` with `for (; i < 3; i++)`) puts the zeroing before the
 * `func_80077D00()` call, so the value has to survive the call and GCC emits
 * an extra instruction: sandbox 29 / build 110. The statement form placed
 * immediately AFTER the call measures 16 / 109. Placement, not form. */
void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    s32 i = 0;
    s32 flags;
    s32 flags0;
    s32 flags1;
    s32 flags2;

    p = func_80077D00();
    f = &D_80106A73;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
    for (; i < 3; i++) {
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
