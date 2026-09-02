/* REJECTED: cast to MATRIX with gte.h not included in ings.c. cc1 (-w) silently accepts the
 * undeclared type name and DROPS the whole block-copy statement: sandbox 36, build_insns 45. */
s32 func_80017D84(u8 *a0) {
    u8 *p;
    s32 i;
    s32 c;

    p = g_file_data_buf;
    for (i = 0; i < 8; i++) {
        if (*(s32 *)p == 0) break;
        p += 0x34;
    }
    if (i == 8) return -1;
    if (D_800A30E8 < i) D_800A30E8 = i;
    *(u16 *)(p + 4) = *(u16 *)a0;
    *(s32 *)p = *(s32 *)(a0 + 4);
    *(MATRIX *)(p + 0x14) = **(MATRIX **)(a0 + 0xC);
    *(s32 *)(p + 8) = *(s16 *)(a0 + 2);
    *(s16 *)(p + 6) = 0;
    c = *(s32 *)(a0 + 0x10);
    *(s32 *)(p + 0xC) = c;
    *(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6);
    func_80017A44(a0, p);
    return i;
}
