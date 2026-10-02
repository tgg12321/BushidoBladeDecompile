s16 *camera_CalcAngles(void) {
    SVECTOR rot;
    VECTOR dir;
    MATRIX mtx;
    s16 yaw;

    math_RotMatrixYXZ(&D_800A3708->xf.rot, &mtx);
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(&mtx, &rot, &dir);
    yaw = ratan2(dir.vx, dir.vz);
    dir.vz = ((s32)Judge[(yaw + 0x400) & 0xFFF] * dir.vz + (s32)Judge[yaw & 0xFFF] * dir.vx) >> 12;
    D_800A33C8[0] = -ratan2(dir.vy, dir.vz);
    D_800A33C8[1] = yaw;
    return D_800A33C8;
}
