/* REJECTED (s3) as the FLOOR, kept as the un-split fallback body: one reused
 * `v` accumulator across the three flag arms. The pseudo is set four times, so
 * reg_n_deaths == 4, local-alloc.c:472 refuses it, and the accumulator is
 * allocated by global-alloc AFTER local-alloc has given $v0/$v1/$a0 to the
 * per-arm temps -- producing the 4-cycle seat permutation (addr $a0, acc $a1,
 * mask $v1, byte $v0) against the target's (addr $a1, acc $v0, mask $a0,
 * byte $v1). Sandbox 39 / build 109. Superseded by the three-way split at 16.
 * Also measured at 39 on this chassis: declaring `v` first, and hoisting
 * `v = p[8];` above the pointer setup -- declaration/statement order does not
 * move the seats, only the death count does. */
void func_80035280(void) {
    s32 *p;
    u8 *f;
    u8 *src;
    u8 *base;
    s32 i;
    s32 v;

    p = func_80077D00();
    f = &D_80106A73;
    src = f - 3;
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
