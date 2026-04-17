#include "common.h"

s32 pad_main_control(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    register s32 result asm("v0");
    s32 x0 = arg0[0];
    s32 x1 = arg1[0];
    s32 x2 = arg2[0];
    s32 z0 = arg0[2];
    s32 z1 = arg1[2];
    s32 z2 = arg2[2];
    s32 center_x = (x0 + x1 + x2) / 3;
    s32 center_z = (z0 + z1 + z2) / 3;
    s32 dz;
    s32 dx;
    s32 center_cross;
    s32 point_cross;

    dz = z1 - z0;
    center_cross = dz * (center_x - x0);
    point_cross = dz * (arg3[0] - x0);
    dx = x1 - x0;
    point_cross -= dx * (arg3[2] - z0);
    center_cross -= dx * (center_z - z0);
    result = 0;
    if ((center_cross ^ point_cross) < 0) {
        goto end;
    }

    dz = z2 - z0;
    center_cross = dz * (center_x - x0);
    point_cross = dz * (arg3[0] - x0);
    dx = x2 - x0;
    point_cross -= dx * (arg3[2] - z0);
    center_cross -= dx * (center_z - z0);
    if ((center_cross ^ point_cross) < 0) {
        goto end;
    }

    dz = z2 - z1;
    center_cross = dz * (center_x - x1);
    point_cross = dz * (arg3[0] - x1);
    dx = x2 - x1;
    point_cross -= dx * (arg3[2] - z1);
    center_cross -= dx * (center_z - z1);
    result = (center_cross ^ point_cross) >= 0;

end:
    return result;
}
