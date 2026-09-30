void func_80077724(s32 arg0, s32 arg1) {
    S7724 s;
    s32 *p;
    s32 temp_v1;
    SELWORK->f24 = (s32 *)(((D_800A36AC & 1) * 0x4090) + (s32)&g_gpu_db);
    temp_v1 = SELWORK->f30 + 1;
    SELWORK->f34 = SELWORK->f34 + 1;
    SELWORK->f30 = temp_v1;
    p = func_80077098(temp_v1 & 1);
    SELWORK->f2C = p;
    s.sp10 = (s32)SELWORK->f04;
    s.sp14 = p[0];
    s.sp18 = p[1];
    s.sp1C = p[2];
    s.sp20 = p[4];
    s.sp24 = p[3];
    s.sp28 = p[5];
    s.sp2C = p[6];
    s.sp30 = p[7];
    s.sp34 = p[8];
    func_80077374(arg1, &s.sp10);
}
