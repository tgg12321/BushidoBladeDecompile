void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {
    s32 mat[8];
    s32 vec[3];
    s32 m0;
    s32 m1;
    s32 m2;
    s32 src;
    *(s16 *)(arg0 + 0x350) = 1;
    *(s16 *)(arg0 + 0x352) = *(u16 *)((u8 *)&D_8008D59E + arg1 * 20);
    {
        s32 new_var;
        new_var = game_GetPlayerData(*(s16 *)(arg0 + 4));
        src = *(s32 *)((((s32)*(s16 *)(arg0 + 0x352)) << 2) + new_var);
    }
    func_8002EECC(src, mat);
    {
        register s32 *mp asm("$12");
        register s32 t5 asm("$13");
        register s32 t6 asm("$14");
        register s32 t7 asm("$15");
        s32 *p = mat;
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(p));
        t5 = *mp;
        t6 = *(mp + 1);
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $1" :: "r"(t6));
        t5 = *(mp + 2);
        t6 = *(mp + 3);
        t7 = *(mp + 4);
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $3" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }
    vec[0] = arg2[0];
    vec[1] = arg2[1];
    vec[2] = arg2[2];
    {
        register s32 *vp asm("$12");
        register s32 t5 asm("$13");
        register s32 t6 asm("$14");
        s32 *p = vec;
        __asm__ volatile ("move %0, %1" : "=r"(vp) : "r"(p));
        t6 = ((u16 *)vp)[2];
        t5 = ((u16 *)vp)[0];
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile ("mtc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("lwc2 $1, 8(%0)" :: "r"(vp));
        __asm__ volatile ("nop");
        __asm__ volatile ("nop");
        __asm__ volatile (".word 0x4A486012");
    }
    arg0 += 0x354;
    {
        register u8 *outp asm("$12");
        u8 *p = arg0;
        __asm__ volatile ("move %0, %1" : "=r"(outp) : "r"(p));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(outp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(outp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(outp));
    }
}
