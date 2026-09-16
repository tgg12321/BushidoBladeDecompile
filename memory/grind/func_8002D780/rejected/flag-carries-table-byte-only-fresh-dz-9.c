/* REJECTED (s23 third run, 2026-09-15): the `flag` parameter carries only the sqrt block's table byte while block 7 keeps a fresh once-written block-local `dz`: 9/202 at 202 build insns (score_F4.json). The block-7 pseudo is single-block again and ties dx in qty_compare_1 (the s14 residual), and the table-byte borrow moves the sqrt block's seats on top of it. Measured on HEAD 2023082f8. */
s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        s32 *vin;
        s32 *vout;
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        vin = (s32 *)(obj + 0xF8);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "lwc2 $0, 0($t4)\n"
            "lwc2 $1, 4($t4)\n"
            "nop\n"
            "nop\n"
            ".word 0x4A486012"
            : : "r"(vin) : "$12", "memory");
        vout = (s32 *)(obj + 0x100);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "swc2 $25, 0($t4)\n"
            "swc2 $26, 4($t4)\n"
            "swc2 $27, 8($t4)"
            : : "r"(vout) : "$12", "memory");
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
                s32 ax = cx - x0;
                s32 dx;
                s32 az;
                s32 dz;
                /* FAKE: the edge difference z2 - z0 is staged through the function-scope
                 * scratch `tmp` (also assigned the sqrt table byte below) instead of a fresh
                 * block-local, mechanism: local-alloc.c local_alloc admission
                 * (local-alloc.c:472 REG_BASIC_BLOCK >= 0 && REG_N_DEATHS == 1) - a pseudo
                 * referenced in two blocks is left to global.c, so block 7's local-alloc
                 * table seats dx first in $v1 and global_alloc gives tmp $a0 (the target's
                 * seats; a block-local dz ties dx in qty_compare_1 and takes $v1 itself),
                 * lever-exhaustion: memory/grind/func_8002D780/hypotheses.md s14-s22
                 * (declaration order/scope, statement order, staging, hoisting, sign flips,
                 * 2,080 + 816 + 528 enumerated block-local spellings, all >= 2/202). */
                dz = z2 - z0;
                dx = x2 - x0;
                az = cz - z0;
                kc = (dz * ax) - (dx * az);
                /* FAKE: the query difference pz - z0 is staged through the existing, now-dead
                 * local `ax` (its cx - x0 value was consumed by the kc line above; this value
                 * is consumed on the next line), mechanism: sched.c adjust_priority ->
                 * birthing_insn_p (reg_n_sets == 1): a once-assigned `ax` gets max priority in
                 * sched1 and is emitted AFTER the twice-assigned `tmp`, transposing the
                 * target's `ax` (delay slot) / `tmp` order; a twice-assigned `ax` ties and
                 * rank_for_schedule falls through to source order, lever-exhaustion:
                 * memory/grind/func_8002D780/hypotheses.md s23 (sh_tt/sh_ttB/tb_tt: 5, 3, 2;
                 * ax reused for px - x0: 23; both reused: 23; tmp first in source: 2). */
                ax = pz - z0;
                kp = (dz * (px - x0)) - (dx * ax);
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
            sqrt_val = (u32)*((&D_8008D118) + dist) >> 3;
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
                __asm__ volatile(
                    "addu $t4, %0, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop"
                    : : "r"(m) : "$12");
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(&sp_var) : "$12", "$14", "$15");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                /* FAKE: the table byte is staged through the existing, now-dead scratch
                 * `tmp` (block 7's z2 - z0 died at its last multiply; this value is consumed
                 * on the next line) - the second job that makes `tmp` a two-block pseudo, see
                 * the block-7 note for the mechanism (local-alloc.c:472 admission) and
                 * lever-exhaustion. */
                flag = *((&D_8008D118) + ((u32)m >> shift));
                sqrt_val = (u32)(flag << 16) >> (0x13 - ((u32)shift >> 1));
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
