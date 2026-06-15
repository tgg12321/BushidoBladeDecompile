/* func_8003D330 (code6cac_c2.c) — CLEAN pin-free body (NO register asm pins,
 * NO phantom `ori %0,%0,0xFFFF :: "r"(base)` coercion asm). GPU primitive
 * OT-link (addPrim) pattern. Honest full-build distance 13 vs HEAD's 4 pins
 * (code a1, mask_lo a2, mask_hi a3, p v1) + the bogus ori-asm.
 * Residual is a register-allocation tie: target puts the three constants in
 * $a1/$a2/$a3 (skipping $a0, which it uses for a temp); GCC packs them
 * differently (code->$a0, masks swapped a2<->a3) and reorders the two *ot reads.
 * 6 declaration/structure orderings flatlined at 13. See notes.md. */
void func_8003D330(void) {
    u32 code = 0xE100001F;
    u32 mask_lo = 0x00FFFFFF;
    u32 mask_hi = 0xFF000000;
    u8 *base = (u8 *)&D_800A3D30;
    u8 *p = base + (D_800A3218 << 3);
    u32 *ot;
    u32 tag;
    p[3] = 1;
    *(u32 *)(p + 4) = code;
    ot = (u32 *)D_800A374C;
    tag = *(u32 *)p;
    tag = (tag & mask_hi) | (*ot & mask_lo);
    *(u32 *)p = tag;
    *ot = (*ot & mask_hi) | ((u32)p & mask_lo);
}
