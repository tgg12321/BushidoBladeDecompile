s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        /* gte_ApplyRotMatrix(obj + 0xF8, obj + 0x100) -- gtemac.h 4.3 :354-357 = inline_o.h 4.3 gte_ldv0 :16-20,
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
        s32 cross_point1 = z0 * px - x0 * pz;
        s32 cross_center1 = z0 * cx - x0 * cz;

        if ((cross_center1 ^ cross_point1) >= 0) {
            s32 cross_point2;
            s32 cross_center2;
            cross_center2 = z2 * cx - x2 * cz;
            cross_point2 = z2 * px - x2 * pz;
            if ((cross_center2 ^ cross_point2) >= 0) {
                s32 ax = cx - x0;
                s32 dx;
                s32 az;
                s32 bz;
                s32 cross_point3;
                s32 cross_center3;
                /* FAKE: the third edge test's edge difference z2 - z0 is staged through the
                 * `flag` parameter (its own job, the mode test at entry, is finished: nothing
                 * reads `flag` after `if (flag == 0)`, and this value is consumed by the two
                 * products below and never needed again), instead of through a fresh
                 * block-local, mechanism: local-alloc.c local_alloc admission (local-alloc.c:472
                 * REG_BASIC_BLOCK >= 0 && REG_N_DEATHS == 1) - a pseudo referenced in two basic
                 * blocks (the entry test and this block) is left to global.c, so block 7's
                 * local-alloc quantity table seats dx first in $v1 and global_alloc, reaching the
                 * parameter's pseudo third in allocno order, seats it in $a0, the lowest free
                 * register at that turn (memory/grind/func_8002D780/dumps-0930/greg-flag-seat.txt:
                 * "72 in 4"; the entry copy from $a0 is folded away by combine AFTER flow has fixed
                 * the pseudo's REG_BASIC_BLOCK as global) (the target's seats; a block-local dz
                 * ties dx in qty_compare_1 and takes $v1 itself),
                 * lever-exhaustion: memory/grind/func_8002D780/hypotheses.md s14-s23
                 * (declaration order/scope, statement order, staging, hoisting, sign flips,
                 * 2,080 + 816 + 528 enumerated block-local spellings, all >= 2/202; a fresh
                 * function-scope scratch shared with the sqrt block reaches 0 but was
                 * Judge-FAILed 2026-09-15 23:16 as an invented multi-write carrier; the
                 * `threshold` parameter as carrier scores 28; s23 third run; re-measured on the
                 * verbatim inline_o.h chassis 2026-09-30: a fresh block-local dz = 9/202, the best
                 * dz spellings 2/202, memory/grind/func_8002D780/evidence.md 2026-09-30). */
                flag = z2 - z0;
                dx = x2 - x0;
                az = cz - z0;
                cross_center3 = (flag * ax) - (dx * az);
                bz = pz - z0;
                cross_point3 = (flag * (px - x0)) - (dx * bz);
                if ((cross_center3 ^ cross_point3) >= 0)
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
            sqrt_val = (u32)(&g_sqrt_table_u8)[dist] >> 3;
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
                 * shared variable: all >= 1/202 or worse; re-measured 2026-09-30 on the verbatim
                 * inline_o.h chassis: dropped = 4/202, spelled `m = m;` = 4/202, folded away). */
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
                sqrt_val = (u32)((&g_sqrt_table_u8)[(u32)m >> shift] << 16) >> (0x13 - ((u32)shift >> 1));
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
