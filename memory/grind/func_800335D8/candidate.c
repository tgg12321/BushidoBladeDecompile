/* func_800335D8 -- CANDIDATE (grind s3 = driver session 1 after ledger reset, 2026-09-08).
 * sandbox --disable all = 0 (176/176, rules_dropped 0), oracle SHA1 proven, edits in place.
 *
 * Measured THIS session (chassis: -mel -msoft-float, tools/gcc-2.7.2/build/cc1):
 *   canonical func_800335D8: verdict C, asm_insns 0, distance 0          (tmp/grind/func_800335D8/s1/resub_canonical_M1.txt)
 *   sandbox func_800335D8 --disable all = 0, 176/176                       (tmp/grind/func_800335D8/s1/resub_sandbox_M1.txt)
 *   sandbox func_80033510 --disable all = 0, 16/16 (sibling whose spelling changed)
 *                                                                          (tmp/grind/func_800335D8/s1/resub_sandbox_sibling_func_80033510.txt)
 *   verify-oracle --rebuild --allow-dirty: ok true,
 *     build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle        (tmp/grind/func_800335D8/s1/resub_verify_oracle_M1_full.txt)
 *   full edit set as a patch:                                              tmp/grind/func_800335D8/s1/resub_M1_full_merge.patch
 *
 * The function BODY is byte-for-byte the s2 K3 body (unchanged). What changed vs the s2 submission that layer-1
 * FAILed (docs/grind/decisions.md, 2026-09-08 20:27): the leaf random buffer merge is now COMPLETE --
 *   include/code6cac.h : `extern s16 D_800A3756;` replaced by `extern s16 D_800A3750[4];` (header-level, shared)
 *   src/code6cac_b.c   : the TU-local `extern s16 D_800A3750;` line REMOVED (no TU-local declaration of the buffer);
 *                        func_80033510's clear loop respelled `s16 *p1 = &D_800A3750[3];` (was `&D_800A3756`) --
 *                        byte-identical (sandbox 0, 16/16; the original asm/funcs/func_80033510.s starts its
 *                        downward walk at 0x800A3756 = element 3 of the 4-entry buffer)
 *   undefined_syms_auto.txt : `D_800A3756 = 0x800A3756;` row REMOVED (no C, header, or assembled .s names it:
 *                        asm/funcs/func_80033510.s is not INCLUDE_ASM'd and asm/6CAC.s is not in the build)
 *   named_syms.txt     : g_leaf_random_buffer_plus_6 (0x800A3756) row removed; g_leaf_random_buffer comment corrected
 *                        to "4 entries (0x7B is the sound-slot id base, not a size)"
 * so storage 0x800A3750..0x800A3757 has exactly ONE C handle (prong (c) of the aggregate-merge family).
 * Object model for the buffer (independent of this session): asm/funcs/func_80033510.s clears 4 halfwords from
 * 0x800A3756 down to 0x800A3750; asm/funcs/func_800335D8.s walks 0x800A3750..+8 at stride 2; every other referrer
 * of the 0x800A375x region (func_8001FBE8/func_80022580/func_800238C4/src/code6cac.c:3070) touches only the
 * separate u8 D_800A3758.
 *
 * The threshold-pair merges (LeafThreshold D_8008EA44[5] / D_8008EBFC[6]), u8 D_8008EBF4[6], and the TU arrays
 * D_8008E914[][8] / D_8008EA00[][4], plus the undefined_syms_auto.txt base rows D_8008EA44 / D_8008EBFC and the
 * alias-suffixed per-word rows D_8008EA45 / D_8008EBFD, are exactly the s2 state (evidence.md s1 OBJECT MODEL, s2
 * SCORER GAP entries). No FAKE constructs, no inline asm, no dead locals. */
void func_800335D8(void) {
    s32 i;
    u8 *tbl = D_8008E914[D_800A36A4];

    for (i = 0; i < 6; i++) {
        if (D_800A3918[i] != 0) {
            s32 cat = func_80033498();
            u8 cur = D_800A3918[i];

            if (cur == 1) {
                func_800325E0(D_8008EBF4[cat], &D_80107850[i].x);
            } else if (cur == D_8008EBFC[cat].a) {
                func_800325E0(D_8008EBF4[cat] + 1, &D_80107850[i].x);
            } else if (cur == D_8008EBFC[cat].b) {
                func_800325E0(D_8008EBF4[cat] + 2, &D_80107850[i].x);
            }

            {
                s32 val = D_800A3918[i] + 1;
                D_800A3918[i] = val;
                if ((u32)D_8008EBFC[cat].b < (u32)(val & 0xFF)) {
                    D_800A3918[i] = 0;
                }
            }
        }
    }

    if (func_8001DB58() != 0) {
        s16 *buf = D_800A3750;

        for (i = 0; i < 4; i++, tbl += 2) {
            u8 *data = tbl + 1;
            s32 rng;
            s32 type;
            s32 rnd;

            rng = rng_Next();
            rnd = rng & 0x3FF;
            type = *tbl;

            if (type == 1) goto handle_type_1;
            if (type < 2) goto skip;
            if (type < 7) goto handle_type_2_6;
            continue;

        handle_type_1:
            if (buf[i] != 0) goto skip;
            buf[i] = 1;
            goto call_default;

        handle_type_2_6:
            {
                s32 adj = type - 2;
                s32 count;
                s32 scount;
                s32 thresh;
                s32 limit;

                count = (u16)buf[i] + 1;
                buf[i] = count;
                scount = (s16)count;
                thresh = D_8008EA44[adj].a;
                limit = thresh * 30;

                if (limit < scount) {
                    s32 ratio = ((scount - limit) << 10) / (D_8008EA44[adj].b * 30);

                    if (rnd < ratio) {
                        buf[i] = 0;
                        type = *tbl;
                        if ((u32)(type - 5) < 2) {
                            func_800325E0(0x7B + i, D_8008EA00[type]);
                            continue;
                        }
                        goto call_default;
                    }
                }
                goto skip;
            }

        call_default:
            {
                u8 val = *data;
                func_8005C650(0x7B + i, val, val);
            }
        skip:;
        }
    }
}
