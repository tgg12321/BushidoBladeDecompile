/* REJECTED s1 (grind recon, 2026-07-21): rz-first statement order kept, but
 * rz's inner sum written with the m2c/target addend order (-vx)*sin FIRST:
 *   s32 rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
 * Sandbox cheat-free score = 25 (build 61 insns vs target 62) — a clear
 * regression from the vz*cos-lead 15-form. The addend lead inside rz-first is
 * load-bearing: only vz*cos-lead reaches 15. Do not re-propose.
 */
void cpu_get_dist(s32 *a0, s16 *a1) {
    s32 angle = single_game_getEnemyCharId(a1[0], a1[2]);
    s16 cos_val = *((&Judge) + ((angle + 0x400) & 0xFFF));
    s32 vx = *((s32 *)(((u8 *)a0) + 0x44));
    s16 sin_val = *((&Judge) + (angle & 0xFFF));
    s32 vz = *((s32 *)(((u8 *)a0) + 0x4C));
    s32 rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
    s32 rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
    s32 v48 = *((s32 *)(((u8 *)a0) + 0x48));
    *((s32 *)(((u8 *)a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
    *((s32 *)(((u8 *)a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
    if (v48 < 0) {
        v48 += 3;
    }
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
