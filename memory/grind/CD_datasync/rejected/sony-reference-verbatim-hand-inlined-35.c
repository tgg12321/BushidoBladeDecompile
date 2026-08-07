/* KILLED (s7, 35/91).  The Sony PsyQ LIBCD `CD_datasync` reference source
 * (Xeeynamo/sotn-decomp src/main/psxsdk/libcd/bios.c) hand-inlined verbatim
 * into BB2's symbols.  Scores 35 against the inherited candidate's 8, though
 * it does hit target's exact 91 instructions.
 *
 * WHY IT LOSES: with a real loop, loop.c hoists BOTH loop-invariant compare
 * constants (0x3C0000, 0x1000000) into callee-saves -- `lui s3,0x3c` and
 * `lui s2,0x100` appear in the prologue -- so five callee-saves are live and
 * the param lands in $s1 instead of target's $s2.  It also fails to hoist a
 * base register for D_800A1494 (Intr): with no explicit pointer local GCC
 * folds the constant address into each `lbu` as %hi/%lo, where target uses
 * `lbu a0,0(s1)` / `lbu v0,1(s1)` off a hoisted base.
 *
 * Variants in the same family, all worse: r1 (+ the three table-base locals)
 * 37/93; r2 (set_alarm/get_alarm as real `static __inline__` helpers) 31/94.
 * Do NOT re-transcribe the reference literally -- see
 * memory/grind/saEft01Init/ref/sotn_libcd_bios_CD_datasync.c for the mapping
 * and for what the reference IS good for. */
/* PsyQ 4.0 LIBCD BIOS: CD_datasync — verbatim-linked Sony object (census
 * 2026-07-09); C ref: Xeeynamo/sotn-decomp src/main/psxsdk/libcd/bios.c
 * (CD_datasync + the static inline set_alarm/get_alarm helpers, hand-inlined). */
s32 saEft01Init(s32 a0) {
    s32 ret;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

    while (1) {
        if (D_800F19B8 < sys_VSync(-1) || D_800F19BC++ > 0x3C0000) {
            tslTm2LoadImage_2(&D_800161B8);
            debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5],
                         D_800A125C[(&D_800A1494)[0]],
                         D_800A125C[(&D_800A1494)[1]]);
            cdrom_ClearIrq();
            ret = -1;
            break;
        }
        if (!(*D_800A14C0 & 0x1000000)) {
            ret = 0;
            break;
        }
        if (a0 != 0) {
            ret = 1;
            break;
        }
    }
    return ret;
}
