/* func_80061250 — candidate. STATE BANNER (s3, 2026-08-26, dispatched as
 * "session 1" of the post-discard renumbering; HEAD ecc1e876):
 * main's committed representation is INCLUDE_ASM("asm/funcs", func_80061250)
 * (asm-until-matched); this file is the LEDGER copy of the best form, NOT a
 * claim about the committed tree. The s3 working-tree probe was applied,
 * measured, and REVERTED before session end.
 *
 * SPELLING CHANGE vs the layer-1-FAILed s2 form: the banned pointer-pun
 * `(&D_800F1154)[5]/[6]` off a single-byte `extern u8 D_800F1154;` is GONE.
 * This form declares the object as an ARRAY — `extern u8 D_800F1154[];`
 * (block-scope here; final placement pending owner ruling, see below) — and
 * indexes it: `D_800F1154[5]`, `&D_800F1154[5]`. This mirrors the committed,
 * integrated sibling func_800619F0 (src/text1b.c:3557-3566 at HEAD ecc1e876:
 * same `extern u8 D_800F1154[];`, publishes `D_800F1154 + 3`), and the naming
 * census models 0x800F1154 as g_text1b_glyph_buf_b — a buffer/aggregate
 * (named_syms.txt:827).
 *
 * MEASURED s3 (2026-08-26): with this exact body applied over the INCLUDE_ASM
 * at src/text1b.c:3271-3272, `sandbox func_80061250 --disable all` == 0
 * (59/59, rules_dropped 0, nothing cheat-stripped from this body). The ARRAY
 * spelling produces the same (const (plus D_800F1154 5)) cse anchor as the
 * banned pun, hence identical bytes; full-build SHA1 == oracle was proven for
 * the identical-RTL pun form in s1-rerun (62efab4f73f992798c43e8c730aa43baa10bb4fa).
 *
 * PENDING: ruling-request filed s3 — whether this TU-consistent array-extern
 * spelling is acceptable as ordinary C (option A), or the full 5-prong
 * per-word-splat aggregate merge (no-new-park-categories.md:215-232) is
 * required first (option B: shared-header canonical decl, sibling respells,
 * splat-config symbol removal at integration). Do NOT submit candidate-ready
 * until the ruling lands; two layer-1 FAILs are already banked on this
 * function.
 *
 * 0 regfix/asmfix rules, 0 cheat-asm, 0 volatile, 0 FAKE constructs.
 * Body mirrors COMPLETED-C sibling func_8006156C (Match 450f69d1, Judge PASS
 * 2026-07-22): same control shape, same *p++ tail; only data differs
 * (flags [5]/[6] vs [1]/[2], codes 0x210009/A vs 0x210005/6, mask 0xFF0060
 * vs 0xFF8080). */
void func_80061250(s32 *arg0) {
    extern u8 D_800F1154[];
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if (D_800F1154[5] != 0) {
        if (D_800F1154[6] != 0) {
            D_800F1154[6] = 0;
            D_800F1154[5] = 0;
        }
        if (D_800F1154[5] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)&D_800F1154[5];
    *(s32 *)D_800A3468 = 0x210009;
    goto end;
check_one_zero:
    if (D_800F1154[6] == 0) {
        D_800F1180 = (s32)&D_800F1154[6];
        *v1 = 0x21000A;
    }
end:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0xFF0060;
}
