/* s3 CHASSIS (score 45, NOT the floor) - kept because it is the only banked body
   whose EXIT STRUCTURE matches the target line-for-line:
     bltz #1 delay slot holds the `<ret> = 0` insn (target: addu $v0,$zero,$zero),
     BOTH bltz branch straight to the epilogue, block 3 falls through with no `j`,
     and there is no orphan `move v0,zero` block at all.
   The ONLY residual on this chassis is the seat: our ret pseudo lands in $a1 and a
   trailing `move v0,a1` survives; the target's ret is $v0 (so no trailing move, and
   block 2 is barred from $v0, which is what forces the target's 6th callee-save $s5).
   v12/candidate.c scores 40 but has structurally WRONG exits (j + orphan block).
   Do not discard this body because 45 > 40 - see hypotheses.md [s3]. */
s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3) {
    s32 center_x = (arg0[0] + arg1[0] + arg2[0]) / 3;
    s32 center_z = (arg0[2] + arg1[2] + arg2[2]) / 3;
    s32 cross_center;
    s32 cross_point;
    s32 ret = 0;

    {
        s32 dz = arg1[2] - arg0[2];
        s32 dx = arg1[0] - arg0[0];
        cross_center = (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
        cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
    }
    if ((cross_center ^ cross_point) < 0) {
        goto end;
    }
    {
        s32 dz = arg2[2] - arg0[2];
        s32 dx = arg2[0] - arg0[0];
        cross_center = (dz * (center_x - arg0[0])) - (dx * (center_z - arg0[2]));
        cross_point = (dz * (arg3[0] - arg0[0])) - (dx * (arg3[2] - arg0[2]));
    }
    if ((cross_center ^ cross_point) < 0) {
        goto end;
    }
    {
        s32 dz = arg2[2] - arg1[2];
        s32 dx = arg2[0] - arg1[0];
        cross_center = (dz * (center_x - arg1[0])) - (dx * (center_z - arg1[2]));
        cross_point = (dz * (arg3[0] - arg1[0])) - (dx * (arg3[2] - arg1[2]));
    }
    ret = (cross_center ^ cross_point) >= 0;
end:
    return ret;
}
