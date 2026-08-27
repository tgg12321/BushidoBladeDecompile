/* candidate.c - func_80023648 (MATCHED form; sandbox --disable all = 0 at
 * 159/159 insns, rules_dropped 0; found s6 2026-08-26, annotation corrected and
 * floor RE-MEASURED at 0 in the s6b annotation-fix session 2026-08-26)
 *
 * MIGRATION BANNER (asm-until-matched, owner ruling 2026-08-19): this function's
 * representation on main is `INCLUDE_ASM("asm/funcs", func_80023648);`. NOTHING in
 * this file is on main. Every score quoted below is a sandbox --disable all
 * measurement taken with this body TEMPORARILY applied to src/code6cac.c inside a
 * grind session; the tree is left at HEAD when the session ends unless the
 * candidate is accepted and integrated by the driver/operator.
 *
 * ---------------------------------------------------------------------------
 * HOW THIS FORM WAS FOUND (s6, synthesis modality; full record in
 * evidence.md/hypotheses.md s6)
 *
 * The 15-line residual inherited from s4/s5 was decomposed by an ALIGNED SEAT
 * DIFF (tmp/grind/func_80023648/s6/{diffseats.sh,cmp.py}) into exactly three
 * clusters, 5 + 7 + 3 = 15:
 *   C1  the element-address add: ours `addu v0,v0,v1` (row first), target
 *       `addu v1,v1,v0` (scaled index first)          -> fixed by `a1[row]`
 *   C2  the clamped |*(s16 *)(arg0+0x150)| seated in $a0, target $a2
 *   C3  the >>12 speed seated in $a1, target $a2
 * C2 and C3 are closed by staging both values through the EXISTING `a2` local
 * (target seats all three unrelated values in $a2 because GCC 2.7.2 makes a
 * multiply-set C variable one pseudo = one allocno). Each edit was measured
 * SEPARATELY at an unchanged 159/159: 15 -> 8 (C1) -> 3 (C2) -> 0 (C3).
 *
 * The four FAKE annotations below are mandatory, not decorative: three cover the
 * staged-value reuses (frozen SOTN family "variable reuse for codegen control",
 * .claude/rules/staged-value-reused-variable.md) and the fourth covers the
 * index-first subscript, which is semantically-true C on the ALLOWED side of the
 * 2026-07-06 construct-honesty line (.claude/rules/do-while-zero-exception.md:46)
 * but is purely-for-matching (`&row[a1]` = 8 vs `a1[row]` = 0) and therefore owes
 * the FAKE mark. The 2026-08-26 18:56 Judge FAIL was on that missing mark ALONE;
 * the body itself was verified and PASSed on every other ground.
 *
 * TRAPS BANKED IN s6 - read before editing this body:
 *   * GCC 2.7.2 is C89 and the build runs with -w: an assignment placed ABOVE a
 *     sibling declaration in the same block silently miscompiles, collapsing
 *     build_insns 159 -> 146 (OFF-MULTISET) with no diagnostic. Six variants died
 *     this way (rejected/s6-*-insn146.c). Keep every assignment BELOW the block's
 *     declarations.
 *   * s4's `ent = &row[a1]` pointer split is worth ZERO once the operand order is
 *     right; it was a proxy for C1 and has been removed.
 *   * The alternative index-first spelling `*(s16 *)((a1 << 1) + (s32)row)` also
 *     measures 0 and is the first fallback if `a1[row]` is ever rejected.
 */
void func_80023648(u8 *arg0) {
    u16 kind = *(u16 *)(arg0 + 0x6A);
    s16 *new_var;

    if (kind == 0x13 || kind == 0x1B || kind == 0x30) {
        s32 a2;
        u32 bits = *(u32 *)(arg0 + 0x2C);
        if (bits & 0xF000) {
            s32 a1 = (bits >> 14) & 1;
            s32 a0;
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
            /* FAKE: index-first element address `a1[row]` (identical value to
             * `row[a1]` - C defines E1[E2] as *(E1+E2), so this is the same load),
             * mechanism: GCC 2.7.2 RTL expansion emits the operands of the
             * commutative PLUS in source order, so index-first flips the addu
             * operand order and the element pointer lands in $a2 as target does -
             * measured 8 -> 0, lever-exhaustion:
             * memory/grind/func_80023648/hypotheses.md s6 */
            a2 = a1[row];

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
            /* FAKE: the clamped |*(s16*)(arg0+0x150)| is staged through the
             * existing `a2` (its D_8008EB40 table-entry value is dead here - it
             * was consumed by the func_8001F860 call above and is never read
             * again), mechanism: GCC 2.7.2 global.c - a multiply-set pseudo is
             * ONE allocno spanning all of its live ranges, so global_alloc seats
             * every staged value in a single hard reg ($a2) exactly as target
             * does; separate locals form separate allocnos that find_reg seats in
             * $a2/$a0/$a1, lever-exhaustion: memory/grind/func_80023648/hypotheses.md
             * (s1-s5: structural axis, named-intermediate axis, two permuter basins) */
            a2 = *(s16 *)(arg0 + 0x150);
            if (a2 < 0) {
                a2 = -a2;
            }
            if (a2 >= 0x401) {
                a2 = 0x400;
            }

            {
                s32 sub_result = *(u16 *)(arg0 + 0x14E) - a2;
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
                new_14e = sub_result + div16;
                *(s16 *)(arg0 + 0x14E) = new_14e;

                tbl_val = (&D_800A310C)[(&D_8008DA08)[*(s16 *)(arg0 + 0xA)]];
                /* FAKE: the second read of *(s16*)(arg0+0x1A) is staged through
                 * the existing `sub_result` (its 0x14E difference is dead here -
                 * consumed by the store above and by new_14e), mechanism: GCC
                 * 2.7.2 global.c multiply-set pseudo / single allocno as above,
                 * lever-exhaustion: memory/grind/func_80023648/hypotheses.md */
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
                    s16 sin_val = (&Judge)[(*(u16 *)(arg0 + 0x1CA) & 0xFFF)];

                    /* FAKE: the >>12 speed is staged through the existing `a2`
                     * (its clamped-|0x150| value is dead here - consumed by
                     * sub_result above), mechanism: GCC 2.7.2 global.c
                     * multiply-set pseudo / single allocno as above,
                     * lever-exhaustion: memory/grind/func_80023648/hypotheses.md */
                    a2 = speed_prod >> 12;

                    *(s32 *)(arg0 + 0xD8) += (sin_val * a2) >> 16;

                    {
                        s16 cos_val = (&Judge)[((*(s16 *)(arg0 + 0x1CA) + 0x400) & 0xFFF)];
                        *(s32 *)(arg0 + 0xE0) += (cos_val * a2) >> 16;
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
