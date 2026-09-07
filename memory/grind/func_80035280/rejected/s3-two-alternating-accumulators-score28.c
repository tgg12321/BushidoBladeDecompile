/* REJECTED (s3): splitting the flag accumulator into TWO alternating locals
 * (fA/fB) leaves each pseudo with reg_n_deaths == 2, so local-alloc.c:472
 * still refuses them (`reg_n_deaths[i] == 1` is the eligibility test) and they
 * still fall through to global-alloc. Sandbox 28 / build 109 -- better than
 * the single accumulator's 39 but 10 points worse than the three-way split's
 * 18, because only part of the flag block lands on the target's seats.
 * The split must be one local per merged bit. */
void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    s32 i;
    s32 flags;
    s32 fA;
    s32 fB;

    p = func_80077D00();
    f = &D_80106A73;
    src = f - 3;
    flags = p[8];
    fA = (flags & ~1) | (src[3] & 1);
    p[8] = fA;
    fB = (fA & ~2) | (src[3] & 2);
    p[8] = fB;
    fA = (fB & ~4) | (src[3] & 4);
    p[8] = fA;
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
