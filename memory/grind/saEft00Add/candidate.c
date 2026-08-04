/* saEft00Add — CURRENT COMMITTED FORM (src/system.c:1105-1159 @ HEAD 33fe8dd2).
 * This body is PROVEN CORRECT this session (s2, 2026-08-04):
 *   - sandbox --disable all --keep-cheat-asm  => score 0 (133/133), 0 rules dropped
 *   - sandbox --disable all (strip on)        => score 1, and the sole diff is the
 *     %lo(D_80082050) reloc ADDEND (0x1FC4 -> 0x1624), i.e. the static callback's
 *     .text offset shifted 0x9A0 because stripping SIBLING cheat-asm shrinks
 *     func_80080828 (-1408 bytes), tslTm2LoadImage (-1044), marionation_Exec (-12).
 *     No saEft00Add-local edit can move that addend (D_80082050 precedes it in the TU).
 * Previously accepted COMPLETED-C at commit c7114628 (2026-07-11, closer session 13);
 * resurrected into the queue by the post--mel regen (33fe8dd2) solely on this artifact.
 * DO NOT restructure this body to chase sandbox 0 — it is unreachable from this
 * function's C until the three siblings above retire their pins (their own queue items).
 *
 * PsyQ 4.0 LIBCD CDREAD: cd_read_retry (static) — verbatim-linked Sony object
 * (census 2026-07-09); C ref: sotn-decomp psxsdk shape, v1.86 deltas.
 * Volatiles are the granted §3 block view (Ruling-4; see closer/phase3-progress.md
 * session 10: "volatiles all granted §3, de-volatile casts gone"). */
s32 saEft00Add(s32 arg0) {
    u8 sp10;
    s32 temp_s0;
    volatile s32 *tsl;
    volatile s32 *md;

    cdrom_SetCallbackA(0);
    cdrom_SetCallbackB(0);
    tsl = &D_800A1500;
    if (*tsl & 1) {
        tslTmlGetHeda(0);
    }
    if (cdrom_GetMode() & 0x10) {
        if (!(sys_VSync(-1) & 0x3F)) {
            tslTm2LoadImage_2(&D_800162EC);
        }
        func_80080390(1, 0);
        D_800A14EC = sys_VSync(-1);
        D_800A14E4 = -1;
        return D_800A14E4;
    }
    if (arg0 != 0) {
        tslTm2LoadImage_2(&D_80016304);
        func_80080258(9, 0, 0);
        temp_s0 = (s32)func_800800CC();
        if (func_80080258(2, temp_s0, 0) == 0) {
            return D_800A14E4 = -1;
        }
    }
    func_80080148();
    md = &D_800A14DC;
    temp_s0 = *md;
    sp10 = temp_s0;
    temp_s0 = temp_s0 & 0xFF;
    if (temp_s0 != cdrom_GetReadyFlag() || arg0 != 0) {
        if (func_80080258(0xE, (s32)&sp10, 0) == 0) {
            D_800A14E4 = -1;
            return D_800A14E4;
        }
    }
    D_800A14F0 = cdrom_BcdToFrames(func_800800CC());
    cdrom_SetCallbackB((s32)&D_80082050);
    if (D_800A1500 & 1) {
        tslTmlGetHeda((s32)&D_80082320);
    }
    D_800A14D8 = D_800A14D4;
    func_80080390(6, 0);
    {
        extern volatile s32 D_800A14D0; /* CD_sectors scalar view: target reads
            sectors as 2-insn macro-form (0x800825EC lui/lw) */
        D_800A14E4 = D_800A14D0;
    }
    D_800A14E8 = sys_VSync(-1);
    return D_800A14E4;
}
