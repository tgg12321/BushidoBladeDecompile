/* REJECTED [s13] — SAME BODY as the match, but the three callees are spelled with
 * symbols that DO NOT EXIST at link time.  `sandbox func_80037A20 --disable all` = 0,
 * 33/33 insns, rules_dropped 0 — and `wteng main build` fails:
 *   undefined reference to `func_80079A30'
 *   undefined reference to `bios_firstfile_B'
 *   undefined reference to `bios_nextfile_B'
 * The sandbox masks relocations, so callee NAMES are invisible to the score.  The
 * correct spellings are `sprintf`, `firstfile`, `nextfile` (declared at
 * src/code6cac_c.c:156-157 and include/code6cac.h:501).  Keep this file as the standing
 * demonstration that sandbox 0 is necessary but not sufficient. */
s32 func_80037A20(s32 arg0, s32 arg1) {
    s32 *var_s0;
    s32 var_s1;
    s32 sp10[8];

    var_s0 = (s32 *)&D_80102810;
    func_80079A30(sp10, (s32)(&g_str_memcard_fmt), arg0, arg1);
    var_s1 = 0;
    if (bios_firstfile_B(sp10, var_s0) != 0) {
        do {
            var_s1++;
            var_s0 = (s32 *)(((u8 *)var_s0) + 0x28);
        } while (bios_nextfile_B(var_s0) != 0);
    }
    D_800A38C8 = var_s1;
    return var_s1;
}
