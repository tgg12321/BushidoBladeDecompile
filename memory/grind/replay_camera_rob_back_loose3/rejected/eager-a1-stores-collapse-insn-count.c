/* REJECTED -- eager-a1-stores-collapse-insn-count
 *
 * s3: emitting every a1[] store as soon as its element is computable.  73 / 109 insns
 * -- eager stores let combine/CSE eat 5 instructions; the store-blocked cosA shape
 * survives but the body collapses.  Eager sub-groups: cosA singles 13 (inert), the two
 * sinAxsinB sums 79/110, the sinAxcosB pair 53/112.  Only the ONE blocking store belongs
 * out of the tail.
 */
extern s16 Judge[];
void replay_camera_rob_back_loose3(u16 *a0, s16 *a1) {
    s32 angA, angB;
    s16 sinA, sinB, sinC;
    s16 cosB, cosC;
    s32 sinAxsinB_12;
    s32 prod_sinC, prod_cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 cosA_sinB, cosA_sinC, cosA_cosC, cosA_cosB;
    s32 sinAxcosB;
    s32 sinAxcosB_12;
    s32 scb_sinC, neg_sinB_cosC;
    s32 scb_cosC;
    s32 sinB_sinC;
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
    prod_sinC = sinAxsinB_12 * sinC;
    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];
    cosB_cosC = cosB * cosC;
    prod_cosC = sinAxsinB_12 * cosC;
    a1[0] = (prod_sinC + cosB_cosC) >> 12;
    cosB_negsinC = cosB * -sinC;
    a1[1] = (prod_cosC + cosB_negsinC) >> 12;
    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[5] = -sinA;
    cosA = (s16)rawA;
    cosA_sinB = cosA * sinB;
    a1[2] = cosA_sinB >> 12;
    cosA_sinC = cosA * sinC;
    a1[3] = cosA_sinC >> 12;
    sinAxcosB = sinA * cosB;
    cosA_cosC = cosA * cosC;
    a1[4] = cosA_cosC >> 12;
    sinAxcosB_12 = sinAxcosB >> 12;
    scb_sinC = sinAxcosB_12 * sinC;
    neg_sinB_cosC = -sinB * cosC;
    a1[6] = (scb_sinC + neg_sinB_cosC) >> 12;
    scb_cosC = sinAxcosB_12 * cosC;
    sinB_sinC = sinB * sinC;
    a1[7] = (scb_cosC + sinB_sinC) >> 12;
    cosA_cosB = cosA * cosB;
    a1[8] = cosA_cosB >> 12;
}
