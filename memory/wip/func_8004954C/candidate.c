s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        sum += arg0;
        arg0 -= 1;
    }
    return sum + (arg2 - arg1);
}
