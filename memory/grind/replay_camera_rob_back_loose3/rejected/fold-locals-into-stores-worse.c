/* REJECTED -- fold-locals-into-stores-worse
 *
 * s3: every single-use intermediate folded into its consumer (12 locals removed).
 * 87 / 116 insns.  Folding only the four cosA_* products (n1) is 71 / 111.  Reducing the
 * pseudo population makes the schedule worse, not the allocation better.
 */
extern s16 Judge[];
void replay_camera_rob_back_loose3(u16 *a0, s16 *a1) {
    s32 angA, angB;
    s16 sinA, sinB, sinC;
    s16 cosB, cosC;
    s32 sinAxsinB_12;
    s32 sinAxcosB_12;
    s32 cosA;
    u16 rawA;
    s32 angC;

    angA = a0[0];
    angB = a0[1];
    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];
    angC = a0[2];
    sinC = Judge[angC & 0xFFF];
    sinAxsinB_12 = (sinA * sinB) >> 12;
    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];
    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[5] = -sinA;
    cosA = (s16)rawA;
    sinAxcosB_12 = (sinA * cosB) >> 12;
    a1[2] = (cosA * sinB) >> 12;
    a1[3] = (cosA * sinC) >> 12;
    a1[4] = (cosA * cosC) >> 12;
    a1[8] = (cosA * cosB) >> 12;
    a1[0] = ((sinAxsinB_12 * sinC) + (cosB * cosC)) >> 12;
    a1[1] = ((sinAxsinB_12 * cosC) + (cosB * -sinC)) >> 12;
    a1[6] = ((sinAxcosB_12 * sinC) + (-sinB * cosC)) >> 12;
    a1[7] = ((sinAxcosB_12 * cosC) + (sinB * sinC)) >> 12;
}
