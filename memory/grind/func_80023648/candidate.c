/* candidate.c - func_80023648 (best-known form; s4 permuter, 2026-08-26)
 * Floor: 15 (sandbox --disable all, measured this session with this exact body
 * applied to src/code6cac.c; 159/159 insns). Down from 30 at s1-s3.
 *
 * s4 (permuter modality) HOW THIS FORM WAS FOUND, and what it means for the
 * three-session RA analysis that preceded it.
 *
 * Three campaigns were run through tools/permuter_campaign.py on a standalone
 * base.c that was first PROVEN faithful to the real chassis: the workspace's
 * base.o vs target.o comparison reproduces exactly the sandbox residual
 * (159/159 insns, 30 mismatching lines) before any permutation, so permuter
 * score movement in this workspace is real chassis movement.
 *   1. tmp/perm_23648_s4  (vanilla, base = s3 candidate)   base_score 180
 *      -> 14 finds in ~12 min, best output-120-1.
 *   2. tmp/perm_23648_s4b (reseeded from varA below)       base_score 120
 *      -> 10 finds in ~21 min, best output-80-1.
 *   3. tmp/perm_23648_s4c (reseeded from THIS body)        base_score 95
 *      -> 17032 iterations, ZERO finds. Basin dry; harvested + stopped.
 *
 * The two levers that actually moved the sandbox floor, each measured
 * SEPARATELY at an unchanged 159/159 (the permuter proposes whole diffs; every
 * delta below was re-spelled by hand and measured on its own before being
 * kept):
 *   varA  reuse the `div16` local to carry `(s16)new_14e` for the `limit <`
 *         comparison                                      30 -> 22
 *   varL  name the table-element address (`ent = &row[a1]; a2 = *ent;`)
 *         on top of varA                                  22 -> 15
 * Both are in this body. varL alone was not measured separately; varA alone is
 * banked at rejected/s4-perm-div16-reuse-only-22.c.
 *
 * What this OVERTURNS from s2/s3: s2 concluded "no declaration/scope/type/
 * statement-order change can reach the allocator here" and s3 narrowed the
 * root to a single regs_someone_prefers bit on allocno 129. The floor moving
 * 30 -> 15 on two ordinary C respellings, at a fixed 159-insn multiset, shows
 * the preference structure IS broadly reachable from C and that the s2/s3
 * "structural axis is dead" framing was too strong -- it was an artefact of
 * hand-enumerating a small neighbourhood, not a property of the function. It
 * does NOT overturn the mechanism findings themselves (H9/H10 are untouched and
 * were never probed this session); it removes their premise that spelling
 * cannot reach preferences.
 *
 * Deltas measured and REJECTED this session (all 159/159, all banked under
 * rejected/s4-perm-*.c): pointer local for the tail `0x14E` store (30, neutral
 * and a dead construct), `abs_val` reused for the 0xD8 accumulate temp (23),
 * `tbl_val = speed` reuse (24), both together (24), `a1 = a2` reuse in the else
 * arm (22, neutral on top of varA), fused `*(...) = (new_14e = sub_result)`
 * (22, neutral), `argp = (s16 *)arg0` for the two func_8001F860 calls (17 --
 * BETTER than varA but WORSE than varL), and argp+ent stacked (24 -- the two
 * pointer intermediates actively fight each other, so they are not additive).
 *
 * NEXT SESSION: the productive move is another permuter cycle from THIS body
 * with a structurally different chassis (the s4c basin off this exact body is
 * dry after 17k iterations, so a plain reseed will not pay). Re-run the
 * seat-level analysis (BB2_ALLOC_DEBUG / BB2_FINDREG_DEBUG) against THIS body
 * first -- the s1-s3 seat map and the H9/H10 allocno ids (129 / 122) were
 * measured against the 30-floor body and are now stale.
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
            s16 *ent;

            if (!(bits & 0x1000)) {
                a1++;
            }
            a0 = (bits >> 15) & 1;
            if (!(bits & 0x2000)) {
                a0++;
            }

            new_var = &D_8008EB40;
            row = new_var + (a0 * 3);
            ent = &row[a1];
            a2 = *ent;

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

                div16 = (s16)new_14e;
                if (limit < div16) {
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
