/* func_8002D780 - grind candidate (s23 rederive, 2026-09-15).  Honest sandbox floor
 * 0/202, build_insns == target_insns == 202, measured THIS session with these exact
 * edits in src/code6cac_b.c (`sandbox func_8002D780 --disable all` -> 0), and the full
 * oracle build re-run once with the body in place: build SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == original (tmp/grind/func_8002D780/s23/
 * verify_oracle.txt).  Chassis: HEAD 37e78bb88 (-mel -msoft-float), with the matched
 * sibling func_8002CA8C (this function's CALLER) now in the TU.
 *
 * WHAT CLOSED THE TWO-INSTRUCTION RESIDUAL (s5..s22 floor 2/202).  The residual was never
 * a spelling of block 7 in isolation; it was the ADMISSION of one block-7 pseudo to
 * local-alloc.  s17/s19 derived that in the target's own emission order dz and dx tie
 * exactly in qty_compare_1 and dz (lower quantity number) takes $v1, whereas the target
 * has dx in $v1 and dz in $a0.  The way out is not to win the tie but to never enter
 * it: local_alloc (local-alloc.c:472) only admits pseudos with REG_BASIC_BLOCK >= 0 and
 * REG_N_DEATHS == 1.  A pseudo referenced in TWO basic blocks is REG_BLOCK_GLOBAL, is
 * left to global_alloc, and block 7's quantity table then contains only dx, which takes
 * the first free seat $v1; global_alloc hands the two-block pseudo $a0 (the only
 * register free over both its ranges).  So the edge difference z2 - z0 is held in a
 * function-scope scratch `tmp` that the sqrt block also uses for its table byte (the
 * target keeps that byte in $a0 too: `lbu a0,LUT(at); sll a0,a0,16`).  Measured: seats
 * correct in every such form (tb_tt 2, tb_B5 0, tb_B1 0; a `tmp` shared with the sqrt
 * `dist` copy instead gives the same seats but a cse residual, sh_axre 1).
 *
 * The second thing the sharing exposed is sched.c adjust_priority -> birthing_insn_p
 * (reg_n_sets == 1): once `tmp` is twice-assigned it loses the "births a register" max
 * priority that a once-assigned local gets, and the once-assigned `ax` is then emitted
 * AFTER it regardless of source order (BB2_SCHED_DEBUG trace, tmp/grind/func_8002D780/
 * s23/adjpri_ttB.txt: ADJPRI insn=180 (ax) birth=1, insn=183 (tmp) birth=0).  Staging the
 * query difference pz - z0 through the now-dead `ax` makes both scratch locals
 * twice-assigned, rank_for_schedule falls through to INSN_LUID (source order) and block
 * 7 emits ax, tmp, dx, az exactly as the target does (delay slot = ax).  The alternative
 * of sharing `ax` with the sqrt block's `y` reload also scores 0/202 and is banked as
 * candidate_alt_s23_ax_shared_with_sqrt_y.c.
 *
 * The `m` re-store's mechanism is CORRECTED here (s5's "cse.c make_regs_eqv" story was
 * wrong): cse_end_of_basic_block follows the `dist < 0` skip over the LZC arm
 * (skip_blocks, cse.c:8092-8140) and canonicalises every later read of `m` to `dist`
 * unless something in the SKIPPED arm sets `m` (invalidate_skipped_block).  That is why
 * the srlv reads $s1 without the re-store (drop-1 = 4/202 this session) and why
 * do-while(0) wraps cannot replace it (cse2 ignores NOTE_INSN_LOOP_END, cse.c:8046).
 *
 * FAKE constructs in this body (all annotated at the site): (1) `tmp = z2 - z0` /
 * `tmp = *LUT` - one scratch local reused for two real, consumed values
 * (staged-value-reused-variable family, .claude/rules/staged-value-reused-variable.md;
 * frozen list "variable reuse for codegen control"); (2) `ax = pz - z0` - a real value
 * staged through the existing dead local `ax` (same family, the rule's own origin
 * mechanism); (3) `m = dist` same-value re-store (dead-store family,
 * .claude/rules/dead-store-fake-exception.md; in-TU precedent src/code6cac_b.c:1330).
 * fake_ablate on this body: keep-all 0, drop the `m` re-store 4.  The rest of the body is
 * the s14 chassis unchanged (split cop2 islands, canonical LZCS/LZCR + mvmva idiom per
 * .claude/rules/cop2-addressing-preamble-cluster.md).  Full derivation: evidence.md and
 * hypotheses.md s23. */
s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    /* One scratch local shared by two distant jobs: the third edge test's `z2 - z0`
     * (block 7) and the sqrt block's table byte.  See the FAKE notes at both stores. */
    s32 tmp;

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
                tmp = z2 - z0;
                dx = x2 - x0;
                az = cz - z0;
                kc = (tmp * ax) - (dx * az);
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
                kp = (tmp * (px - x0)) - (dx * ax);
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
                    : "=m"(sp_var) : "r"(&sp_var) : "$12", "$13", "$14", "$15");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                /* FAKE: the table byte is staged through the existing, now-dead scratch
                 * `tmp` (block 7's z2 - z0 died at its last multiply; this value is consumed
                 * on the next line) - the second job that makes `tmp` a two-block pseudo, see
                 * the block-7 note for the mechanism (local-alloc.c:472 admission) and
                 * lever-exhaustion. */
                tmp = *((&D_8008D118) + ((u32)m >> shift));
                sqrt_val = (u32)(tmp << 16) >> (0x13 - ((u32)shift >> 1));
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
