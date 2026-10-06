/* _SendPAD (0x80079000, LIBAPI SENDPAD, src/main/psxsdk/libapi/sendpad.c). Best honest C:
 * sandbox --disable all = 4 of 10 (2026-10-03), operand-only: GCC loads the pointer into $v0 and
 * saves $ra at 0x10(sp); the original uses $t1 and 0x14(sp) (frame 0x18 either way, the load
 * already ahead of the frame). cc1psx-check: the original cc1psx scores the same 4.
 * Also 4: pass-through (int a0..a3) returning the call; an int global cast to a function
 * pointer at the call; a local copy of the pointer.
 * 2026-10-05 manual (fresh eyes): unused `int pad[1]` / `char buf[4]` / plain unused local = 4 (dropped);
 * `volatile int pad` (FAKE probe: the bare call plus an unused `volatile int pad;`, re-scored 6), void or 4-arg pass-through = 6 but frame 0x20, $ra 0x18 (GCC rounds
 * var_size to 8: $ra = args 16 + var 8 + 4 - 4), never 0x18/0x14. In 2.7.2's compute_frame_size $ra at
 * 0x14 of 0x18 with nothing else saved needs args_size 20 (a 5th stored arg) or a second saved reg -
 * both visible; $t1 needs $v0..$t0 all occupied. Neither is reachable from honest C.
 * VERDICT: hand-written. PsyQ 4.0 LIBAPI.A (ECOFF, sozud/psy-q) names this module's source `sendpad.s`
 * (includes r3000.h/asm.h, no gcc2_compiled.); its C mate send.c (SendPAD -> _SendPAD) saves $ra at
 * 0x10. LIBAPI.LIB SN section order: all 75 .s modules (incl. SENDPAD) .text-first, all 15 .c .rdata-first.
 * Reproduce: python memory/grind/_SendPAD/asm_evidence.py */
extern void (*D_800A362C)(void);

void _SendPAD(void) {
    D_800A362C();
}
