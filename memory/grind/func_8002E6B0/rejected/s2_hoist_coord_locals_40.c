s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    s32 x0 = arg0[0];
    s32 z0 = arg0[2];
    s32 center_x = (x0 + arg1[0] + arg2[0]) / 3;
    s32 center_z = (z0 + arg1[2] + arg2[2]) / 3;
    s32 px = arg3[0];
    s32 pz = arg3[2];
    s32 cross_center;
    s32 cross_point;

    {
        s32 dz = arg1[2] - z0;
        s32 dx = arg1[0] - x0;
        cross_center = (dz * (center_x - x0)) - (dx * (center_z - z0));
        cross_point = (dz * (px - x0)) - (dx * (pz - z0));
    }
    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }
    {
        s32 dz = arg2[2] - z0;
        s32 dx = arg2[0] - x0;
        cross_center = (dz * (center_x - x0)) - (dx * (center_z - z0));
        cross_point = (dz * (px - x0)) - (dx * (pz - z0));
    }
    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }
    {
        s32 dz = arg2[2] - arg1[2];
        s32 dx = arg2[0] - arg1[0];
        cross_center = (dz * (center_x - arg1[0])) - (dx * (center_z - arg1[2]));
        cross_point = (dz * (px - arg1[0])) - (dx * (pz - arg1[2]));
    }
    return (cross_center ^ cross_point) >= 0;
}
