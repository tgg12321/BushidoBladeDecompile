/* REJECTED s2: staging the store expressions through the dead vx/vz locals
 * (v9) = 26, regression from 15. Also killed the same round: v48 >>= 2 as a
 * separate statement (v11) = 17; v48-load placement between rz/rx or between
 * the stores = neutral 15; natural-order statement position rx-before-rz in
 * the 15-load-order context (v8) = 27.
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
    rz = -(((vz * cos_val) + ((-vx) * sin_val)) >> 12);
    rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
    v48 = *((s32 *)(((u8 *)a0) + 0x48));
    vx = ((rx * cos_val) - (rz * sin_val)) >> 15;
    *((s32 *)(((u8 *)a0) + 0x44)) = vx;
    vz = ((rx * sin_val) + (rz * cos_val)) >> 15;
    *((s32 *)(((u8 *)a0) + 0x4C)) = vz;
    if (v48 < 0) {
        v48 += 3;
    }
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
