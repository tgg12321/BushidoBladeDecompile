s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    s32 *vp = (s32 *)(obj + 0xF8);
    s32 *rp = (s32 *)(obj + 0x100);
    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
    __asm__ volatile("lwc2 $0, 0(%0)" : : "r"(vp));
    __asm__ volatile("lwc2 $1, 4(%0)" : : "r"(vp));
    __asm__ volatile("nop\nnop\n.word 0x4A486012");
    __asm__ volatile("swc2 $25, 0(%0)" : : "r"(rp));
    __asm__ volatile("swc2 $26, 4(%0)" : : "r"(rp));
    __asm__ volatile("swc2 $27, 8(%0)" : : "r"(rp));

    {
        s32 x = *(s32 *)(obj + 0x100);
        s32 z;
        s32 a0_var;
        s32 sp_var;
        s32 min_y;
        s32 max_y;
        s32 y_low;
        s32 y;
        s32 neg_threshold = -threshold;

        if (x < neg_threshold || threshold < x) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        a0_var = x * x + z * z;
        if (r_sq < a0_var) return 0;
        a0_var = r_sq - a0_var;

        if ((u32)a0_var < 0x400) {
            a0_var = (u32)*(((u8 *)&D_8008D118) + a0_var) >> 3;
        } else {
            s32 lzcr = 0;
            if (a0_var >= 0) {
                {
                    s32 *lp = &sp_var;
                    __asm__ volatile("mtc2 %0, $30" : : "r"(a0_var));
                    __asm__ volatile("nop\nnop\nswc2 $31, 0(%0)" : : "r"(lp));
                }
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)a0_var >> shift));
                a0_var = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_y = 0;
        min_y = 0;
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else {
            max_y = y_low;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - a0_var) return 0;
        if (y + a0_var < min_y) return 0;
        return 1;
    }
}
