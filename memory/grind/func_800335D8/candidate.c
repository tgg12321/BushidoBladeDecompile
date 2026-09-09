/* func_800335D8 -- BANKED CANDIDATE (grind s1, 2026-09-08). BYTES PROVEN, integration surface pending.
 *
 * Measured this session (chassis: -mel -msoft-float, tools/gcc-2.7.2/build/cc1):
 *   sandbox --disable all = 6 (176/176 insns) -- ALL SIX points are a scorer artifact, see below.
 *   verify-oracle --rebuild --allow-dirty with this body + the declaration changes below applied:
 *   build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_800335D8/s1/verify_oracle_J_full.txt).
 *   Reference-normalized diff vs target (tmp/grind/func_800335D8/s1/sdiffbodyI3.txt): the only lines that differ are
 *   the three lui/lbu pairs reading the second byte of a threshold pair, which cc1 emits as %hi/%lo(D_8008EBFC+1) /
 *   %hi/%lo(D_8008EA44+1) while the splat asm spells them %hi/%lo(D_8008EBFD) / (D_8008EA45). They link to the same
 *   words (the oracle build proves it); engine/score.py cannot resolve them because D_8008EBFC / D_8008EA44 are defined
 *   only as dlabels in asm/data/7D920.data.s, which is not in cfg.LD_SYM_FILES (score.py _symtab), so the pair stays raw.
 *
 * REQUIRED DECLARATION CHANGES (exact patch: tmp/grind/func_800335D8/s1/integration_surfaces.patch):
 *   include/code6cac.h : replace `extern u8 D_8008EA44; extern u8 D_8008EA45;` with
 *                        typedef struct { u8 a; u8 b; } LeafThreshold;  extern LeafThreshold D_8008EA44[5];
 *                        replace `extern u8 D_8008EBF4; extern u8 D_8008EBFC; extern u8 D_8008EBFD;` with
 *                        extern u8 D_8008EBF4[6];  extern LeafThreshold D_8008EBFC[6];
 *   src/code6cac_b.c   : TU externs become `extern u8 D_8008E914[][8]; extern s32 D_8008EA00[][4]; extern s16 D_800A3750[4];`
 *   undefined_syms_auto.txt : delete `D_8008EA45 = 0x8008EA45;` (line 37) and `D_8008EBFD = 0x8008EBFD;` (line 42)
 *   named_syms.txt          : delete the g_leaf_action_threshold_b (0x8008EBFD) and g_threshold_lookup_EA45 rows
 *   (aggregate-merge prong (c): no C consumer of D_8008EBFD / D_8008EA45 exists; no other asm/funcs/*.s references them.)
 *
 * Object model (evidence in memory/grind/func_800335D8/evidence.md, OBJECT MODEL entry): the original binary reads
 * D_8008EBFC[cat*2] and D_8008EBFD[cat*2] at the same index, i.e. a 6-entry table of 2-byte {a,b} threshold records
 * (rodata 8008EBFC: 2E 5C 2E 62 2E 62 2E 2E 32 62 2E 62); D_8008EA44/45 is the same 2-byte record shape for types 2..6
 * (8008EA44: 06 09 0F 0F 0A 0D 0A 0A 05 0F). D_8008EBF4 is a 6-entry u8 table of base action ids padded to the next word.
 * D_8008E914 is a per-stage row of 4 {type,param} byte pairs (8-byte stride, indexed by D_800A36A4); D_8008EA00 is the
 * 16-byte-stride record table passed to func_800325E0 (census); D_800A3750 is the 4-entry s16 per-slot counter buffer
 * (loop bound base+8).
 *
 * No FAKE constructs, no inline asm, no dead locals. */
void func_800335D8(void) {
    s32 i;
    u8 *tbl = D_8008E914[D_800A36A4];

    for (i = 0; i < 6; i++) {
        if (D_800A3918[i] != 0) {
            s32 cat = func_80033498();
            u8 cur = D_800A3918[i];

            if (cur == 1) {
                func_800325E0(D_8008EBF4[cat], (s32 *)&D_80107850[i]);
            } else if (cur == D_8008EBFC[cat].a) {
                func_800325E0(D_8008EBF4[cat] + 1, (s32 *)&D_80107850[i]);
            } else if (cur == D_8008EBFC[cat].b) {
                func_800325E0(D_8008EBF4[cat] + 2, (s32 *)&D_80107850[i]);
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
