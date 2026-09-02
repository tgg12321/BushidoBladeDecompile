/* REJECTED: tail order (w8, h6, c, wC, w10). Correct registers (t0/a3) and block move, but the
 * scheduler emits move a0 / sh zero,6 before sw 8(a3) and lh 4(a3) before lw 16(t0): 10-11 tail
 * insns out of order. Only order (w8, c, h6, wC, w10) matches (raw-cc1 sweep of all 24 legal orders). */
typedef struct { s32 v[8]; } ObjBlock;
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
    *(ObjBlock *)(p + 0x14) = **(ObjBlock **)(a0 + 0xC);
    *(s32 *)(p + 8) = *(s16 *)(a0 + 2);
    *(s16 *)(p + 6) = 0;
    c = *(s32 *)(a0 + 0x10);
    *(s32 *)(p + 0xC) = c;
    *(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6);
    func_80017A44(a0, p);
    return i;
}
