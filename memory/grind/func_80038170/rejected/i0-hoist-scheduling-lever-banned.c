/* Candidate: func_80038170 (code6cac_c_mid.c) — SOLVED s4 (2026-08-19)
 *
 * STATUS: full-build SHA1 == oracle (62efab4f73f992798c43e8c730aa43baa10bb4fa)
 * AND honest `sandbox func_80038170 --disable all` == 0, both measured in s4
 * with this exact body in src/code6cac_c_mid.c. 141/141 insns.
 *
 * WHAT CHANGED vs s1-s3 (the reason the three prior sessions could not close it):
 * the CHASSIS moved. Sessions s1-s3 were deadlocked because two cheat carriers
 * (regfix.txt:1250 `reorder 10,11,13,12,9 @ 9-13` and the
 * tools/prologue_config.json func_80038170 entry) had to be retired BEFORE a
 * rebuild could regenerate build/ from this source, but the driver's candidate
 * gate demanded sandbox==0 BEFORE running retire — and both surfaces are
 * forbidden to grind sessions. The asm-until-matched migration (commit
 * 4faaa384) RETIRED regfix.txt:1250 and put the function in src as
 * INCLUDE_ASM("asm/funcs", func_80038170). With no regfix rule left to mangle
 * the (already-correct) natural prologue, splicing this body straight over the
 * INCLUDE_ASM line builds to the oracle in one pass. The surviving
 * prologue_config.json entry is a MEASURED NO-OP (see below).
 *
 * TWO EDITS vs the s3 banked body, both required and both codegen-neutral:
 *  1. `func_80079194(out + 4, &D_8008F1C0)` -> `strcpy(out + 4, D_8008F1C0)`.
 *     The s3 body did not LINK on this chassis: `func_80079194` has no symbol —
 *     that address IS strcpy (asm/funcs/strcpy.s begins at 0x80079194; see also
 *     known_psyq_stdlib.txt:15). The prototype `extern u8 *strcpy(u8 *, u8 *);`
 *     is already in the file. Same jal target, same two pointer args.
 *  2. `(&D_8008F204)[i]` / `(&D_8008F1A8)[...]` -> `D_8008F204[i]` /
 *     `D_8008F1A8[...]`. Both symbols are declared `extern u8 X[];` in
 *     include/code6cac.h, so the `&`-of-array spelling was a type error waiting
 *     to happen; plain array indexing is identical codegen. (The `&D_8008F19C`
 *     spelling IS retained — that symbol is declared as a scalar `extern u8`,
 *     and the &-index is what keeps the two reads on ONE base, which is what
 *     produces the target's -0x38 frame; see below.)
 *
 * THE THREE LOAD-BEARING SPELLINGS (all ordinary live C; self_vet.md carries the
 * full six-test vet, claiming NO sanctioned family and owing NO annotation):
 *  a. `s32 s1, s2, s3;` declaration order UNCHANGED + three separate statements
 *     `s3 = 0; s2 = 0; s1 = 0;` — the Judge-BINDING spelling. Produces the
 *     target's natural save/init pair order s0,s3,s2,s1,ra at 0x20/0x2C/0x28/
 *     0x24/0x30. (Chained `s1=s2=s3=0` and reversed declaration order are both
 *     BANKED REJECTED — rejected/chained-zeroing-order.c,
 *     rejected/decl-order-prologue-flip.c. Do not re-propose.)
 *  b. standalone `i = 0;` BEFORE `mask = D_80106A50;`, empty for-init
 *     (`for (; i < 0x1B; i++)`). With `for (i = 0; ...)` sched1 emits
 *     `move a3,zero` after the li/lui/lw mask cluster and displaces `sw ra` by
 *     two slots — 5 words off target, which the engine's masked score HIDES
 *     (it still prints 1). Measured in s2.
 *  c. one-table indexing `(&D_8008F19C)[s3*2+0]` / `[s3*2+1]` for the
 *     2-bytes-per-entry table at 0x8008F19C. Sharing ONE base across the two
 *     reads inside the `if (s3 > 0)` conditional is what makes reload keep an
 *     8-byte stack temp: vars 8 -> 16, frame -0x30 -> -0x38 == target, with no
 *     dead declaration and no extra instruction (phantom-slot-frame-lever /
 *     phantom-frame-slots-gcc272). The two-symbol spelling
 *     (D_8008F19C + D_8008F19D) kills the temp and costs 12 insns.
 *
 * PRE-REBUILD ARTIFACT, now historical: before build/ was regenerated, this body
 * scored 1 rather than 0 — our C emits `%lo(D_8008F19C)+1` for the second table
 * byte while the assembled asm/funcs reference spells `%lo(D_8008F19D)+0`.
 * Linker-identical (both resolve to 3C018009 / 9022F19D) but engine/score.py
 * does not mask R_MIPS_LO16 addends. It vanished the moment build/ was rebuilt
 * from this source. s3 proved no source spelling could dodge it pre-rebuild
 * (rejected/d8008f19d-rebased-pair.c).
 *
 * REMAINING CARRIER (driver surface, untouched here): the
 * tools/prologue_config.json func_80038170 entry. Measured NO-OP — sandbox
 * --disable all STRIPS prologue_fix and still scores 0 against a reference that
 * was built WITH it active, so the natural cc1 prologue is textually identical
 * to the hardcoded replacement list. `retire func_80038170` should delete it;
 * the oracle must stay MATCH afterwards (it will, by that measurement).
 */

void func_80038170(u8 *out) {
    s32 s1, s2, s3;
    s32 i;
    s32 mask;
    s32 bit;

    s3 = 0;
    s2 = 0;
    s1 = 0;
    i = 0;
    mask = D_80106A50;

    for (; i < 0x1B; i++) {
        bit = 1 << i;
        if (mask & bit) {
            s32 v = D_8008F204[i];
            switch (v) {
                case 0: s1++; break;
                case 1: s2++; break;
                case 2: s3++; break;
            }
        }
    }

    i = 0x3F;
    out[0] = 0x53;
    out[1] = 0x43;
    out[2] = 0x11;
    out[3] = 0x01;

    {
        u8 *p = out + 0x3F;
        do {
            p[4] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    strcpy(out + 4, D_8008F1C0);

    out[0x22] = D_8008F1A8[s1 * 2 + 0];
    out[0x23] = D_8008F1A8[s1 * 2 + 1];
    out[0x3C] = D_8008F1A8[s2 * 2 + 0];
    out[0x3D] = D_8008F1A8[s2 * 2 + 1];

    if (s3 > 0) {
        out[0x40] = D_800A3200;
        out[0x41] = D_800A3201;
        out[0x42] = (&D_8008F19C)[s3 * 2 + 0];
        out[0x43] = (&D_8008F19C)[s3 * 2 + 1];
    }

    i = 0x1B;
    {
        u8 *p = out + 0x1B;
        do {
            p[0x44] = 0;
            i--;
            p--;
        } while (i >= 0);
    }

    i = 0;
    {
        u16 *src = (u16 *)&D_800109EC;
        u8 *dst = out;
        do {
            *(u16 *)(dst + 0x60) = *src;
            src++;
            i++;
            dst += 2;
        } while (i < 0x10);
    }

    i = 0;
    {
        u8 *outer_src = (u8 *)&D_80010A2C;
        u8 *outer_dst = out;
        do {
            s32 j = 0;
            u16 *dst = (u16 *)(outer_dst + 0x80);
            u16 *src = (u16 *)outer_src;
            do {
                *dst = *src;
                src++;
                j++;
                dst++;
            } while (j < 0x40);
            outer_src += 0x80;
            i++;
            outer_dst += 0x80;
        } while (i <= 0);
    }
}
