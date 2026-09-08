s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    s32 center_x = (arg0[0] + arg1[0] + arg2[0]) / 3;
    s32 center_z = (arg0[2] + arg1[2] + arg2[2]) / 3;
    s32 cross_center;
    s32 cross_point;

    {
        s32 dz = arg1[2] - arg0[2];
        s32 dx = arg1[0] - arg0[0];
        s32 cx = center_x - arg0[0];
        s32 cz = center_z - arg0[2];
        s32 pxd = arg3[0] - arg0[0];
        s32 pzd = arg3[2] - arg0[2];
        cross_center = (dz * cx) - (dx * cz);
        cross_point = (dz * pxd) - (dx * pzd);
    }
    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }
    {
        s32 dz = arg2[2] - arg0[2];
        s32 dx = arg2[0] - arg0[0];
        s32 cx = center_x - arg0[0];
        s32 cz = center_z - arg0[2];
        s32 pxd = arg3[0] - arg0[0];
        s32 pzd = arg3[2] - arg0[2];
        cross_center = (dz * cx) - (dx * cz);
        cross_point = (dz * pxd) - (dx * pzd);
    }
    if ((cross_center ^ cross_point) < 0) {
        return 0;
    }
    {
        s32 dz = arg2[2] - arg1[2];
        s32 dx = arg2[0] - arg1[0];
        s32 cx = center_x - arg1[0];
        s32 cz = center_z - arg1[2];
        s32 pxd = arg3[0] - arg1[0];
        s32 pzd = arg3[2] - arg1[2];
        cross_center = (dz * cx) - (dx * cz);
        cross_point = (dz * pxd) - (dx * pzd);
    }
    return (cross_center ^ cross_point) >= 0;
}
