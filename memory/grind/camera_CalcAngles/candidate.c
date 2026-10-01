s16 *camera_CalcAngles(void) {
    s16 rot[3];
    s32 sp18[3];
    s32 pos[8];
    s16 s0;

    math_RotMatrixYXZ((s32 *)&D_800A3708->xf.rot, pos);
    rot[0] = 0;
    rot[1] = 0;
    rot[2] = 0x1000;
    ApplyMatrix((MATRIX *)pos, (SVECTOR *)rot, (VECTOR *)sp18);
    s0 = ratan2(sp18[0], sp18[2]);
    sp18[2] = ((s32)Judge[((s16)s0 + 0x400) & 0xFFF] * sp18[2]
              + (s32)Judge[s0 & 0xFFF] * sp18[0]) >> 12;
    D_800A33C8[0] = -ratan2(sp18[1], sp18[2]);
    D_800A33C8[1] = s0;
    return D_800A33C8;
}
