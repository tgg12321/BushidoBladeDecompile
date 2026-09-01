/* CANDIDATE — func_8002FC80 (src/code6cac_b.c) — session s2-of-run3, 2026-09-01.
 *
 * FORMAT CHANGE (deliberate): this file is now the FUNCTION BODY ONLY, not a
 * full-file snapshot of src/code6cac_b.c.  The previous full-file snapshot was a
 * stale-image trap — func_8002D320 landed as C in the same TU after the snapshot
 * was taken, so copying candidate.c over src silently reverted it.  Apply this
 * file by REPLACING the single line
 *     INCLUDE_ASM("asm/funcs", func_8002FC80);
 * in src/code6cac_b.c with the body below, and adding `#include "gte.h"` to the
 * TU's include block (immediately before "code6cac.h") — gte.h supplies VECTOR
 * and is not otherwise included by that file.
 *
 * MEASURED THIS SESSION with exactly this text in src:
 *   sandbox func_8002FC80 --disable all -> score 0, target_insns 74,
 *   build_insns 74, rules_dropped 0  (tmp/grind/func_8002FC80/s2/
 *   s2run3_sandbox_vector_form_0.txt)
 *   verify-oracle -> ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa
 *   == oracle, build_matches true (s2run3_verify_oracle.txt)
 *
 * This is the THIRD byte-exact spelling class found for this function and the
 * first one that contains no pointer cast on the load side at all: the three
 * parameters are VECTOR *, the components are read as ->vx/->vy/->vz, and the
 * two scratchpad destinations are the same VECTOR type.  See evidence.md
 * s2-of-run3 for why the two earlier classes (u8*-param widening casts; s32*
 * params with VECTOR-cast stores) are not needed and are not used here.
 */

s32 func_8002FC80(VECTOR *a0, VECTOR *a1, VECTOR *a2) {
    VECTOR *p;
    s32 ret;

    /* Difference vectors: (a1 - a0) into the scratchpad VECTOR at
     * SCR[0x60..0x68], (a2 - a0) into the one at SCR[0x70..0x78] — the two
     * operands the GTE macros below read back (same slots as func_8002FDB0). */
    ((VECTOR *)0x1F800360)->vx = a1->vx - a0->vx;
    ((VECTOR *)0x1F800360)->vy = a1->vy - a0->vy;
    ((VECTOR *)0x1F800360)->vz = a1->vz - a0->vz;
    ((VECTOR *)0x1F800370)->vx = a2->vx - a0->vx;
    ((VECTOR *)0x1F800370)->vy = a2->vy - a0->vy;
    ((VECTOR *)0x1F800370)->vz = a2->vz - a0->vz;

    /* PsyQ libgte inline macro gte_SetRotMatrix(r) — loads the 3 packed
     * rotation-matrix words at r into cop2 control regs R11R12/R13R21/R22R23.
     * The SDK macro body hardcodes $12-$15 and copies the operand into $12. */
    __asm__ volatile(
        "move   $12, %0
"
        "lw     $13, 0($12)
"
        "lw     $14, 4($12)
"
        "ctc2   $13, $0
"
        "lw     $15, 8($12)
"
        "ctc2   $14, $2
"
        "ctc2   $15, $4
"
        :: "r"((VECTOR *)0x1F800360) : "$12", "$13", "$14", "$15");
    /* PsyQ libgte inline macro gte_ldlvl(r) — load long vector at r into
     * IR1/IR2/IR3 ($9/$10/$11), IR3 first, then the 2-cycle GTE load delay. */
    __asm__ volatile(
        "move   $12, %0
"
        "lwc2   $11, 8($12)
"
        "lwc2   $9, 0($12)
"
        "lwc2   $10, 4($12)
"
        "nop
"
        "nop
"
        :: "r"((VECTOR *)0x1F800370) : "$12");
    /* GTE OP (outer/cross product of the IR vector with the rotation matrix
     * diagonal), sf=0 — cop2 command 0x0170000C. */
    __asm__ volatile(".word 0x4B70000C");
    /* PsyQ libgte inline macro gte_stlvnl(r) — store MAC1/MAC2/MAC3
     * ($25/$26/$27) to r. */
    p = (VECTOR *)0x1F800380;
    __asm__ volatile(
        "move   $12, %0
"
        "swc2   $25, 0($12)
"
        "swc2   $26, 4($12)
"
        "swc2   $27, 8($12)
"
        :: "r"(p) : "$12");
    /* Angle of the cross product in the XZ plane, +0x800 (180 deg) when its
     * Y component is positive. */
    ret = ratan2(p->vx, p->vz);
    if (p->vy > 0) {
        ret += 0x800;
    }
    return ret;
}
