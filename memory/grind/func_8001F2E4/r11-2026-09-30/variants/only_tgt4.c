void func_8001F2E4(u8 *obj, u8 *a, u8 *b) {
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 tgt_z;
    s32 tgt_y;
    s32 tgt_x;
    s32 dx;
    s32 dz;
    s32 dist_sq;
    s32 dist;
    s32 ang;
    s32 twist_tgt;
    s16 t;

    if (*(s16 *)(obj + 0x26C) == 0) {
        func_80027334((s32 *)a);
        func_80027334((s32 *)b);
    }
    if (*(u16 *)(obj + 0x6A) == 0x15 || *(u16 *)(obj + 0x6A) == 0x25) {
        if (*(s16 *)(obj + 0xC) == 0x1F) {
            tgt_y = 0x100;
            tgt_x = 0;
            tgt_z = 0;
        } else {
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
                dist = (u32)*(((u8 *)&g_sqrt_table_u8) + dist_sq) >> 3;
            } else {
                s32 lzcr = 0;
                if (dist_sq >= 0) {
                    /* PsyQ libgte inline macro gte_Lzc(r1,r2) --- gtemac.h:174-178,
                     * which expands to gte_ldlzc(r1) (inline_c.h:228-231, `mtc2 %0,$30`),
                     * two gte_nop() (inline_c.h:1346-1347), then gte_stlzc(r2)
                     * (inline_c.h:1318-1322, `swc2 $31,0(%0)`).  DISCLOSURE OF THE
                     * ADDRESSING PREAMBLE: the two `addu $t4, ..., $zero` moves and the
                     * `addiu $v0,$sp,0x10` are NOT macro text -- they are the cop2
                     * operand-addressing preamble of the 2026-08-17 owner cluster
                     * (.claude/rules/cop2-addressing-preamble-cluster.md, census row for
                     * this function, 2 idiom sites).  Instructions, operands and
                     * clobbers are identical to the authorized func_8002E838 island
                     * (src/code6cac_b.c; that copy also carries inline comments).
                     * `"=m"(sp_tmp)` names the frame slot, `"r"(dist_sq)` the input. */
                    __asm__ volatile(
                        "addu   $t4, %1, $zero\n"
                        "mtc2   $t4, $30\n"
                        "nop\n"
                        "nop\n"
                        "addiu  $v0, $sp, 0x10\n"
                        "addu   $t4, $v0, $zero\n"
                        "swc2   $31, 0($t4)\n"
                        : "=m"(sp_tmp)
                        : "r"(dist_sq)
                        : "$2", "$12");
                    lzcr = sp_tmp;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tbl = *(((u8 *)&g_sqrt_table_u8) + ((u32)dist_sq >> shift));
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            tgt_y = 0x400 - ratan2(dist, *(s32 *)(*(u8 **)obj + 0x184) - *(s32 *)(obj + 0x184));
            if (tgt_y < -0xFF) {
                tgt_y = -0xFF;
            } else if (tgt_y >= 0x100) {
                tgt_y = 0xFF;
            }
            tgt_x = 0;
        }
    } else {
        tgt_x = 0;
        tgt_y = 0;
        tgt_z = 0;
    }

    ang = (tgt_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;
    if (ang >= 0x800) {
        ang -= 0x1000;
    }
    *(s16 *)(obj + 0x1E6) = *(s16 *)(obj + 0x1E6) + ang / 8;
    ang = (tgt_y - *(s16 *)(obj + 0x1E8)) & 0xFFF;
    if (ang >= 0x800) {
        ang -= 0x1000;
    }
    *(s16 *)(obj + 0x1E8) = *(s16 *)(obj + 0x1E8) + ang / 8;
    func_8002F770((s16 *)(a + 0x36), *(s16 *)(obj + 0x1E6), *(s16 *)(obj + 0x1E8), tgt_x);
    func_8002F770((s16 *)(b + 0x36), *(s16 *)(obj + 0x1E6), *(s16 *)(obj + 0x1E8), tgt_x);

    t = *(s16 *)(obj + 0xC);
    if ((t == 0x1D || t == 0xE) && *(s16 *)(obj + 0x8C) != 0) {
        /* FAKE: variable reuse -- the clamped twist is held in `ang`, the
         * angle scratch of the easing steps above (its last value there is
         * dead: consumed by the 0x1E8 store), not in a local of its own.
         * mechanism: register allocation of the shared pseudo (effect
         * observed in the diff, not dump-traced) -- shared, it sits in $a0 as
         * the target has it; a separate local lands in $v1
         * (measured 11/347, rejected/split-twist-11.c). All three reuses separated:
         * 26/347 (rejected/split-all-26.c). Family: variable reuse for codegen control
         * (SOTN-accepted, no-new-park-categories.md, 2026-06-02). */
        ang = (ratan2(D_800A387C, *(s32 *)(*(u8 **)obj + 0xF8) - *(s32 *)(obj + 0xF8)) - 0x400) & 0xFFF;
        if (ang >= 0x800) {
            ang -= 0x1000;
        }
        if (ang >= 0x200) {
            ang = 0x1FF;
        } else if (ang < -0x1FF) {
            ang = -0x1FF;
        }
        *(s16 *)(obj + 0x1EA) = ang;
        *(u16 *)(a + 0x7E) += ang;
        *(u16 *)(b + 0x7E) += ang;
    }

    if ((u32)(*(u16 *)(obj + 0xE) - 6) < 2U && *(u16 *)(obj + 0x6A) == 2) {
        if (*(s32 *)(obj + 0x268) == 0) {
            *(s32 *)(obj + 0x25C) = *(s32 *)(obj + 0xF4);
            *(s32 *)(obj + 0x260) = *(s32 *)(obj + 0xF8);
            *(s32 *)(obj + 0x264) = *(s32 *)(obj + 0xFC);
        }
        dx = *(s32 *)(*(u8 **)obj + 0xF4) - *(s32 *)(obj + 0x25C);
        dz = *(s32 *)(*(u8 **)obj + 0xFC) - *(s32 *)(obj + 0x264);
        dist_sq = dx * dx + dz * dz;
        if ((u32)dist_sq < 0x400) {
            dist = (u32)*(((u8 *)&g_sqrt_table_u8) + dist_sq) >> 3;
        } else {
            s32 lzcr = 0;
            if (dist_sq >= 0) {
                /* gte_Lzc(r1,r2) again (gtemac.h:174-178; see the first island
                 * for the macro expansion and the addressing-preamble
                 * disclosure).  Identical text except the LZCR frame slot: this
                 * site's `"=m"(sp_tmp2)` sits at sp+0x14, the target's own
                 * second site (asm/funcs/func_8001F2E4.s, `addiu $v0,$sp,0x14`). */
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addiu  $v0, $sp, 0x14\n"
                    "addu   $t4, $v0, $zero\n"
                    "swc2   $31, 0($t4)\n"
                    : "=m"(sp_tmp2)
                    : "r"(dist_sq)
                    : "$2", "$12");
                lzcr = sp_tmp2;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&g_sqrt_table_u8) + ((u32)dist_sq >> shift));
                dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }
        /* FAKE: variable reuse -- the twist target is held in `twist_tgt`, the
         * elevation target of the first step (dead after its last read in the
         * 0x1E8 easing step),
         * mechanism: register allocation of the shared pseudo (effect
         * observed in the diff, not dump-traced) -- shared, it sits in $a1 as
         * the target has it; a separate local lands in $v1 (measured 9/347,
         * rejected/split-twist-target-9.c). Same family as above. */
        twist_tgt = (ratan2(dist, *(s32 *)(*(u8 **)obj + 0xF8) - *(s32 *)(obj + 0x260)) - 0x400) & 0xFFF;
        if (twist_tgt >= 0x800) {
            twist_tgt -= 0x1000;
        }
        if (twist_tgt >= 0x200) {
            twist_tgt = 0x1FF;
        } else if (twist_tgt < -0x1FF) {
            twist_tgt = -0x1FF;
        }
        ang = (twist_tgt - *(s16 *)(obj + 0x1EA)) & 0xFFF;
        if (ang >= 0x800) {
            ang -= 0x1000;
        }
        *(s16 *)(obj + 0x1EA) = *(s16 *)(obj + 0x1EA) + ang / 8;
        *(u16 *)(a + 0x72) += *(s16 *)(obj + 0x1EA);
        *(u16 *)(b + 0x72) += *(s16 *)(obj + 0x1EA);
    }

    if (*(s16 *)(obj + 0x26E) != 0 && *(s16 *)(obj + 0x96) == 0) {
        /* FAKE: variable reuse -- the jitter is held in `ang` (dead on every
         * path here: its last value, from the 0x1E8 step, block 3 or the
         * 0x1EA step, is never read again), mechanism: register allocation of the shared
         * pseudo (global.c; effect observed in the diff, not dump-traced) --
         * with it, the 0x1EA delta above divides in place in $a0 as the target
         * does; a separate jitter local leaves
         * a copied temp there (measured 6/347, rejected/split-jitter-6.c). Same family. */
        ang = (rng_Next() & 0x3F) - 0x20;
        *(u16 *)(a + 0xC) += ang;
        *(u16 *)(b + 0xC) += ang;
        *(u16 *)(a + 0x14) -= ang;
        *(u16 *)(b + 0x14) -= ang;
        ang = (rng_Next() & 0x3F) - 0x20;
        *(u16 *)(a + 0x1E) += ang;
        *(u16 *)(b + 0x1E) += ang;
        *(u16 *)(a + 0x26) -= ang;
        *(u16 *)(b + 0x26) -= ang;
    }
}
