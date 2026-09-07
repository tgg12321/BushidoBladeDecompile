/* REJECTED -- s14 (2026-09-07, synthesis).  score 13, build_insns 93 (baseline, unchanged).
 * FORM: `do { lim = 0xC8; } while (0);` wrapped INSIDE the main loop body, as an attempt to
 * arm move_movables' moved_once doubling (loop.c:1609-1611) at ZERO emitted cost -- the
 * do-while(0) family is owner-sanctioned for any codegen effect, so if it created a scannable
 * inner loop it would have been the free version of s10's dead arming loop.
 * MEASURED KILL: it does create a NOTE_INSN_LOOP_BEG (the .loop dump prints
 *   "Loop from 18 to 32 is phony."), but loop.c REJECTS it before scan_loop does any work:
 *   at tools/gcc-2.7.2/loop.c:568-575 scan_loop returns immediately unless `scan_start` is a
 *   CODE_LABEL.  For a do-while(0) the back edge is gone, so jump1 deletes the unreferenced
 *   top label and scan_start is a plain insn.  No movable is registered, moved_once is never
 *   written, and the main loop's dump is bit-identical to the baseline (all four switch
 *   comparison constants still `moved to` the pre-header).
 * Three placements measured, all identical (13 / 93 / insn_count 91): wrapping `lim = 0xC8;`
 * alone, wrapping `lim = 0xC8; p = base + i * 2;`, and wrapping `p = base + i * 2;` alone.
 * This is a CLASS kill for the do-while(0) arming route (predicate: loop.c:570).
 */
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        do {
            lim = 0xC8;
        } while (0);
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
