/* candidate.c - func_80023648 (best-known form; floor 15; s4 permuter, header
 * updated s5 2026-08-26)
 *
 * MIGRATION BANNER (asm-until-matched, owner ruling 2026-08-19): this function's
 * representation on main is `INCLUDE_ASM("asm/funcs", func_80023648);`. NOTHING in
 * this file is on main. Every score quoted below is a sandbox --disable all
 * measurement taken with this body TEMPORARILY applied to src/code6cac.c inside a
 * grind session and then reverted; the tree is left at HEAD when the session ends.
 *
 * FLOOR: 15 (sandbox --disable all, 159/159 insns). Re-measured and CONFIRMED at
 * the start of s5 with this exact body applied. Down from 30 at s1-s3.
 *
 * ---------------------------------------------------------------------------
 * HOW THIS FORM WAS FOUND (s4, permuter modality)
 *
 * Three campaigns were run through tools/permuter_campaign.py on a standalone
 * base.c that was first PROVEN faithful to the real chassis: the workspace's
 * base.o vs target.o comparison reproduced exactly the sandbox residual
 * (159/159 insns, 30 mismatching lines) before any permutation.
 *   1. tmp/perm_23648_s4  (vanilla, base = s3 candidate)   base_score 180
 *      -> 14 finds in ~12 min, best output-120-1.
 *   2. tmp/perm_23648_s4b (reseeded from varA below)       base_score 120
 *      -> 10 finds in ~21 min, best output-80-1.
 *   3. tmp/perm_23648_s4c (reseeded from THIS body)        base_score 95
 *      -> 17032 iterations, ZERO finds. Basin dry; harvested + stopped.
 *
 * The two levers that actually moved the sandbox floor, each measured
 * SEPARATELY at an unchanged 159/159:
 *   varA  reuse the `div16` local to carry `(s16)new_14e` for the `limit <`
 *         comparison                                      30 -> 22
 *   varL  name the table-element address (`ent = &row[a1]; a2 = *ent;`)
 *         on top of varA                                  22 -> 15
 * Both are in this body. varA alone is banked at
 * rejected/s4-perm-div16-reuse-only-22.c.
 *
 * What this OVERTURNED from s2/s3: the floor moving 30 -> 15 on two ordinary C
 * respellings, at a fixed 159-insn multiset, shows the preference structure IS
 * reachable from C and that the s2/s3 "structural axis is dead" framing was too
 * strong. It does NOT overturn the mechanism findings themselves (H9/H10 are
 * untouched); it removes their premise that spelling cannot reach preferences.
 *
 * ---------------------------------------------------------------------------
 * WHAT s5 ESTABLISHED ABOUT THESE TWO LEVERS (read this before proposing more
 * naming edits - it will save you a whole session)
 *
 * s4's frontier claimed "variable reuse and named address intermediates are
 * broadly effective on this function's preference structure". s5 tested that
 * EXHAUSTIVELY with a PERM_GENERAL cross-product over all six remaining
 * anonymous/single-use sites (the D_800A310C/D_8008DA08 table chain, the
 * (mult_res << 4) >> 12 limit computation, the sin index read of 0x1CA, the
 * 0xD8 accumulate, the cos index read of 0x1CA+0x400, the 0xE0 accumulate),
 * each with inline / named-scalar / named-pointer alternatives. The permuter
 * enumerated the whole space (216 iterations) and EVERY combination scored
 * EXACTLY the base score. Naming those values is invisible to GCC 2.7.2.
 *
 * So varL is NOT "a named intermediate". It is specifically the split of an
 * ADDRESS COMPUTATION away from its LOAD. Do not propose more naming edits on
 * plain values in this body; that axis is measured dead.
 *
 * Also measured dead in s5 (all banked in rejected/): the p14e pointer chassis
 * (16), hoisting the tbl chain above the 0x14E arithmetic (44 at 157 insns -
 * OFF-MULTISET), and three legal spellings of an `argp` alias for the two
 * func_8001F860 calls (15 / 22 / 15 - neutral).
 *
 * NEXT SESSION: the seat-level RA analysis is the live axis. The s1-s3 seat map
 * and the H9/H10 allocno ids (129 = div16, 122 = abs_val, 86 = the table entry)
 * were measured against the 30-floor body and are STALE. Apply this body, run
 * `pwsh tools/grinder/dump.ps1 func_80023648` plus BB2_ALLOC_DEBUG=1 and
 * BB2_FINDREG_DEBUG, produce a fresh 159-line aligned seat diff, and re-state
 * H9/H10 against the NEW allocno ids before spending a probe.
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
