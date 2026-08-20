s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
    __asm__ volatile("" ::: "memory");
    {
        register s32 t4 asm("t4");
        register s32 ptr_tmp asm("v0");
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

    {
        register s32 x asm("a1") = *(s32 *)(obj + 0x100);
        register s32 neg_threshold asm("t1") = -threshold;
        if (x < neg_threshold || threshold < x) return 0;
    }
    {
        s32 z = *(s32 *)(obj + 0x104);
        register s32 neg_threshold asm("t1") = -threshold;
        if (z < neg_threshold || threshold < z) return 0;
    }

    {
        register s32 x asm("a1") = *(s32 *)(obj + 0x100);
        s32 z = *(s32 *)(obj + 0x104);
        register s32 a0_var asm("a0");
        s32 sp_var;
        register s32 min_y asm("a2");
        register s32 max_y asm("a1");
        s32 y_low;
        s32 y;

        a0_var = x * x + z * z;
        if (r_sq < a0_var) return 0;
        a0_var = r_sq - a0_var;

        if ((u32)a0_var < 0x400) {
            register s32 tbl_val asm("v0") = *(&D_8008D118 + a0_var);
            a0_var = (u32)tbl_val >> 3;
        } else {
            s32 lzcr = 0;
            if (a0_var >= 0) {
                register s32 t4 asm("t4") = a0_var;
                __asm__ volatile(".word 0x488CF000" : : "r"(t4));
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                t4 = (s32)&sp_var;
                __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                register s32 shifted asm("v0") = (u32)a0_var >> shift;
                register s32 tbl asm("a0") = *(&D_8008D118 + shifted);
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
