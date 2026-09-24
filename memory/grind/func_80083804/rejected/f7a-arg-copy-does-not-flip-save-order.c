/*
 * REJECTED (session 8) — F7a's conditional disproof is now UNCONDITIONAL, and
 * the mechanism session 6 named as NECESSARY is measured INSUFFICIENT too.
 *
 * BACKGROUND. The target saves in the order `sw $s0,4($sp)` / `sw $s1,8($sp)` /
 * `sw $ra,12($sp)` — ASCENDING register number. Our honest build emits
 * `sw $ra,24($sp)` / `sw $s1,20($sp)` / `sw $s0,16($sp)` — DESCENDING. (The
 * offset-to-register MAPPING is the same in both: ra highest, then s1, then s0.
 * Only the order of the store instructions in the stream differs.) Session 6
 * killed this as F7a with the finding that flipping the order needs a WAR
 * anti-dependence INSIDE the prologue basic block — in practice an incoming
 * argument copy `move sN,aM` — which a void(void) function cannot have. That
 * disproof was CONDITIONAL on the signature, and frontier F10 named it as the
 * one residual mechanism a TU-level reshape might reach.
 *
 * PROBE 1 (empirical) — tmp/grind/motion_Close/s8/paramsweep.py gave
 * motion_Close incoming parameters on purpose (an ABI lie; these forms are
 * never submittable — they exist only to answer the mechanism question) and
 * dumped the emitted prologue with the instrumented cc1:
 *
 *   w1_param_p                13  param dead -> optimised away, prologue
 *                                 identical to the void(void) form
 *   w2_param_count            13  ditto
 *   w3_param_both_overwritten 13  ditto
 *   w5_param_p_used           19  sw s1,20 / move s1,a0 / sw ra,24 / beqz /
 *                                 sw s0,16      <- ra displaced, order still
 *                                                  s1-before-s0
 *   w4_param_both_used        21  sw s1,20 / move s1,a0 / sw s0,16 /
 *                                 move s0,a1 / beqz / sw ra,24
 *                                               <- BOTH WAR anti-deps present,
 *                                                  saves STILL descending
 *
 * w4 is the decisive cell. It has exactly the construct session 6 identified as
 * the missing ingredient — two genuine `move sN,aM` incoming-argument copies,
 * each WAR-dependent on the save above it, inside the prologue block. The block
 * scheduler duly reacts: it interleaves the copies between the saves and pushes
 * `sw $ra` out of first position into the beqz delay slot. But the s0/s1 saves
 * remain in DESCENDING order. The anti-dependence changes what the saves are
 * interleaved WITH; it does not change their relative order.
 *
 * PROBE 2 (backend) — the reason is not schedulability, it is emission.
 * `save_restore_insns` (tools/gcc-2.7.2/config/mips/mips.c:4680) is
 *
 *     for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)
 *
 * with `gp_offset` initialised to `current_frame_info.gp_sp_offset` (the TOP of
 * the save area) and decremented by UNITS_PER_WORD after each emitted store
 * (mips.c:4710). The loop direction is unconditional — no target flag, no
 * frame-pointer case, no ABI case, nothing a C source can reach selects the
 * other direction; the comment at mips.c:4609-4612 states the high-to-low order
 * is a deliberate debugger/epilogue convention. So the SAVES ARE ALWAYS EMITTED
 * ra, s1, s0, and only a block-local scheduling move can reorder them. The
 * scheduler breaks ties between two structurally symmetric stores by program
 * order, which IS the descending emission order; w4 measures that it does so
 * even when both stores carry an anti-dependent consumer.
 *
 * VERDICT. F7a is KILLED unconditionally, at the same evidence tier as session
 * 7's H1 kill: the ascending save order is not a property of any C input to
 * this compiler. Both halves of F10 (signature reshape, TU reshape) are closed.
 */

/* w4_param_both_used — the disproof cell, NOT a proposal. Its parameters are a
   lie about motion_Close's ABI (it is a crt0 destructor-table walker called
   with no arguments) and it scores 21, worse than the floor of 13. */

void motion_Close(void (**p)(void), s32 count) {
    if (D_800A2668 != 0) {
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
