/* RULING-BLOCKED CANDIDATES (Closer phase-3 session 3, 2026-07-10)
 *
 * Both measured sandbox --disable all == 0 AND full-build SHA1 == oracle
 * with these bodies in place (in-context, this session). Reverted to HEAD
 * pending an owner ruling because layer-1 cheat-reviewer FAILed the
 * unwritten-element-inside-a-used-volatile-array construct (see
 * memory/closer/sotn-volatile-delay-array-research-2026-07-10.md and the
 * ruling question in the session outcome).
 *
 * If the ruling sanctions the pattern: re-apply both bodies verbatim
 * (func_80082A14 in src/ings2.c — also DELETE its asm-barrier HEAD body;
 * func_80078E58 in src/text1b_b.c), re-measure (expected 0/0), full build,
 * layer-2 review, done.
 */

/* PsyQ 4.0 LIBETC VSYNC: v_wait (static) — verbatim-linked Sony object
   (census 2026-07-09); C ref: sotn-decomp src/main/psxsdk/libetc/vsync.c
   (SOTN ships `volatile s32 timeout[2]` with only [0] referenced).
   Replaces HEAD's asm volatile("" ::: "memory") barrier (cheat-asm). */
void func_80082A14(s32 a0, s32 a1) {
    volatile s32 timeout[2];

    timeout[0] = a1 << 0xF;
    while (g_sys_dma_region < a0) {
        if (timeout[0]-- == 0) {
            tslTm2LoadImage_2(&D_80016318);
            bios_ChangeClearPad(0);
            bios_ChangeClearRCnt(3, 0);
            return;
        }
    }
}

/* PsyQ 4.0 LIBAPI PAD: _Pad1 (static) — verbatim-linked Sony object.
   Replaces HEAD's forbidden unwritten `s32 pad[2]` with the same
   volatile-delay-counter-array idiom (frame 16 = i[3], only [0] used). */
s32 func_80078E58(void) {
    volatile s32 i[3];
    *(s16 *)((u8 *)D_8009BD84 + 0xA) = 0;
    i[0] = 10;
    i[0] = i[0] - 1;
    if (i[0] != -1) {
        do {
            i[0] = i[0] - 1;
        } while (i[0] != -1);
    }
    return 0;
}
