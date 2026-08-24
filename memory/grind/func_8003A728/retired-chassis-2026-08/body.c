void func_8003A728(s32 a0) {
    register s32 buf8 asm("s0");
    register s32 lower asm("s2");
    s32 packed;
    s32 hash;
    s32 hi16;
    s32 vsync;
    s32 v0;
    s32 v1;

    if (D_800A320C == 0) goto end_zero;

    buf8 = *(volatile s32 *)(a0 + 8);
    __asm__ volatile("" ::: "memory");
    vsync = D_800A38A0;

    packed = (vsync << 31)
           | (D_800A3730 << 30)
           | ((D_800A3870 & 3) << 28);
    lower = buf8 & 0xFFFF;
    packed |= ((s32)*(s16 *)a0 << 16);
    packed |= lower;

    hi16 = (s32)D_800A37C4 << 16;

    D_800A3698 = packed;
    hash = hi16 | (((packed ^ (packed >> 16)) ^ (hi16 >> 16)) & 0xFFFF);
    D_800A369C = hash;

    if (D_800A3916 == 0) goto path_pad;

    if (vsync == 0) {
        func_8003A574();
        goto post_8F4;
    }

    if (((FuncBufType)func_8003A450)(&D_800A3698) == 0) goto err_no_check;
    D_800A3908 += func_8003A6FC(lower);
    func_8003A574();
    goto post_8F4;

path_pad:
    if (func_8003A5A0() == 0) goto err_no_check;
    if (D_800A38A0 != 1) goto retry_a450;
    if ((D_800A36C0 & 0x40000000) != 0) goto err_with_check;
    if ((D_800A36D0 & 0x40000000) != 0) goto err_with_check;

retry_a450:
    if (((FuncBufType)func_8003A450)(&D_800A3698) != 0) goto continue_a450;
err_no_check:
    func_8003A3F0();
    return;
continue_a450:
    D_800A3908 += func_8003A6FC(buf8 & 0xFFFF);
    func_8003A574();
    if (D_800A38A0 != 0) goto post_8F4;
    if (D_800A3730 != 0) goto err_with_check;
    if ((D_800A36C0 & 0x40000000) == 0) goto post_8F4;

err_with_check:
    func_8003A39C();
    return;

post_8F4:
    if (D_800A3916 != 0) goto skip_buf_update;

    {
        extern u16 _D_36C0_u16 asm("D_800A36C0");
        extern u16 _D_36D0_u16 asm("D_800A36D0");
        extern u16 _D_3698_u16 asm("D_800A3698");
        D_800A38FC += func_8003A6FC(_D_36C0_u16);
        {
            register u32 c0lo asm("v1") = _D_36C0_u16;
            s32 t;
            if (D_800A38A0 == 0) {
                *(s32 *)(a0 + 8) = (c0lo << 16) | _D_3698_u16;
                t = D_800A36C2;
                v0 = t & 0xF;
            } else {
                *(s32 *)(a0 + 8) = (_D_36D0_u16 << 16) | c0lo;
                t = D_800A36C2;
                *(s16 *)a0 = t & 0xF;
                t = D_800A36D2;
                v0 = t & 0xF;
            }
        }
    }
    *(s16 *)(a0 + 2) = v0;

    if (D_800A38A0 == 0) {
        v1 = (D_800A36C0 >> 28) & 3;
        if (v1 != 2) goto skip_buf_update;
        v0 = D_800A3870;
    } else {
        v1 = (D_800A36C0 >> 28) & 3;
        if (v1 != 2) goto skip_buf_update;
        v0 = (D_800A36D0 >> 28) & 3;
    }
    if (v0 != v1) goto skip_buf_update;
    D_800A3870 = 0;

skip_buf_update:
    D_800A3916 = 0;
    D_800A36D0 = D_800A3698;
    D_800A36D4 = D_800A369C;
    return;

end_zero:
    D_800A3870 = 0;
}
