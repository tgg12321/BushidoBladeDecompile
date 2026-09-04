/* func_80062020 - ALTERNATIVE DECLARATION (E14), measured in s17 (structural, 2026-09-03).
 * NOT the submitted form.  Equally matching, strictly smaller invented surface: no typedef,
 * no struct tag, no invented member names - just the record shape as a 2-D array bound.
 *
 * include/game.h (in place of the Unk800F1198Record typedef + extern):
 *     extern s32 D_800F1198[][3];
 *
 * MEASURED s17 on the live chassis, in FULL build context (not a harness):
 *   sandbox func_80062020 --disable all -> score 0, target_insns 38, build_insns 38,
 *   scorable true, rules_dropped 0;  verify-oracle --allow-dirty -> ok true,
 *   build_matches true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
 * (The scoring reference had been rebuilt from the record-typedef form; both declarations
 * emit the same relocations against D_800F1198 with in-field addends 0/4/8, so the score
 * is directly comparable.)
 *
 * WHY IT IS BANKED: it proves the target's epilogue arrangement is declaration-independent.
 * The DISP8 | DISP4 | LOSUM0 split comes from the chained assignment on an element of a
 * 3-word record, not from the typedef.  The typedef is therefore not a codegen device.
 */
void func_80062020(s32 *arg0) {
    s32 i;
    s32 ofs;
    s32 t;
    t = *(s32 *)((u8 *)arg0 + 0);
    D_800A32B8 = 0;
    i = 0;
    if ((t & 1) == 0) goto end;
    ofs = 0;
    do {
        D_800F1198[i][0] = *(s32 *)((u8 *)arg0 + ofs + 0);
        D_800F1198[i][1] = *(s32 *)((u8 *)arg0 + ofs + 4);
        D_800F1198[i][2] = *(s32 *)((u8 *)arg0 + ofs + 8);
        i = i + 1;
        ofs = ofs + 12;
        t = *(s32 *)((u8 *)arg0 + ofs + 0);
    } while ((t & 1) != 0);
end:
    D_800F1198[i][0] = D_800F1198[i][1] = D_800F1198[i][2] = 0;
}
