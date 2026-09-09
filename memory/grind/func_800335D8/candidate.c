/* func_800335D8 -- CANDIDATE (grind s2, 2026-09-08). sandbox --disable all = 0 (176/176), oracle SHA1 proven.
 *
 * Measured THIS session (chassis: -mel -msoft-float, tools/gcc-2.7.2/build/cc1), edits in place in src/ +
 * include/code6cac.h + undefined_syms_auto.txt + named_syms.txt (the integration-handoff scope grant, 2026-09-08):
 *   sandbox func_800335D8 --disable all = 0, 176/176 insns, rules_dropped 0   (tmp/grind/func_800335D8/s2/sandbox_K3_nocast.txt)
 *   verify-oracle --rebuild --allow-dirty: ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
 *                                                                              (tmp/grind/func_800335D8/s2/verify_oracle_K3_full.txt)
 *
 * What closed the s1 "scorer floor" of 6: engine/score.py _symtab() resolves %hi/%lo pairs only through the three
 * ld symbol files (undefined_funcs_auto.txt, undefined_syms_auto.txt, named_syms.txt). cc1 spells the second byte of a
 * threshold record as %lo(D_8008EBFC+1) / %lo(D_8008EA44+1); the splat asm spells it %lo(D_8008EBFD) / (D_8008EA45).
 * Both link to the same word (oracle SHA1), but the scorer could not see it because the record BASE symbols existed
 * only as dlabels in asm/data/7D920.data.s. Adding the base symbols' address rows to undefined_syms_auto.txt
 * (D_8008EA44 = 0x8008EA44; and D_8008EBFC = 0x8008EBFC; -- the same address the dlabel already defines, the same
 * "row + dlabel" coexistence D_8008EBFD / D_800A3918 / D_80107850 already have) lets the scorer link both spellings to
 * @lo(0xebfd) / @lo(0xea45): 6 -> 0. Ablation this session (rows removed, nothing else changed): 6 again
 * (s2/sandbox_K2_no_base_rows.txt). The per-word rows D_8008EA45 / D_8008EBFD STAY, each suffixed with the prong (c)
 * amendment-2026-09-03 alias comment ("alias of <base>+1; retire with func_800335D8"): asm/funcs/func_800335D8.s
 * (the sandbox's reference side, assembled into build/src/code6cac_b.o) is their only referrer; no C names them.
 *
 * s2 body change vs the s1-banked K2: (s32 *)&D_80107850[i] -> &D_80107850[i].x at the three func_800325E0 calls
 * (the callee takes s32 * to a 3-word position; this passes the record's first coordinate without a cast).
 * Byte-identical: sandbox 0 both ways, oracle SHA1 both ways (s2/verify_oracle_K2_rows_full.txt, s2/verify_oracle_K3_full.txt).
 *
 * DECLARATIONS (in place; identical to tmp/grind/func_800335D8/s1/integration_surfaces.patch except the sym rows):
 *   include/code6cac.h : typedef struct { u8 a; u8 b; } LeafThreshold;  extern LeafThreshold D_8008EA44[5];
 *                        extern u8 D_8008EBF4[6];  extern LeafThreshold D_8008EBFC[6];   (D_8008EA45 / D_8008EBFD decls removed)
 *   src/code6cac_b.c   : extern u8 D_8008E914[][8]; extern s32 D_8008EA00[][4]; extern s16 D_800A3750[4];
 *   named_syms.txt     : g_leaf_action_threshold_b (0x8008EBFD) and g_threshold_lookup_EA45 rows removed
 *   undefined_syms_auto.txt : + D_8008EA44 / D_8008EBFC base rows; D_8008EA45 / D_8008EBFD kept with the alias suffix.
 *
 * Object model: see memory/grind/func_800335D8/evidence.md (OBJECT MODEL entry, s1). No FAKE constructs, no inline asm,
 * no dead locals. */
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
