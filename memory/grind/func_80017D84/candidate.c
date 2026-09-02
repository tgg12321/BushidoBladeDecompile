/* func_80017D84 — candidate, session 1 (recon), 2026-09-02.
 * sandbox --disable all = 0 (66/66 insns); verify-oracle ok:true with this body in src/ings.c.
 * Object-slot allocator: finds the first free 0x34-byte slot in g_file_data_buf (8 slots),
 * bumps the high-water byte D_800A30E8, copies the descriptor fields + a 32-byte block
 * (struct assignment = GCC 2.7.2 mips block move, 4 regs per batch), calls func_80017A44(desc, slot).
 * Load-bearing details: (1) the block copy MUST be a struct assignment (an undeclared type name
 * such as MATRIX — gte.h is NOT included in ings.c — is silently accepted by cc1 -w and the whole
 * statement is DROPPED); (2) tail statement order w8, c, h6, wC, w10 is the only one of the 24
 * legal orders that reproduces the target scheduling (see evidence E-s1-3).
 */
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
    c = *(s32 *)(a0 + 0x10);
    *(s16 *)(p + 6) = 0;
    *(s32 *)(p + 0xC) = c;
    *(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6);
    func_80017A44(a0, p);
    return i;
}
