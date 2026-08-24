void func_8002EBDC(s16 *arg0, s16 *arg1, s32 *arg2, s32 arg3, s32 arg4) {
    s32 sp_tmp;
    u8 *scr = (u8 *)0x1F8002B8;
    s32 *mat;
    s32 charId1;
    u32 dist_sq;
    u32 volume;
    s32 charId2;
    s32 scaled_x, scaled_y, scaled_z;
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");

    mat = (s32 *)(scr + 0xD8);

    charId1 = ratan2(arg1[0], arg1[2]);
    *(s16 *)0x1F8003B2 = (s16)(0x800 - charId1);

    {
        s32 dx = arg1[0];
        s32 dz = arg1[2];
        dist_sq = (u32)(dx * dx + dz * dz);
    }
    if (dist_sq < 0x400U) {
        volume = (u32)(u8)(*(((u8 *)&D_8008D118) + dist_sq)) >> 3;
    } else {
        u32 v1_sqrt = 0;
        if ((s32)dist_sq >= 0) {
            register s32 t4_v asm("$12");
            register s32 v0_pin asm("$2");
            t4_v = (s32)dist_sq;
            __asm__ volatile (".word 0x488CF000" :: "r"(t4_v));
            __asm__ volatile ("nop");
            __asm__ volatile ("nop");
            v0_pin = (s32)&sp_tmp;
            __asm__ volatile ("move %0, %1" : "=r"(t4_v) : "r"(v0_pin));
            __asm__ volatile (".word 0xE99F0000" :: "r"(t4_v));
            v1_sqrt = (u32)sp_tmp;
        }
        {
            u32 clz = v1_sqrt;
            u32 v0_m = clz & (u32)-2;
            u32 v1_m = 0x16 - v0_m;
            u32 idx = dist_sq >> v1_m;
            u32 hi = (u32)(u8)(*(((u8 *)&D_8008D118) + idx));
            do { v0_m = 0x13 - (v1_m >> 1); } while (0);
            volume = (hi << 16) >> v0_m;
        }
    }

    charId2 = ratan2(arg1[1], (s32)volume);
    *(s16 *)(scr + 0xF8) = (s16)(0x800 - charId2);
    *(s16 *)(scr + 0xD8) = 0x1000;
    *(s16 *)(scr + 0xDA) = 0;
    *(s16 *)(scr + 0xDC) = 0;
    *(s16 *)(scr + 0xDE) = 0;
    *(s16 *)(scr + 0xE0) = 0x1000;
    *(s16 *)(scr + 0xE2) = 0;
    *(s16 *)(scr + 0xE4) = 0;
    *(s16 *)(scr + 0xE6) = 0;
    *(s16 *)(scr + 0xE8) = 0x1000;
    RotMatrixY(*(s16 *)(scr + 0xFA), mat);
    RotMatrixX(*(s16 *)(scr + 0xF8), mat);

    /* First MVMVA: rotate arg0's vec via composed rotation */
    {
        register s32 *mp asm("$12");
        s32 *src = mat;
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(src));
        t5 = mp[0];
        t6 = mp[1];
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $1" :: "r"(t6));
        t5 = mp[2];
        t6 = mp[3];
        t7 = mp[4];
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $3" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }
    {
        register s32 *vp asm("$12");
        s32 *src = (s32 *)arg0;
        __asm__ volatile ("move %0, %1" : "=r"(vp) : "r"(src));
        t6 = ((u16 *)vp)[2];
        t5 = ((u16 *)vp)[0];
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile ("mtc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("lwc2 $1, 8(%0)" :: "r"(vp));
    }
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");
    {
        register s32 *rp asm("$12");
        s32 *dst = (s32 *)(scr + 0xA8);
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(dst));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }

    /* Scale by arg3 (z) and arg4 (x, y), with sign-correction /256 */
    {
        s32 z_in = *(s32 *)(scr + 0xB0) * arg3;
        if (z_in < 0) z_in += 0xFF;
        *(s32 *)(scr + 0xB0) = z_in >> 8;
        scaled_z = z_in >> 8;
    }
    {
        s32 x_in = *(s32 *)(scr + 0xA8) * arg4;
        if (x_in < 0) x_in += 0xFF;
        *(s32 *)(scr + 0xA8) = x_in >> 8;
        scaled_x = x_in >> 8;
    }
    {
        s32 y_in = *(s32 *)(scr + 0xAC) * arg4;
        if (y_in < 0) y_in += 0xFF;
        *(s32 *)(scr + 0xAC) = y_in >> 8;
        scaled_y = y_in >> 8;
    }
    (void)scaled_x;
    (void)scaled_y;
    (void)scaled_z;

    /* Rebuild identity matrix, apply INVERSE rotation (negated angles) */
    *(s16 *)(scr + 0xD8) = 0x1000;
    *(s16 *)(scr + 0xDA) = 0;
    *(s16 *)(scr + 0xDC) = 0;
    *(s16 *)(scr + 0xDE) = 0;
    *(s16 *)(scr + 0xE0) = 0x1000;
    *(s16 *)(scr + 0xE2) = 0;
    *(s16 *)(scr + 0xE4) = 0;
    *(s16 *)(scr + 0xE6) = 0;
    *(s16 *)(scr + 0xE8) = 0x1000;
    RotMatrixX(-(s32)*(s16 *)(scr + 0xF8), mat);
    RotMatrixY(-(s32)*(s16 *)(scr + 0xFA), mat);

    /* Second MVMVA: inverse-rotate the scaled vec, store to arg2 */
    {
        register s32 *mp asm("$12");
        s32 *src = mat;
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(src));
        t5 = mp[0];
        t6 = mp[1];
        __asm__ volatile ("ctc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $1" :: "r"(t6));
        t5 = mp[2];
        t6 = mp[3];
        t7 = mp[4];
        __asm__ volatile ("ctc2 %0, $2" :: "r"(t5));
        __asm__ volatile ("ctc2 %0, $3" :: "r"(t6));
        __asm__ volatile ("ctc2 %0, $4" :: "r"(t7));
    }
    {
        register s32 *vp asm("$12");
        s32 *src = (s32 *)(scr + 0xA8);
        __asm__ volatile ("move %0, %1" : "=r"(vp) : "r"(src));
        t6 = ((u16 *)vp)[2];
        t5 = ((u16 *)vp)[0];
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile ("mtc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("lwc2 $1, 8(%0)" :: "r"(vp));
    }
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");
    {
        register s32 *rp asm("$12");
        s32 *dst = (s32 *)arg2;
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(dst));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }
}
