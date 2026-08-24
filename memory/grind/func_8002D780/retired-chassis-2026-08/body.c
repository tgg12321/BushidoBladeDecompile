s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        {
            register s32 t4 asm("t4");
            t4 = (s32)(obj + 0xF8);
            __asm__ volatile("lwc2 $0, 0($12)" : : "r"(t4));
            __asm__ volatile("lwc2 $1, 4($12)" : : "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            __asm__ volatile(".word 0x4A486012");
            t4 = (s32)(obj + 0x100);
            __asm__ volatile("swc2 $25, 0($12)" : : "r"(t4));
            __asm__ volatile(".word 0xE99A0004" : : "r"(t4));
            __asm__ volatile(".word 0xE99B0008" : : "r"(t4));
        }
    }

    {
        s32 z_val = *(s32 *)(obj + 0x108);
        if (z_val < -threshold || threshold < z_val) {
            return 0;
        }
    }

    {
        s32 x0 = *(s32 *)(obj + 0xA8);
        s32 x2 = *(s32 *)(obj + 0xB8);
        s32 z0 = *(s32 *)(obj + 0xAC);
        s32 z2 = *(s32 *)(obj + 0xBC);
        s32 cx = (x0 + x2) / 3;
        s32 cz = (z0 + z2) / 3;
        s32 px = *(s32 *)(obj + 0x100);
        s32 pz = *(s32 *)(obj + 0x104);

        {
            s32 cross_c = z0 * cx - x0 * cz;
            s32 cross_p = z0 * px - x0 * pz;
            if ((cross_c ^ cross_p) < 0) goto edge_check;
        }

        {
            s32 cross_c = z2 * cx - x2 * cz;
            s32 cross_p = z2 * px - x2 * pz;
            if ((cross_c ^ cross_p) < 0) goto edge_check;
        }

        {
            s32 dz = z2 - z0;
            s32 dx = x2 - x0;
            s32 cross_c = dz * (cx - x0) - dx * (cz - z0);
            s32 cross_p = dz * (px - x0) - dx * (pz - z0);
            if ((cross_c ^ cross_p) >= 0) return 1;
        }
    }

edge_check:
    {
        s32 z_val = *(s32 *)(obj + 0x108);
        s32 sp_var;
        s32 dist = r_sq - z_val * z_val;
        s32 sqrt_val;
        s32 *p118;
        s32 *p124;

        if ((u32)dist < 0x400) {
            sqrt_val = ((u32)*(&D_8008D118 + dist)) >> 3;
        } else {
            s32 lzcr = 0;
            if (dist >= 0) {
                register s32 t4 asm("t4") = dist;
                __asm__ volatile(".word 0x488CF000" : : "r"(t4));
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                t4 = (s32)&sp_var;
                __asm__ volatile(".word 0xE99F0000" : : "r"(t4));
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(&D_8008D118 + ((u32)dist >> shift));
                sqrt_val = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = (s32 *)(obj + 0x118);
        p124 = (s32 *)(obj + 0x124);

        p118[0] = *(s32 *)(obj + 0xA8) - *(s32 *)(obj + 0x100);
        p118[1] = *(s32 *)(obj + 0xAC) - *(s32 *)(obj + 0x104);
        p124[0] = *(s32 *)(obj + 0xB8) - *(s32 *)(obj + 0x100);
        p124[1] = *(s32 *)(obj + 0xBC) - *(s32 *)(obj + 0x104);

        if (func_8002D518(sqrt_val, dist, p118, p124) != 0) return 1;

        {
            s32 *p10C = (s32 *)(obj + 0x10C);
            p10C[0] = -*(s32 *)(obj + 0x100);
            p10C[1] = -*(s32 *)(obj + 0x104);

            if (func_8002D518(sqrt_val, dist, p10C, p118) != 0) return 1;

            return func_8002D518(sqrt_val, dist, p10C, p124) != 0;
        }
    }
}
