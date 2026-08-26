/* candidate.c - func_80023648 (best-known form; unchanged at s2, 2026-08-26)
 * Floor: 30 (sandbox --disable all, re-measured s2 with this body applied;
 * 159/159 insns). Residual is pure RA renames.
 *
 * s3 (structural, 2026-08-26) UPDATE -- READ THIS BEFORE TRUSTING THE s2 NOTE
 * BELOW. The s2 note's "attack pseudo 86" conclusion is superseded. Measured
 * with the instrumented cc1's second hook, BB2_ALLOC_DEBUG=1 (global.c:601-618),
 * the REAL allocation order is `129 122 85 81 72 134 176 75 86 126 153 83 110
 * 135 192 107 158 173` -- div16 is pseudo 129 (not 127; 127 is not an allocno
 * at all, which is why s2's BB2_FINDREG_DEBUG=127 showed NO HIT), and 129 is
 * ord=0, the FIRST allocno find_reg scans. Its measured pass-0 state is
 * conflicts={2,29}, someone_prefers={3}: $v1 is denied to div16 by a single
 * regs_someone_prefers bit -- a PREFERENCE bit, not a conflict bit. Target
 * seats div16 in $v1. So the root divergence is 129, pseudo 86 (ord=8) is
 * cascade, and the lever is preference structure (reachable from C: s3's
 * `swapadd` operand-order variant moved the score to 33 at 159/159, proving
 * set_preference retargeting works here) rather than conflict structure (which
 * the 159-insn multiset pins). s2's lever (A) is dead: s3's `andcond`
 * diagnostic put two values live across the table-entry load and the entry took
 * $v0, not $a2. See hypotheses.md H9 for the live frontier.
 * s3 structural probes, all 159/159: swapadd 33, swapelse 30 (neutral),
 * swapboth 33, andcond 47, condswap 36 -- banked in rejected/s3-*.c.
 *
 * s2 (structural modality) measured SEVEN structural respellings of this body;
 * every byte-neutral one scored exactly 30 and NONE changed the allocation:
 *   declmove (new_var declared in the inner block)          -> 30
 *   sib      (sibling func_800233AC's nested `s16 *tbl=...`)-> 30
 *   abs_outer(abs_val declared at the kind-block top)       -> 30
 *   merge_a2abs (ONE local serving table entry AND abs_val) -> 30 (no-op:
 *              ours already co-seats both in $v1)
 *   flatten  (decls hoisted out of the two nested sub-blocks)-> 30
 *   declorder(inner-block declaration order permuted)       -> 30
 *   scope_lookup (lookup pointers in their own nested scope)-> 30
 *   clamp_gt (`> 0x400` instead of `>= 0x401`)              -> 30
 * Non-neutral (banked in rejected/): declinit (`s16 *new_var = &D_8008EB40;`
 * as an inner declaration-with-initializer) -> 38; `s16 a2` narrowing of the
 * table value -> 39 at 161/159 insns.
 *
 * WHY none of them can work (s2 mechanism, measured with the instrumented cc1's
 * BB2_FINDREG_DEBUG hook, not hypothesised): the root flip is global.c
 * find_reg pass 0 for pseudo 86 (the D_8008EB40 table entry). Its measured
 * pass-0 state is conflicts={2,4,5,16,29}, someone_prefers={}, and
 * regs_used_so_far already contains 6($a2). $v1(3) is in NEITHER exclusion set,
 * so pass 0 takes the lowest free reg = 3. Target seats it in $a2. Adding 3 to
 * either hard_reg_conflicts[86] or regs_someone_prefers[86] lands $a2 exactly
 * -- but both sets are fixed by the DATA FLOW, which the 159-insn multiset
 * pins, so no declaration/scope/type/statement-order change can reach them.
 * Do NOT re-run the structural axis; do NOT respell the lookup region.
 * Next: see hypotheses.md H4/H5 (data-flow-level v1 occupancy; local-alloc
 * seat of pseudos 125/133 feeding set_preference).
 */
void func_80023648(u8 *arg0) {
    u16 kind = *(u16 *)(arg0 + 0x6A);
    s16 *new_var;

    if (kind == 0x13 || kind == 0x1B || kind == 0x30) {
        u32 bits = *(u32 *)(arg0 + 0x2C);
        if (bits & 0xF000) {
            s32 a1 = (bits >> 14) & 1;
            s32 a0;
            s32 a2;
            s16 *row;

            if (!(bits & 0x1000)) {
                a1++;
            }
            a0 = (bits >> 15) & 1;
            if (!(bits & 0x2000)) {
                a0++;
            }

            new_var = &D_8008EB40;
            row = new_var + (a0 * 3);
            a2 = row[a1];

            if (D_800A38BA != 0 && *(s16 *)(arg0 + 6) == 0) {
                func_8001F860((s16 *)arg0, *(s16 *)(arg0 + 0x1CA) + a2 / 4);
            } else {
                func_8001F860((s16 *)arg0, *(s16 *)(arg0 + 0x1D8) + a2);
            }
        } else {
            if (D_800A38BA != 0 && *(s16 *)(arg0 + 6) == 0) {
                *(s16 *)(arg0 + 0x14C) = 0;
            }
        }

        {
            s32 abs_val = *(s16 *)(arg0 + 0x150);
            if (abs_val < 0) {
                abs_val = -abs_val;
            }
            if (abs_val >= 0x401) {
                abs_val = 0x400;
            }

            {
                s32 sub_result = *(u16 *)(arg0 + 0x14E) - abs_val;
                s32 div16 = *(s16 *)(arg0 + 0x1A);
                s16 new_14e;
                s32 tbl_val;
                s32 mult_res;
                s32 limit;

                *(s16 *)(arg0 + 0x14E) = sub_result;
                if (div16 < 0) {
                    div16 += 15;
                }
                div16 >>= 4;
                new_14e = sub_result;
                new_14e = new_14e + div16;
                *(s16 *)(arg0 + 0x14E) = new_14e;

                tbl_val = (&D_800A310C)[(&D_8008DA08)[*(s16 *)(arg0 + 0xA)]];
                sub_result = *(s16 *)(arg0 + 0x1A);
                mult_res = sub_result * tbl_val;
                limit = (mult_res << 4) >> 12;

                if (limit < (s16)new_14e) {
                    *(s16 *)(arg0 + 0x14E) = limit;
                } else if ((s16)new_14e < 0) {
                    *(s16 *)(arg0 + 0x14E) = 0;
                }

                {
                    s32 speed_prod = *(s16 *)(arg0 + 0x14E) * *(s16 *)(arg0 + 0x44);
                    s32 speed = speed_prod >> 12;
                    s16 sin_val = (&Judge)[(*(u16 *)(arg0 + 0x1CA) & 0xFFF)];

                    *(s32 *)(arg0 + 0xD8) += (sin_val * speed) >> 16;

                    {
                        s16 cos_val = (&Judge)[((*(s16 *)(arg0 + 0x1CA) + 0x400) & 0xFFF)];
                        *(s32 *)(arg0 + 0xE0) += (cos_val * speed) >> 16;
                    }
                }
            }
        }
    } else {
        if (*(s16 *)(arg0 + 0x14E) > 0) {
            if (kind != 0x22) {
                *(s16 *)(arg0 + 0x14C) = 0;
            }
            *(s16 *)(arg0 + 0x14E) = 0;
        }
    }
}
