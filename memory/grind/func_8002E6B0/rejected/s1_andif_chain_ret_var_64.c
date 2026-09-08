s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    s32 center_x = (arg0[0] + arg1[0] + arg2[0]) / 3;
    s32 center_z = (arg0[2] + arg1[2] + arg2[2]) / 3;
    s32 ret;

    ret = (((arg1[2] - arg0[2]) * (center_x - arg0[0]) - (arg1[0] - arg0[0]) * (center_z - arg0[2]))
           ^ ((arg1[2] - arg0[2]) * (arg3[0] - arg0[0]) - (arg1[0] - arg0[0]) * (arg3[2] - arg0[2]))) >= 0
       && (((arg2[2] - arg0[2]) * (center_x - arg0[0]) - (arg2[0] - arg0[0]) * (center_z - arg0[2]))
           ^ ((arg2[2] - arg0[2]) * (arg3[0] - arg0[0]) - (arg2[0] - arg0[0]) * (arg3[2] - arg0[2]))) >= 0
       && (((arg2[2] - arg1[2]) * (center_x - arg1[0]) - (arg2[0] - arg1[0]) * (center_z - arg1[2]))
           ^ ((arg2[2] - arg1[2]) * (arg3[0] - arg1[0]) - (arg2[0] - arg1[0]) * (arg3[2] - arg1[2]))) >= 0;
    return ret;
}
