/* REJECTED - func_80070C70 s4. WHY DEAD: TWO source increment sites for var_s0 (biv_count 2)
 * leave the combined D_800A3590 address giv at benefit 6 - 2*2 = 2, product 56..62 against
 * insn_count 48, so loop.c:3824 still REDUCES it - and both 2-site spellings measure 101, i.e.
 * two points WORSE than the one-site floor-99 chassis. Only biv_count 3 (benefit 0) rejects it.
 * Measured on the floor-99 chassis (s3 candidate.c + array/RecC70/IconC70 declarations),
 * no FAKE constructs present, `sandbox --disable all`, 2026-09-10.
 *   A-continue-2sites : all three increments duplicated into a `continue`-style skip arm  -> 101 (200 insns)
 *   D-ifelse-2sites   : the same with the skip arm spelled as an `if/else`                -> 101 (200 insns)
 *   C-for-comma       : `for (; cond; var_s3 += 0x16C, var_s0 += 1, ctx += 3)` (1 site)   -> 102 (200 insns)
 * Script: tmp/grind/func_80070C70/s4/probe.py
 */
        do {
            u8 code = D_800A3560[ctx_or_var_s2];
            if ((code == 5) || (code == 16)) {
                var_s3 += 0x16C;
                var_s0 += 1;
                ctx_or_var_s2 += 3;
                continue;
            }
            {
                s32 t = prim.p_geom + 0xC;
                prim.p_static = t;
                prim.p_static = t + (D_800A3590[var_s0].v << 4);
                if (((D_800A35B0 + (s16)(u16)D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.mode = var_s3;
                } else {
                    prim.mode = 0x105;
                }
                prim.code = 1;
                prim.link = *(s32 *)(arg0 + 0x10);
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
            var_s3 += 0x16C;
            var_s0 += 1;
            ctx_or_var_s2 += 3;
        } while (var_s0 < (s32)(D_800A35B0 + ((s16)D_800A3558 + 1)));
