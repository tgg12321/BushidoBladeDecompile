s16 *camera_CalcAngles(void) {
    SVECTOR rot;
    VECTOR sp18;
    MATRIX pos;
    s16 s0;

    math_RotMatrixYXZ((s32 *)&D_800A3708->xf.rot, (s32 *)&pos);
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(&pos, &rot, &sp18);
    s0 = ratan2(sp18.vx, sp18.vz);
    sp18.vz = ((s32)Judge[(s0 + 0x400) & 0xFFF] * sp18.vz
              + (s32)Judge[s0 & 0xFFF] * sp18.vx) >> 12;
    D_800A33C8[0] = -ratan2(sp18.vy, sp18.vz);
    D_800A33C8[1] = s0;
    return D_800A33C8;
}
