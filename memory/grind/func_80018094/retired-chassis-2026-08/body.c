void func_80018094(s32 *arg0, s32 *arg1) {
    register s32 *p0 asm("s0") = arg0;
    register s32 *p1 asm("s1") = arg1;
    s32 sp_area[4];

    {
        register s32 mat_ptr asm("a2") = p0[1];
        /* Block 1 (split per-instruction). Original cheat was 12 .word insns
         * loading 5 packed rotation-matrix words into GTE coef regs $0..$4. */
        __asm__ volatile (".word 0x00000000" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x00C06021" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x8D8D0000" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x8D8E0004" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x48CD0000" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x48CE0800" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x8D8D0008" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x8D8E000C" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x8D8F0010" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x48CD1000" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x48CE1800" :: "r"(mat_ptr));
        __asm__ volatile (".word 0x48CF2000" :: "r"(mat_ptr));
    }

    /* Block 2 (split per-instruction). Original cheat was 9 .word insns
     * loading translation vector + reloading p0[1]. */
    __asm__ volatile (".word 0x8E060004");
    __asm__ volatile (".word 0x00000000");
    __asm__ volatile (".word 0x00C06021");
    __asm__ volatile (".word 0x8D8D0014");
    __asm__ volatile (".word 0x8D8E0018");
    __asm__ volatile (".word 0x48CD2800");
    __asm__ volatile (".word 0x8D8F001C");
    __asm__ volatile (".word 0x48CE3000");
    __asm__ volatile (".word 0x48CF3800");

    func_80017FA0(p0);

    {
        s32 *mat;
        s32 dx, dy, dz;
        s32 sum_sq;
        s32 scale;

        mat = (s32 *)p0[1];
        dx = mat[5] - p1[10];
        *(volatile s32 *)0x1F800024 = dx;

        mat = (s32 *)p0[1];
        dy = mat[6] - p1[11];
        *(volatile s32 *)0x1F800028 = dy;

        mat = (s32 *)p0[1];
        dz = mat[7] - p1[12];
        *(volatile s32 *)0x1F80002C = dz;

        sum_sq = (dx * dx) + (dy * dy) + (dz * dz);

        scale = 0x100;
        if (sum_sq <= 250000) {
            if (sum_sq < 0) {
                scale = 0;
            } else {
                s32 log2_val;
                if (sum_sq < 0x400) {
                    log2_val = (u32)(*(&D_8008D118 + sum_sq)) >> 3;
                } else {
                    s32 shift_a, shift_b;
                    register s32 t4_v asm("t4");
                    asm volatile("" : "=r"(t4_v));
                    t4_v = sum_sq;
                    {
                        s32 *new_var = &sp_area[0];
                        asm volatile(".word 0x488CF000" : : "r"(t4_v));
                        asm volatile("nop");
                        asm volatile("nop");
                        {
                            s32 addr_v0 = (s32)new_var;
                            t4_v = addr_v0;
                            asm volatile(".word 0xE99F0000" : : "r"(t4_v));
                        }
                    }
                    {
                        s32 lw_v1 = sp_area[0];
                        s32 li_v0 = -2;
                        li_v0 = lw_v1 & li_v0;
                        shift_a = 0x16 - li_v0;
                    }
                    shift_b = shift_a >> 1;
                    log2_val = (((u8)(*(&D_8008D118 + (sum_sq >> shift_a)))) << 16) >> (0x13 - shift_b);
                }
                scale = ((log2_val << 6) / 500) + 0xC0;
            }
        }

        {
            s32 v24 = *(volatile s32 *)0x1F800024;
            s32 v28 = *(volatile s32 *)0x1F800028;
            s32 v2c = *(volatile s32 *)0x1F80002C;
            *(volatile s32 *)0x1F800024 = (v24 * scale) >> 1;
            *(volatile s32 *)0x1F800028 = (v28 * scale) >> 1;
            *(volatile s32 *)0x1F80002C = (v2c * scale) >> 1;
        }
    }

    {
        s32 *src = (s32 *)p0[1];
        *(Copy16 *)((u8 *)p1 + 0x14) = *(Copy16 *)src;
        *(Copy16 *)((u8 *)p1 + 0x24) = *(Copy16 *)((u8 *)src + 0x10);
    }

    func_80018300(p1);
}
