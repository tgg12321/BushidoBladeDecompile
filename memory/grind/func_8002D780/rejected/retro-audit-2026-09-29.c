/* RETRO-AUDIT 2026-09-29 FAIL -- func_8002D780 reopened (Q38, owner rulings 803d0fea1).
 * Landed on main in 957140ebc (src/code6cac_b.c); this is that landed text, verbatim from main
 * as of the reopen, banked before the body went back to INCLUDE_ASM.
 * FAIL: canonical grant rows (9bdfcc6cc, 'owner-instructed 2026-09-21') have no recorded owner instruction, and the islands do not qualify under the 2026-09-26 inline_o.h class grant (audit-q38: gtemacro match_unit None; `addu $t4` preamble form, joined statements, unpinned macros).
 * Detail: tmp/audit-2026-09-29/review/batch_00.md (gitignored), tmp/audit-2026-09-29/SUMMARY.md;
 * rulings: docs/grind/owner-rulings-2026-09-26.md Q37/Q38 (803d0fea1).
 * Reopen notes: Registry rows removed (commented): inline_asm_canonical.txt, tools/grinder/owner_cluster_grants.txt, tools/canonical_asm_regions.json.
 * DO NOT resubmit this body as-is. */

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
                /* FAKE: the third edge test's edge difference z2 - z0 is staged through the
                 * `flag` parameter (its own job, the mode test at entry, is finished: nothing
                 * reads `flag` after `if (flag == 0)`, and this value is consumed by the two
                 * products below and never needed again), instead of through a fresh
                 * block-local, mechanism: local-alloc.c local_alloc admission (local-alloc.c:472
                 * REG_BASIC_BLOCK >= 0 && REG_N_DEATHS == 1) - a pseudo referenced in two basic
                 * blocks (the entry test and this block) is left to global.c, so block 7's
                 * local-alloc quantity table seats dx first in $v1 and global_alloc, reaching the
                 * parameter's pseudo fourth in allocno order, seats it in $a0, the lowest free
                 * register at that turn (tmp/grind/func_8002D780/s23/r4 greg: "72 in 4"; the
                 * entry copy from $a0 is folded away by combine AFTER flow has fixed the
                 * pseudo's REG_BASIC_BLOCK as global) (the target's seats; a block-local dz
                 * ties dx in qty_compare_1 and takes $v1 itself),
                 * lever-exhaustion: memory/grind/func_8002D780/hypotheses.md s14-s23
                 * (declaration order/scope, statement order, staging, hoisting, sign flips,
                 * 2,080 + 816 + 528 enumerated block-local spellings, all >= 2/202; a fresh
                 * function-scope scratch shared with the sqrt block reaches 0 but was
                 * Judge-FAILed 2026-09-15 23:16 as an invented multi-write carrier; the
                 * `threshold` parameter as carrier scores 28; s23 third run). */
                flag = z2 - z0;
                dx = x2 - x0;
                az = cz - z0;
                kc = (flag * ax) - (dx * az);
                /* FAKE: the query difference pz - z0 is staged through the existing, now-dead
                 * local `ax` (its cx - x0 value was consumed by the kc line above; this value
                 * is consumed on the next line), mechanism: sched.c adjust_priority ->
                 * birthing_insn_p (reg_n_sets == 1): a once-assigned `ax` gets max priority in
                 * sched1 and is emitted AFTER the twice-assigned `flag`, transposing the
                 * target's `ax` (delay slot) / `flag` order; a twice-assigned `ax` ties and
                 * rank_for_schedule falls through to source order, lever-exhaustion:
                 * memory/grind/func_8002D780/hypotheses.md s23 (sh_tt/sh_ttB/tb_tt: 5, 3, 2;
                 * ax reused for px - x0: 23; both reused: 23; tmp first in source: 2). */
                ax = pz - z0;
                kp = (flag * (px - x0)) - (dx * ax);
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
                __asm__ volatile(
                    "addu $t4, %0, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop"
                    : : "r"(m) : "$12", "$13", "$14", "$15");
                /* Canonical GTE LZCS island (mtc2/swc2 -- no C form).
                 * CLOBBER PROVENANCE (do not read this list as SDK text): PsyQ
                 * publishes NEITHER half's clobbers. gte_ldlzc (inline_c.h:228)
                 * is "mtc2 %0, $30" with no clobber list at all, and gte_stlzc
                 * (inline_c.h:1318) is "swc2 $31, 0(%0)" with only "memory" --
                 * none of $12-$15 is SDK text. The "$12","$13","$14","$15" set is
                 * the PROJECT-established conservative t4-t7 footprint for this
                 * hand-written LZCS island, granted 2026-07-28
                 * (docs/grind/decisions.md:1750) and shipped byte-identically by
                 * the two matched siblings in this same file (func_8002BC68 at
                 * src/code6cac_b.c:766, func_8002BEA0 at :829; both registry-listed
                 * in inline_asm_canonical.txt). Per register, honestly: $12 is
                 * WRITTEN by the island itself; $15 and $14 are each forced by this
                 * function's own target bytes (below); $13 is BYTE-INERT here and is
                 * present only so the list stays a non-selective conservative set
                 * rather than a tuned subset -- the exact remedy the 2026-09-16
                 * layer-2 note asked for. It is a bytes-forced reconstruction of the
                 * original island's register footprint, NOT a register pin:
                 * reload1.c:3730-3739 puts
                 * every regs_explicitly_used[] hard reg into bad_spill_regs, and
                 * reload1.c:727 seeds retry_global_alloc's forbidden_regs from it,
                 * so (a) the sqrt block's y*y reload lands in $t8 (target .s:128)
                 * instead of $t7 only if $15 is mentioned, and (b) pseudo 128's
                 * retry seat is $s1 (target .s:99-100) instead of $t6 only if $14
                 * is mentioned -- global.c:999-1001/1052-1082 otherwise take the
                 * lowest free regno, which is 14. $t7 has zero pseudo uses anywhere
                 * in the target; $t6 carries pseudo 119 in test 1 only (.s:68,82)
                 * and is provably not conflicting with pseudo 128's window, so an
                 * RTL mention is the only route for either. The list is NOT selected
                 * per register: the identical four-register set is written on BOTH
                 * halves of this one macro and matches the file's accepted siblings,
                 * which is what makes it a conservative over-approximation (the
                 * unsafe direction for a clobber is omission, never inclusion). */
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(&sp_var) : "$12", "$13", "$14", "$15");
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
