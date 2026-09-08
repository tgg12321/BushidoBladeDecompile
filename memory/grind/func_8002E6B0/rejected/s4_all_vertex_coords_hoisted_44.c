s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    s32 x0 = arg0[0];
    s32 z0 = arg0[2];
    s32 x1 = arg1[0];
    s32 z1 = arg1[2];
    s32 x2 = arg2[0];
    s32 z2 = arg2[2];
    s32 px = arg3[0];
    s32 pz = arg3[2];
    s32 center_x = (x0 + x1 + x2) / 3;
    s32 center_z = (z0 + z1 + z2) / 3;
    s32 cross_center;
    s32 cross_point;

    {
        s32 dz = z1 - z0;
        s32 dx = x1 - x0;
        cross_center = (dz * (center_x - x0)) - (dx * (center_z - z0));
        cross_point = (dz * (px - x0)) - (dx * (pz - z0));
    }
    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }
    {
        s32 dz = z2 - z0;
        s32 dx = x2 - x0;
        cross_center = (dz * (center_x - x0)) - (dx * (center_z - z0));
        cross_point = (dz * (px - x0)) - (dx * (pz - z0));
    }
    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }
    {
        s32 dz = z2 - z1;
        s32 dx = x2 - x1;
        cross_center = (dz * (center_x - x1)) - (dx * (center_z - z1));
        cross_point = (dz * (px - x1)) - (dx * (pz - z1));
    }
    return (cross_center ^ cross_point) >= 0;
}
