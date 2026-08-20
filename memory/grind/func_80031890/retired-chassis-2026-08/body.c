void func_80031890(s32 arg0, s32 arg1, s32 arg2) {
    s32 *mat;
    s32 angle1;
    s32 angle2;
    s32 sum_sq;
    s32 adj;
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");

    if (*(s16 *)((u8 *)arg1 + 0x2) != 0xE) {
        s32 vx = *(s32 *)((u8 *)arg1 + 0x44);
        s32 vz = *(s32 *)((u8 *)arg1 + 0x4C);
        s32 av_ext = (s32)*(s16 *)((u8 *)arg1 + 0x5E);
        sum_sq = vx * vx + vz * vz;
        if (rng_Next() & 1) {
            if (sum_sq >= 0) {
                adj = sum_sq;
            } else {
                adj = sum_sq + 0x3F;
            }
        } else {
            adj = -sum_sq;
            if (adj < 0) adj = adj + 0x3F;
        }
        *(s16 *)((u8 *)arg1 + 0x5E) = (s16)(av_ext + (adj >> 6));
    }

    mat = (s32 *)((u8 *)arg0 + 0xD8);
    {
        u32 angle1_raw = ((u16 *)&D_8008EBA0)[arg2];
        s32 pos_x = *(s32 *)((u8 *)arg1 + 0x2C);
        s32 pos_y = *(s32 *)((u8 *)arg1 + 0x30);
        s32 pos_z = *(s32 *)((u8 *)arg1 + 0x34);
        *(s16 *)((u8 *)arg0 + 0xD8) = 0x1000;
        *(s16 *)((u8 *)arg0 + 0xDA) = 0;
        *(s16 *)((u8 *)arg0 + 0xDC) = 0;
        *(s16 *)((u8 *)arg0 + 0xDE) = 0;
        *(s16 *)((u8 *)arg0 + 0xE0) = 0x1000;
        *(s16 *)((u8 *)arg0 + 0xE2) = 0;
        *(s16 *)((u8 *)arg0 + 0xE4) = 0;
        *(s16 *)((u8 *)arg0 + 0xE6) = 0;
        *(s16 *)((u8 *)arg0 + 0xE8) = 0x1000;
        angle1 = (s32)(angle1_raw & 0xFFF);
        angle2 = (s32)((((pos_x * 16) + pos_y + (pos_z * 8)) & 0x7FF) - 0x400);
        RotMatrixY(angle1, mat);
    }
    RotMatrixX(angle2, mat);

    /* Load 5 packed rotation-matrix words into GTE coef regs $0..$4. */
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

    /* Pack vel low halves into V0.xy, load V0.z, mvmva, store back. */
    {
        register s32 *vp asm("$12");
        s32 *src = (s32 *)((u8 *)arg1 + 0x44);
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
    __asm__ volatile (".word 0x4A486012");  /* mvmva 1, 0, 0, 3, 0 */
    {
        register s32 *rp asm("$12");
        s32 *dst = (s32 *)((u8 *)arg1 + 0x44);
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(dst));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }

    if ((u32)(angle1 - 0x401) < 0x7FFU) {
        s32 vx = *(s32 *)((u8 *)arg1 + 0x44);
        s32 vy;
        s32 vz;
        if (vx < 0) vx += 7;
        vy = *(s32 *)((u8 *)arg1 + 0x48);
        *(s32 *)((u8 *)arg1 + 0x44) = vx >> 3;
        if (vy < 0) vy += 7;
        vz = *(s32 *)((u8 *)arg1 + 0x4C);
        *(s32 *)((u8 *)arg1 + 0x48) = vy >> 3;
        if (vz < 0) vz += 7;
        *(s32 *)((u8 *)arg1 + 0x4C) = vz >> 3;
    } else {
        s32 vx = *(s32 *)((u8 *)arg1 + 0x44);
        s32 vy;
        s32 vz;
        if (vx < 0) vx += 3;
        vy = *(s32 *)((u8 *)arg1 + 0x48);
        *(s32 *)((u8 *)arg1 + 0x44) = vx >> 2;
        if (vy < 0) vy += 3;
        vz = *(s32 *)((u8 *)arg1 + 0x4C);
        *(s32 *)((u8 *)arg1 + 0x48) = vy >> 2;
        if (vz < 0) vz += 3;
        *(s32 *)((u8 *)arg1 + 0x4C) = vz >> 2;
    }

    {
        s32 vx = *(s32 *)((u8 *)arg1 + 0x44);
        s32 vy = *(s32 *)((u8 *)arg1 + 0x48);
        *(s32 *)((u8 *)arg1 + 0x2C) += (s32)((u32)vx + ((u32)vx >> 31)) >> 1;
        {
            s32 vz = *(s32 *)((u8 *)arg1 + 0x4C);
            *(s32 *)((u8 *)arg1 + 0x30) += (s32)((u32)vy + ((u32)vy >> 31)) >> 1;
            *(s32 *)((u8 *)arg1 + 0x34) += (s32)((u32)vz + ((u32)vz >> 31)) >> 1;
        }
    }
}
