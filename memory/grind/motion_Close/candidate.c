/*
 * motion_Close — BEST HONEST PURE-C FORM (session 1, recon).
 *
 * Honest sandbox floor with this body: 20 (target 26 insns, build 25).
 * The committed HEAD body scores 21 only because it is built from register
 * pins + a hardcoded `addiu $17, $17, -1` / `jalr %0` inline-asm pair; when the
 * cheat-stripper removes those the loop body vanishes entirely (build_insns 13),
 * so the 21 was an artifact of a degenerate stripped function, not a real
 * measurement of a C form. This file is the first genuine pure-C measurement.
 *
 * STRUCTURE IS AN EXACT MATCH. Instruction-for-instruction, this body produces
 * the target's control-flow graph and operation sequence:
 *
 *   target                          this form
 *   ------                          ---------
 *   lui/lw   t0 <- D_800A2668       lui/lw   v0 <- D_800A2668
 *   addiu sp,sp,-16                 addiu sp,sp,-32          <-- FRAME (see below)
 *   sw s0,4 / s1,8 / ra,12          sw ra,24 / s1,20 / s0,16 <-- SAVE OFFSETS
 *   beqz t0 ; nop                   beqz v0 ; sw s0,16(sp)   <-- delay-slot fill
 *   lui/addiu s0 <- &D_8008D070     lui/addiu s1 <- &D_8008D070
 *   lui/addiu s1 <- &D_00000000     lui/addiu s0 <- &D_00000000
 *   beqz s1 ; nop                   beqz s0 ; nop
 *   lw t0,0(s0) ; addiu s0,s0,4     lw v0,0(s1) ; addiu s1,s1,4
 *   jalr ; addiu s1,s1,-1           jalr ; addiu s0,s0,-1
 *   bnez s1 ; nop                   bnez s0 ; nop
 *   lw ra/s1/s0 ; addiu sp,sp,16    lw ra/s1/s0 ; addiu sp,sp,32
 *   jr ; nop                        jr ; nop
 *
 * The entire residual is (a) the frame size / save-offset block, (b) the
 * s0<->s1 role assignment and the v0-vs-t0 scalar temp, (c) the beqz delay
 * slot. These are EXACTLY the nine regfix rules at regfix.txt:115-124.
 *
 * WHY THE FRAME CANNOT BE REACHED IN PURE C (measured, session 1 — see
 * evidence.md H1). GCC 2.7.2's MIPS o32 backend reserves REG_PARM_STACK_SPACE
 * = 16 bytes of outgoing-argument area for EVERY C-level call, so any pure-C
 * body containing a call has frame >= 16(args) + 12(s0,s1,ra) = 28 -> 32.
 * The target's frame is 16 with ra@12, which back-solves to args+vars = 4,
 * i.e. an outgoing-arg area of ZERO. A census of all 854 call-making functions
 * in the oracle-matching build found ZERO with a sub-16 arg area except
 * motion_Close and its sibling func_80083794 — and both of those only achieve
 * it because their `jalr` is inline asm, which never sets
 * current_function_outgoing_args_size. Do not spend another session trying to
 * shrink this frame from C; the axis is closed.
 *
 * Two independent hand-written-asm signals for the escalation packet:
 *   S-a: no outgoing-arg area despite making a call (unique in the corpus).
 *   S-b: the `beqz` after the register saves has an UNFILLED delay slot (nop)
 *        while a pending `sw s0` was available to fill it — GCC's reorg.c
 *        always fills that slot (this form does). A human wrote the nop.
 * Neighborhood corroborates: _start, InitHeap, func_800836B8 (a raw `.word`
 * BIOS syscall trampoline) and bios_FileRead all sit within a few functions —
 * this is the PsyQ/BIOS crt0 runtime region, and the body is the classic
 * "walk a linker-provided table of function pointers" ctor/dtor loop.
 */

void motion_Close(void) {
    void (**p)(void);
    s32 count;

    if (D_800A2668 != 0) {
        p = &D_8008D070;
        count = (s32)&D_00000000;
        if (count != 0) {
            do {
                void (*f)(void) = *p;
                p++;
                f();
                count--;
            } while (count != 0);
        }
    }
}
