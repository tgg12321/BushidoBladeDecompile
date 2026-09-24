s32 func_8005763C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 *arg8, s32 *arg9) {
    s32 slope1;
    s32 x1;
    s32 dx1;
    s32 dy1;
    s32 dx2;
    s32 dy2;
    s32 slope2;
    s32 intercept1;
    s32 intersection_x;
    s32 y;

    x1 = arg0;
    x1 >>= 3;
    arg2 >>= 3;
    arg1 >>= 3;
    arg3 >>= 3;
    dx1 = arg2 - x1;
    dy1 = arg3 - arg1;
    arg4 >>= 3;
    arg6 >>= 3;
    arg5 >>= 3;
    arg7 >>= 3;
    dx2 = arg6 - arg4;
    dy2 = arg7 - arg5;
    if ((arg4 == arg6) && (arg1 == arg3)) {
        *arg8 = arg6;
        *arg9 = arg3;
        goto check_bounds;
    }
    if ((x1 == arg2) && (arg5 == arg7)) {
        *arg8 = arg2;
        *arg9 = arg7;
        goto check_bounds;
    }
    if ((arg4 == arg6) && (x1 != arg2)) {
        *arg8 = arg6;
        *arg9 = arg1 + (dy1 * (arg6 - x1)) / dx1;
        goto check_bounds;
    }
    if ((x1 == arg2) && (arg4 != arg6)) {
        *arg8 = x1;
        *arg9 = arg5 + (dy2 * (x1 - arg4)) / dx2;
        goto check_bounds;
    }
    if ((arg1 == arg3) && (arg5 != arg7)) {
        *arg9 = arg3;
        *arg8 = arg4 + (dx2 * (arg3 - arg5)) / dy2;
        goto check_bounds;
    }
    if ((arg5 == arg7) && (arg1 != arg3)) {
        *arg9 = arg5;
        *arg8 = x1 + (dx1 * (arg5 - arg1)) / dy1;
        goto check_bounds;
    }
    if (arg2 == x1) {
        return 0;
    }
    {
        if (arg6 != arg4) {
            slope1 = (dy1 << 7) / dx1;
            slope2 = (dy2 << 7) / dx2;
            if (slope1 != slope2) {
                intercept1 = (((arg1 * arg2) - (arg3 * x1)) << 7) / dx1;
                intersection_x = (((((arg5 * arg6) - (arg7 * arg4)) << 7) / dx2) - intercept1) / (slope1 - slope2);
                *arg8 = intersection_x;
                *arg9 = ((intersection_x * slope1) + intercept1) >> 7;
check_bounds:
                if ((((*arg8 - x1) >= -50) || (((*arg8 - arg2) < -50) == 0)) &&
                    (((x1 - *arg8) >= -50) || (((arg2 - *arg8) < -50) == 0)) &&
                    ((y = *arg9, (((arg1 - y) < -50) == 0)) || (((arg3 - y) < -50) == 0)) &&
                    (((y - arg1) >= -50) || (((y - arg3) < -50) == 0)) &&
                    (((*arg8 - arg4) >= -50) || (((*arg8 - arg6) < -50) == 0)) &&
                    (((arg4 - *arg8) >= -50) || (((arg6 - *arg8) < -50) == 0)) &&
                    (((arg5 - y) >= -50) || (((arg7 - y) < -50) == 0))) {
                    if ((y - arg5) >= -50) {
                        goto intersection_found;
                    }
                    if ((y - arg7) >= -50) {
                        goto intersection_found;
                    }
                }
                goto no_intersection;
            }
        }
no_intersection:
        return 0;
    }
intersection_found:
    *arg8 = *arg8 << 3;
    *arg9 = *arg9 << 3;
    return 1;
}
