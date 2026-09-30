void func_8001F2E4(u8 *obj, u8 *a, u8 *b) {
    s32 lzc_out;
    s32 lzc_out2;
    s32 tgt_z;
    s32 temp2;
    /* FAKE: constant-holder (named-local-fake-exception.md) -- tgt_x is 0 on
     * every path; cse works per extended basic block, so the three arm
     * writes reach both func_8002F770 calls through the join unfolded, and
     * global.c seats the pseudo, live across the first call, in $s2 as the
     * target does (`addu $s2,$zero,$zero` in the arms, `addu $a3,$s2,$zero`
     * before each call). The literal 0, one `= 0` initializer, or one write
     * after the join / before the calls score 43-46; see
     * memory/grind/func_8001F2E4/r11/proof.md. */
    s32 tgt_x;
    s32 dx;
    s32 dz;
    s32 d1e6;
    s32 d1e8;
    s16 t;

    if (*(s16 *)(obj + 0x26C) == 0) {
        func_80027334((s32 *)a);
        func_80027334((s32 *)b);
    }
    if (*(u16 *)(obj + 0x6A) == 0x15 || *(u16 *)(obj + 0x6A) == 0x25) {
        if (*(s16 *)(obj + 0xC) == 0x1F) {
            temp2 = 0x100;
            tgt_x = 0;
            tgt_z = 0;
        } else {
            s32 dist_sq;
            s32 dist;

            tgt_z = (*(s16 *)(obj + 0x1D8) - *(s16 *)(obj + 0x1CA)) & 0xFFF;
            if (tgt_z >= 0x800) {
                tgt_z -= 0x1000;
            }
            if (tgt_z < -0x1FF) {
                tgt_z = -0x1FF;
            } else if (tgt_z >= 0x200) {
                tgt_z = 0x1FF;
            }
            dx = *(s32 *)(*(u8 **)obj + 0x180) - *(s32 *)(obj + 0x180);
            dz = *(s32 *)(*(u8 **)obj + 0x188) - *(s32 *)(obj + 0x188);
            dist_sq = dx * dx + dz * dz;
            if ((u32)dist_sq < 0x400) {
                dist = (u32)*(&g_sqrt_table_u8 + dist_sq) >> 3;
            } else {
                s32 lzcr = 0;
                if (dist_sq >= 0) {
                    /* gte_Lzc(dist_sq, &lzc_out): gtemac.h 4.3 :174-178 = inline_o.h 4.3
                     * gte_ldlzc :207-210, gte_nop :1095-1097 (x2), gte_stlzc :1074-1077;
                     * LZCR slot sp+0x10 in the target. */
                    __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                    __asm__ volatile ("move  $12,%0": :"r"(&lzc_out):"$12","$13","$14","$15","memory");
                    __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                    lzcr = lzc_out;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tbl = *(&g_sqrt_table_u8 + ((u32)dist_sq >> shift));
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            temp2 = 0x400 - ratan2(dist, *(s32 *)(*(u8 **)obj + 0x184) - *(s32 *)(obj + 0x184));
            if (temp2 < -0xFF) {
                temp2 = -0xFF;
            } else if (temp2 >= 0x100) {
                temp2 = 0xFF;
            }
            tgt_x = 0;
        }
    } else {
        tgt_x = 0;
        temp2 = 0;
        tgt_z = 0;
    }

    d1e6 = (tgt_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;
    if (d1e6 >= 0x800) {
        d1e6 -= 0x1000;
    }
    *(s16 *)(obj + 0x1E6) = *(s16 *)(obj + 0x1E6) + d1e6 / 8;
    d1e8 = (temp2 - *(s16 *)(obj + 0x1E8)) & 0xFFF;
    if (d1e8 >= 0x800) {
        d1e8 -= 0x1000;
    }
    *(s16 *)(obj + 0x1E8) = *(s16 *)(obj + 0x1E8) + d1e8 / 8;
    func_8002F770((s16 *)(a + 0x36), *(s16 *)(obj + 0x1E6), *(s16 *)(obj + 0x1E8), tgt_x);
    func_8002F770((s16 *)(b + 0x36), *(s16 *)(obj + 0x1E6), *(s16 *)(obj + 0x1E8), tgt_x);

    t = *(s16 *)(obj + 0xC);
    if ((t == 0x1D || t == 0xE) && *(s16 *)(obj + 0x8C) != 0) {
        s32 twist;

        twist = (ratan2(D_800A387C, *(s32 *)(*(u8 **)obj + 0xF8) - *(s32 *)(obj + 0xF8)) - 0x400) & 0xFFF;
        if (twist >= 0x800) {
            twist -= 0x1000;
        }
        if (twist >= 0x200) {
            twist = 0x1FF;
        } else if (twist < -0x1FF) {
            twist = -0x1FF;
        }
        *(s16 *)(obj + 0x1EA) = twist;
        *(u16 *)(a + 0x7E) += twist;
        *(u16 *)(b + 0x7E) += twist;
    }

    if ((u32)(*(u16 *)(obj + 0xE) - 6) < 2U && *(u16 *)(obj + 0x6A) == 2) {
        s32 dist_sq;
        s32 dist;
        s32 delta;

        if (*(s32 *)(obj + 0x268) == 0) {
            *(s32 *)(obj + 0x25C) = *(s32 *)(obj + 0xF4);
            *(s32 *)(obj + 0x260) = *(s32 *)(obj + 0xF8);
            *(s32 *)(obj + 0x264) = *(s32 *)(obj + 0xFC);
        }
        dx = *(s32 *)(*(u8 **)obj + 0xF4) - *(s32 *)(obj + 0x25C);
        dz = *(s32 *)(*(u8 **)obj + 0xFC) - *(s32 *)(obj + 0x264);
        dist_sq = dx * dx + dz * dz;
        if ((u32)dist_sq < 0x400) {
            dist = (u32)*(&g_sqrt_table_u8 + dist_sq) >> 3;
        } else {
            s32 lzcr = 0;
            if (dist_sq >= 0) {
                /* gte_Lzc(dist_sq, &lzc_out2): gtemac.h 4.3 :174-178 = inline_o.h 4.3
                 * gte_ldlzc :207-210, gte_nop :1095-1097 (x2), gte_stlzc :1074-1077;
                 * LZCR slot sp+0x14 in the target. */
                __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("move  $12,%0": :"r"(&lzc_out2):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                lzcr = lzc_out2;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(&g_sqrt_table_u8 + ((u32)dist_sq >> shift));
                dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }
        temp2 = (ratan2(dist, *(s32 *)(*(u8 **)obj + 0xF8) - *(s32 *)(obj + 0x260)) - 0x400) & 0xFFF;
        if (temp2 >= 0x800) {
            temp2 -= 0x1000;
        }
        if (temp2 >= 0x200) {
            temp2 = 0x1FF;
        } else if (temp2 < -0x1FF) {
            temp2 = -0x1FF;
        }
        delta = (temp2 - *(s16 *)(obj + 0x1EA)) & 0xFFF;
        if (delta >= 0x800) {
            delta -= 0x1000;
        }
        *(s16 *)(obj + 0x1EA) = *(s16 *)(obj + 0x1EA) + delta / 8;
        *(u16 *)(a + 0x72) += *(s16 *)(obj + 0x1EA);
        *(u16 *)(b + 0x72) += *(s16 *)(obj + 0x1EA);
    }

    if (*(s16 *)(obj + 0x26E) != 0 && *(s16 *)(obj + 0x96) == 0) {
        s32 jit1;
        s32 jit2;

        jit1 = (rng_Next() & 0x3F) - 0x20;
        *(u16 *)(a + 0xC) += jit1;
        *(u16 *)(b + 0xC) += jit1;
        *(u16 *)(a + 0x14) -= jit1;
        *(u16 *)(b + 0x14) -= jit1;
        jit2 = (rng_Next() & 0x3F) - 0x20;
        *(u16 *)(a + 0x1E) += jit2;
        *(u16 *)(b + 0x1E) += jit2;
        *(u16 *)(a + 0x26) -= jit2;
        *(u16 *)(b + 0x26) -= jit2;
    }
}
