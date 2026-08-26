/* s7 (2026-08-26, structural). MEASUREMENT PROBE, not a proposal.
 *
 * func_800372F4 widened to (nbytes, buf, mode) -- which is what the target's two
 * call sites require -- while the body keeps calling the header's stale 1-arg
 * CdRead. buf and mode are therefore never read in the callee. This is exactly
 * the shape the layer-1 cheat-reviewer FAILed on 2026-08-26 01:09
 * (rejected/layer1-fail-0826-0109.c) and it is banked here ONLY for the number
 * it produced, which is the load-bearing fact:
 *
 *     sandbox special_camera_get_rot_dir --disable all -> score 0, 72/72
 *     sandbox func_800372F4              --disable all -> score 0, 21/21
 *
 * Identical to the variant that forwards all three arguments to a correctly
 * declared 3-arg CdRead. CONCLUSION: the CdRead prototype's arity contributes
 * ZERO bytes to either function. The wrapper's incoming $a1/$a2 are already the
 * outgoing $a1/$a2, so forwarding them is a register identity and GCC emits
 * nothing for it.
 *
 * This is what converts the remaining question from a codegen problem into a
 * pure SOURCE-FIDELITY problem: no C spelling can be selected on byte evidence,
 * because every spelling produces the same bytes. The only discriminator left is
 * which one is the original source, and the answer (forward the arguments to a
 * correctly declared CdRead, fixing include/code6cac.h:510) needs a scope grant.
 * See docs/grind/decisions.md, 2026-08-26 s7 decision packet.
 *
 * DO NOT RE-PROPOSE. Unread parameters are a standing layer-1 FAIL for this
 * function; the number above is the only thing worth inheriting.
 */

s32 func_800372F4(s32 nbytes, s32 buf, s32 mode) {
    s32 v = nbytes;
    nbytes += 0x7FF;
    if (nbytes < 0) {
        nbytes = v + 0xFFE;
    }
    CdRead(nbytes >> 11);   /* buf, mode never read -- layer-1 FAIL shape */
    do {
        v = CdReadSync(1, 0);
        if (v > 0) {
            VSync(0);
        }
    } while (v > 0);
    return v;
}
