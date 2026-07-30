/* REJECTED -- decl-order-whole-block-inert
 *
 * s3: the ENTIRE local declaration block reversed (s2 had only permuted angC's line).
 * 13 / 114, bit-identical.  Also inert: all locals collapsed into one group (d2) and
 * declaration order == first-use order (d3).  Declaration order does not reach
 * local-alloc here even though it renumbers every pseudo (verified in the .lreg dumps:
 * pseudo 188 vs 192 for the same quantity, same final assignment).
 */
extern s16 Judge[];
void replay_camera_rob_back_loose3(u16 *a0, s16 *a1) {
    s32 angC;
    u16 rawA;
    s32 cosA;
    s32 sinB_sinC;
    s32 scb_cosC;
    s32 scb_sinC, neg_sinB_cosC;
    s32 sinAxcosB_12;
    s32 sinAxcosB;
    s32 cosA_sinB, cosA_sinC, cosA_cosC, cosA_cosB;
    s32 cosB_cosC, cosB_negsinC;
    s32 prod_sinC, prod_cosC;
    s32 sinAxsinB_12;
    s16 cosB, cosC;
    s16 sinA, sinB, sinC;
    s32 angA, angB;

    angA = a0[0];
    angB = a0[1];
    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];
    angC = a0[2];
    sinC = Judge[angC & 0xFFF];
    sinAxsinB_12 = (sinA * sinB) >> 12;
    prod_sinC = sinAxsinB_12 * sinC;
    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];
    cosB_cosC = cosB * cosC;
    prod_cosC = sinAxsinB_12 * cosC;
    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[5] = -sinA;
    cosA = (s16)rawA;
    cosB_negsinC = cosB * -sinC;
    cosA_sinB = cosA * sinB;
    cosA_sinC = cosA * sinC;
    sinAxcosB = sinA * cosB;
    cosA_cosC = cosA * cosC;
    sinAxcosB_12 = sinAxcosB >> 12;
    scb_sinC = sinAxcosB_12 * sinC;
    neg_sinB_cosC = -sinB * cosC;
    scb_cosC = sinAxcosB_12 * cosC;
    cosA_cosB = cosA * cosB;
    a1[2] = cosA_sinB >> 12;
    a1[3] = cosA_sinC >> 12;
    a1[4] = cosA_cosC >> 12;
    a1[8] = cosA_cosB >> 12;
    a1[0] = (prod_sinC + cosB_cosC) >> 12;
    a1[1] = (prod_cosC + cosB_negsinC) >> 12;
    a1[6] = (scb_sinC + neg_sinB_cosC) >> 12;
    sinB_sinC = sinB * sinC;
    a1[7] = (scb_cosC + sinB_sinC) >> 12;
}
