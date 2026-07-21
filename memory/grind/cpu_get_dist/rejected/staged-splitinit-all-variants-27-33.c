/* REJECTED s2: same-variable split-init staging (var=a; var+=b; var>>=12)
 * regresses in BOTH algebraic spaces — natural order (late or early vz) = 27,
 * 15-form staged rz = 33, staged both = 31. Full-staging is a DEAD lever class
 * for this function (frontier hypothesis #2 "staged forms deepen lh cos's
 * successor chain" KILLED by measurement: staging shortens the loads' shared
 * chains and worsens the schedule). Partial splits (neg-only, shift-only) are
 * score-NEUTRAL (15). Representative form below = v3_15form_staged_both (31).
 */
void cpu_get_dist(s32 *a0, s16 *a1) {
    s32 angle;
    s16 cos_val;
    s16 sin_val;
    s32 vx;
    s32 vz;
    s32 rx;
    s32 rz;
    s32 v48;
    angle = single_game_getEnemyCharId(a1[0], a1[2]);
    cos_val = *((&Judge) + ((angle + 0x400) & 0xFFF));
    vx = *((s32 *)(((u8 *)a0) + 0x44));
    sin_val = *((&Judge) + (angle & 0xFFF));
    vz = *((s32 *)(((u8 *)a0) + 0x4C));
    rz = vz * cos_val;
    rz += (-vx) * sin_val;
    rz >>= 12;
    rz = -rz;
    rx = vx * cos_val;
    rx += vz * sin_val;
    rx >>= 12;
    v48 = *((s32 *)(((u8 *)a0) + 0x48));
    *((s32 *)(((u8 *)a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
    *((s32 *)(((u8 *)a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
    if (v48 < 0) {
        v48 += 3;
    }
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
