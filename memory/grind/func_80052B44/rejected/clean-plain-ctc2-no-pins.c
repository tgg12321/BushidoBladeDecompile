/* REJECTED — clean pure-C form, no pins, no rule. Honest floor 13 (re-measured
 * s1 2026-07-27: sandbox --disable all = 13, build 16 vs target 14).
 * Why dead (two independent mechanisms, both measured/pinned):
 * 1. Register rotation: target loads matrix[0..4] into $t0-$t4 (regs 8-12);
 *    GCC's ascending allocator preference picks $v0/$v1/$a1/$a2/$a0 — the
 *    OPPOSITE bias; no pure-C lever forces higher regs (pins are forbidden and
 *    score-inert). Also GCC orders the base-aliasing lw last, adding a
 *    load-delay nop.
 * 2. jr-ra delay slot: target ends `jr ra; ctc2 $zero,$7`. ctc2 can only enter
 *    compilation as implicitly-volatile inline asm (no C form), and GCC 2.7.2
 *    reorg.c can NEVER fill a delay slot with an asm insn: stop_search_p
 *    (reorg.c:730-735) halts the candidate search at ASM_INPUT/asm_noperands
 *    insns, and mark_referenced_resources (reorg.c:366-367) marks asm
 *    always-volatile so resource_conflicts_p (reorg.c:751) always conflicts.
 *    The last two target instructions are unreachable by compilation —
 *    pure-C distance can never reach 0. This is a no-C-form function.
 */
void func_80052B44(s32 *matrix) {
    s32 t0, t1, t2, t3, t4;
    t0 = matrix[0];
    t1 = matrix[1];
    t2 = matrix[2];
    t3 = matrix[3];
    t4 = matrix[4];
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("ctc2 $0, $5");
    __asm__ volatile ("ctc2 $0, $6");
    __asm__ volatile ("ctc2 $0, $7");
}
