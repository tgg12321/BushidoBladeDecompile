/* REJECTED s2: the do-while(0) fence ALONE (HEAD's wrap span, dead statement
 * removed) = 27 — far WORSE than the unwrapped 15-form baseline. The fence and
 * a dead-store bias must combine to reach <=8; neither suffices alone. Also
 * killed in the same wrap-placement sweep: 15-form algebra inside the wrap
 * (w2=27, y8=26), wrap starting at vz (w3=22), at rx (w4=22), full-body wrap
 * (w5=21), empty wrap between vx and sin (w6=23), sequential second wrap around
 * cos (r2=20), if-outside-wrap (z8=12), sin-outside-wrap-at-vx (r3=10),
 * vx-after-sin inside wrap (z2=8 vs y4's 6), dead store before sin (y7=21,
 * r4=25), dead store after the vz load reading initialized vz (x1=27, z5=25),
 * pivot dead-cos as a simple move rx=cos_val (t1=6, t2 without second dead=25),
 * variable-reuse cosidx/vx single-var forms (q1=10, q2=12, q3=13, q4=13).
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
    do {
        sin_val = *((&Judge) + (angle & 0xFFF));
        vz = *((s32 *)(((u8 *)a0) + 0x4C));
        rx = ((vx * cos_val) + (vz * sin_val)) >> 12;
        rz = -((((-vx) * sin_val) + (vz * cos_val)) >> 12);
        v48 = *((s32 *)(((u8 *)a0) + 0x48));
        *((s32 *)(((u8 *)a0) + 0x44)) = ((rx * cos_val) - (rz * sin_val)) >> 15;
        *((s32 *)(((u8 *)a0) + 0x4C)) = ((rx * sin_val) + (rz * cos_val)) >> 15;
        if (v48 < 0) {
            v48 += 3;
        }
    } while (0);
    *((s32 *)(((u8 *)a0) + 0x48)) = v48 >> 2;
}
