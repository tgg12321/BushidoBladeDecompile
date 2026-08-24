void func_8003D39C(s32 x, s32 y, s32 ch, s32 color) {
    register s32 sx asm("t0") = x;
    register s32 sy asm("t1") = y;
    register s32 scolor asm("t2") = color;
    register u32 mask_lo asm("a3");
    register u32 mask_hi asm("a2");
    register u32 tag asm("a1");
    register s32 count asm("v1");
    register u8 *p asm("a0");
    register u32 cmd_tag asm("v1");
    u32 *ot;

    count = D_800A3358;
    if (count == 0x20) return;

    mask_lo = 0x00FFFFFF;
    {
        register u32 v0_t asm("v0") = count + 1;
        D_800A3358 = v0_t;
    }
    {
        register u32 v0_off asm("v0") = count << 4;
        register u8 *arr_ptr asm("v1") = (u8 *)&D_800A3930;
        v0_off += (u32)arr_ptr;
        p = (u8 *)((D_800A3218 << 9) + v0_off);
    }
    cmd_tag = 0x74000000;

    p[3] = 3;
    p[7] = 0x74;
    p[0xC] = ((ch & 7) << 3) - 0x40;
    p[0xD] = ((ch >> 5) << 3) - 0x20;
    *(u16 *)(p + 0xE) = ((ch << 3) & 0xC0) | 0x773F;
    tag = *(u32 *)p;
    {
        register u32 cmd_v0 asm("v0");
        cmd_v0 = (scolor >> 1) | cmd_tag;
        *(u32 *)(p + 4) = cmd_v0;
    }
    ot = (u32 *)D_800A374C;
    mask_hi = 0xFF000000;
    *(s16 *)(p + 8) = sx;
    *(s16 *)(p + 0xA) = sy;
    tag = (tag & mask_hi) | (*ot & mask_lo);
    *(u32 *)p = tag;
    *ot = (*ot & mask_hi) | ((u32)p & mask_lo);
}
