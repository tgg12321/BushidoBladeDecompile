/* s7 REJECTED — "third-parameter $a2 occupancy": the FIRST form in this grind
 * that reproduces BOTH of target's residual register names at once
 * (`addu $a3,$a1,$zero` in the bnez delay slot AND the D_80101E62 address in
 * $t0) — and still scores WORSE than the 13 floor.
 *
 * Measured (sandbox --disable all, s7):
 *   vf3u (this shape, a2 consumed at the E68 store)      15 / 39
 *   w5   (same + *pe62 = 2 hoisted above the E7C store)  14 / 39   <- basin best
 *   w6   (same + EC38 load first)                        14 / 39
 *   w3   (same + each load followed by its store)        14 / 38
 *   w1   (a2 consumed at the E9E store instead)          16 / 39
 *   w7   (E60 store below the loads)                     17 / 39
 *   candidate.c (2 params, floor)                        13 / 38
 *
 * WHY IT IS DEAD, on two independent grounds:
 *  (1) SEMANTICS.  Target stores a literal zero to D_80101E68 (`sh $zero`).
 *      The third parameter here exists only so that hard reg $a2 is occupied;
 *      its consumer is a fabricated store.  That is a dead-value coercion by
 *      any spelling, and the asm call sites pass two arguments.
 *  (2) IT DOES NOT PAY.  The register names come free, but the occupancy of
 *      $a1 (which is what denies the a1-parm allocno its own $a1) forces
 *      cam_val into $a1, and its D_80101E6C store then schedules to the very
 *      END of the function instead of immediately after the loads.  Net: the
 *      three register names we gain cost more than they save.
 *
 * KEEP the forensic content (s7 H24/H25/H27 in hypotheses.md): this form is
 * the measurement that refutes s6 H22's "impossible by construction" and that
 * shows target blocks $a1 and $a2 WITHOUT allocating anything to them.
 */
s32 replay_camera_Init(s32 a0, s32 a1, s32 a2) {
    s32 sval;
    s16 *pe62 = &D_80101E62;
    s32 *pe70 = &D_80101E70;
    s32 reloaded;

    if (*pe62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E6C = cam_val;
        D_80101E70 = ec_val;
    }
    reloaded = *pe70;
    *pe62 = 2;
    D_80101E7C = a1;
    D_80101E68 = a2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
