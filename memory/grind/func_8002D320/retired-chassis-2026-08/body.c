s32 func_8002D320(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        __asm__ volatile("" ::: "memory");
        {
            register s32 t4 asm("t4");
            s32 ptr_tmp;
            ptr_tmp = (s32)(obj + 0xF8);
            __asm__ volatile("addu %0, %1, $zero" : "=r"(t4) : "r"(ptr_tmp));
            __asm__ volatile("lwc2 $0, 0($12)" : : "r"(t4));
            __asm__ volatile("lwc2 $1, 4($12)" : : "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            __asm__ volatile(".word 0x4A486012");
            ptr_tmp = (s32)(obj + 0x100);
            __asm__ volatile("addu %0, %1, $zero" : "=r"(t4) : "r"(ptr_tmp));
            __asm__ volatile("swc2 $25, 0($12)" : : "r"(t4));
            __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
            __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
        }
    }

    {
        s32 x = *(s32 *)(obj + 0x100);
        if (x < -threshold || threshold < x) return 0;
    }
    {
        s32 z = *(s32 *)(obj + 0x104);
        if (z < -threshold || threshold < z) return 0;
    }

    {
        s32 x = *(s32 *)(obj + 0x100);
        s32 z = *(s32 *)(obj + 0x104);
        register s32 dist_sq asm("a0") = x * x + z * z;
        s32 disc;
        s32 sqrt_val;
        s32 sp_var;
        register s32 min_y asm("a2");
        register s32 max_y asm("a3");
        s32 y_low;
        s32 y_high;
        s32 y;

        if (r_sq < dist_sq) return 0;
        disc = r_sq - dist_sq;

        if ((u32)disc < 0x400) {
            sqrt_val = ((u32)*(&D_8008D118 + disc)) >> 3;
        } else {
            s32 lzcr = 0;
            if (disc >= 0) {
                register s32 t4 asm("t4") = disc;
                __asm__ volatile(".word 0x488CF000" : : "r"(t4));
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                t4 = (s32)&sp_var;
                __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(&D_8008D118 + ((u32)disc >> shift));
                sqrt_val = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        {
            s32 result = 0;
            max_y = 0;
            min_y = 0;
            y_low = *(s32 *)(obj + 0xB0);
            if (y_low < 0) {
                min_y = y_low;
            } else if (y_low > 0) {
                max_y = y_low;
            }
            y_high = *(s32 *)(obj + 0xC0);
            if (y_high < min_y) {
                min_y = y_high;
            } else if (max_y < y_high) {
                max_y = y_high;
            }
            y = *(s32 *)(obj + 0x108);
            if (max_y >= y - sqrt_val) {
                if (y + sqrt_val >= min_y) {
                    result = 1;
                }
            }
            return result;
        }
    }
}
