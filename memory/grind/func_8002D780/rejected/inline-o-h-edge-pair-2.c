s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        /* gte_ApplyRotMatrix(vin, vout) -- gtemac.h 4.3 :354-357 = inline_o.h 4.3 gte_ldv0 :16-20,
         * gte_rtv0 :426-430 (post-DMPSX word 0x4A486012 for the placeholder 0x0000013f),
         * gte_stlvnl :904-909 */
        __asm__ volatile ("move  $12,%0": :"r"((s32 *)(obj + 0xF8)):"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $0,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
        __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("move  $12,%0": :"r"((s32 *)(obj + 0x100)):"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $25,($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
        __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        if (y < -threshold || threshold < y) return 0;
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
        s32 kc = z0 * cx - x0 * cz;
        s32 kp = z0 * px - x0 * pz;

        if ((kc ^ kp) >= 0) {
            kc = z2 * cx - x2 * cz;
            kp = z2 * px - x2 * pz;
            if ((kc ^ kp) >= 0) {
                s32 ex = x2 - x0;
                s32 ez = z2 - z0;
                kc = ez * (cx - x0) - ex * (cz - z0);
                kp = ez * (px - x0) - ex * (pz - z0);
                if ((kc ^ kp) >= 0)
                    return 1;
            }
        }
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        s32 sp_var;
        s32 dist = r_sq - y * y;
        s32 sqrt_val;
        s32 *p118;
        s32 *p124;
        s32 *p10C;

        if ((u32)dist < 0x400) {
            sqrt_val = (u32)*((&g_sqrt_table_u8) + dist) >> 3;
        } else {
            s32 m = dist;
            s32 lzcr = 0;
            if (dist >= 0) {
                /* FAKE: same-value re-store of the local `m`, mechanism: cse.c
                 * invalidate_skipped_block - cse_end_of_basic_block follows the `dist < 0`
                 * skip over this arm (skip_blocks) and only a SET of `m` inside the skipped
                 * arm invalidates the m == dist equivalence made by the copy above, so the
                 * `(u32)m >> shift` read below keeps reading the $a0 copy instead of being
                 * canonicalised to dist ($s1); without it the srlv reads $s1 (drop-1 = 4/202),
                 * lever-exhaustion: memory/grind/func_8002D780/hypotheses.md s1-s5 (14 copy
                 * spellings) and s23 (do-while(0) wraps, copy placement, arm re-stores of the
                 * shared variable: all >= 1/202 or worse). */
                m = dist;
                /* gte_Lzc(m, &sp_var) -- gtemac.h 4.3 :174-178 = inline_o.h 4.3 gte_ldlzc :207-210,
                 * gte_nop :1095-1097 (x2), gte_stlzc :1074-1077 */
                __asm__ volatile ("move  $12,%0": :"r"(m):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("move  $12,%0": :"r"(&sp_var):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                sqrt_val = (u32)(*((&g_sqrt_table_u8) + ((u32)m >> shift)) << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = (s32 *)(obj + 0x118);
        p124 = (s32 *)(obj + 0x124);
        p118[0] = *(s32 *)(obj + 0xA8) - *(s32 *)(obj + 0x100);
        p118[1] = *(s32 *)(obj + 0xAC) - *(s32 *)(obj + 0x104);
        p124[0] = *(s32 *)(obj + 0xB8) - *(s32 *)(obj + 0x100);
        p124[1] = *(s32 *)(obj + 0xBC) - *(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p118, p124) != 0) return 1;

        p10C = (s32 *)(obj + 0x10C);
        p10C[0] = -*(s32 *)(obj + 0x100);
        p10C[1] = -*(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p10C, p118) != 0) return 1;

        if (func_8002D518(sqrt_val, dist, p10C, p124) != 0) return 1;
        return 0;
    }
}
