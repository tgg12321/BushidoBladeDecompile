/* saEft00Add = cd_read_retry (LIBCD/CDREAD, PsyQ 4.0) — faithful Sony shape
 * candidate on the volatile CdlREAD struct chassis.
 * MEASURED 2026-07-09: 81 differing EXE words vs target (whole function
 * reshuffled) — the early-return chassis diverges badly under our fork;
 * HEAD's goto-end chassis is far closer (honest floor 18 at HEAD).
 * All-volatile members also let cross-jump merge the three cnt=-1 exit
 * tails (-28 bytes). Next avenue: HEAD goto-end chassis + struct members
 * with Sony's three DISTINCT exit spellings (stmt/stmt/expr-return, per
 * sotn cdread.c lines 71-73, 79, 87-88) to keep the tails apart.
 */
s32 saEft00Add(s32 arg0) {
    u8 mode;
    s32 mode2;

    cdrom_SetCallbackA(0);
    cdrom_SetCallbackB(0);
    if (D_800A14D0.tslmode & 1) {
        tslTmlGetHeda(0);
    }
    if (cdrom_GetMode() & 0x10) {
        if (!(sys_VSync(-1) & 0x3F)) {
            tslTm2LoadImage_2(&D_800162EC);
        }
        func_80080390(1, 0);
        D_800A14D0.t1 = sys_VSync(-1);
        D_800A14D0.cnt = -1;
        return D_800A14D0.cnt;
    }
    if (arg0 != 0) {
        tslTm2LoadImage_2(&D_80016304);
        func_80080258(9, 0, 0);
        if (func_80080258(2, (s32)func_800800CC(), 0) == 0) {
            return D_800A14D0.cnt = -1;
        }
    }
    func_80080148();
    mode2 = D_800A14D0.mode;
    mode = mode2;
    if ((mode2 & 0xFF) != cdrom_GetReadyFlag() || arg0 != 0) {
        if (func_80080258(0xE, (s32)&mode, 0) == 0) {
            D_800A14D0.cnt = -1;
            return D_800A14D0.cnt;
        }
    }
    D_800A14D0.pos = cdrom_BcdToFrames(func_800800CC());
    cdrom_SetCallbackB((s32)&D_80082050);
    if (D_800A14D0.tslmode & 1) {
        tslTmlGetHeda((s32)&D_80082320);
    }
    D_800A14D0.p = D_800A14D0.buf;
    func_80080390(6, 0);
    D_800A14D0.cnt = D_800A14D0.sectors;
    D_800A14D0.t2 = sys_VSync(-1);
    return D_800A14D0.cnt;
}
