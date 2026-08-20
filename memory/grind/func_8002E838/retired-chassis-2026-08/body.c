void func_8002E838(u8 *arg0) {
    s32 sp_tmp;
    s32 *mat;
    u32 dist_sq;
    s32 a1_volume;
    s32 charId;

    *(s32 *)((u8 *)arg0 + 0xA8) =
        (*(s32 **)((u8 *)arg0 + 0x64))[0] - (*(s32 **)((u8 *)arg0 + 0x60))[0];
    *(s32 *)((u8 *)arg0 + 0xAC) =
        (*(s32 **)((u8 *)arg0 + 0x64))[1] - (*(s32 **)((u8 *)arg0 + 0x60))[1];
    {
        s32 dz = (*(s32 **)((u8 *)arg0 + 0x64))[2] - (*(s32 **)((u8 *)arg0 + 0x60))[2];
        *(s32 *)((u8 *)arg0 + 0xB0) = dz;
        charId = ratan2(*(s32 *)((u8 *)arg0 + 0xA8), dz);
    }

    dist_sq = (u32)((*(s32 *)((u8 *)arg0 + 0xA8) * *(s32 *)((u8 *)arg0 + 0xA8))
                  + (*(s32 *)((u8 *)arg0 + 0xB0) * *(s32 *)((u8 *)arg0 + 0xB0)));
    *(s16 *)((u8 *)arg0 + 0xFA) = (s16)(0x800 - charId);

    if (dist_sq < 0x400) {
        a1_volume = (s32)((u32)(u8)(*(((u8 *)&D_8008D118) + dist_sq)) >> 3);
    } else {
        u32 v1_sqrt = 0;
        if ((s32)dist_sq >= 0) {
            register s32 t4_v asm("$12");
            register s32 v0_pin asm("$2");
            t4_v = (s32)dist_sq;
            __asm__ volatile (".word 0x488CF000" :: "r"(t4_v));  /* mtc2 $t4, $30 */
            __asm__ volatile ("nop");
            __asm__ volatile ("nop");
            v0_pin = (s32)&sp_tmp;
            __asm__ volatile ("move %0, %1" : "=r"(t4_v) : "r"(v0_pin));
            __asm__ volatile (".word 0xE99F0000" :: "r"(t4_v));  /* swc2 $31, 0($t4) */
            v1_sqrt = (u32)sp_tmp;
        }
        {
            u32 clz = v1_sqrt;
            u32 v0_m = clz & (u32)-2;
            u32 v1_m = 0x16 - v0_m;
            u32 idx = dist_sq >> v1_m;
            u32 hi = (u32)(u8)(*(((u8 *)&D_8008D118) + idx));
            do { v0_m = 0x13 - (v1_m >> 1); } while (0);
            a1_volume = (s32)((hi << 16) >> v0_m);
        }
    }

    charId = ratan2(*(s32 *)((u8 *)arg0 + 0xAC), a1_volume);
    mat = (s32 *)((u8 *)arg0 + 0xD8);
    *(s16 *)((u8 *)arg0 + 0xF8) = (s16)(0x800 - charId);

    /* 3x3 identity rotation matrix at arg0+0xD8.  The final entry (0xE8)
     * lands in the delay slot of jal func_8007FA1C. */
    *(s16 *)((u8 *)arg0 + 0xD8) = 0x1000;
    *(s16 *)((u8 *)arg0 + 0xDA) = 0;
    *(s16 *)((u8 *)arg0 + 0xDC) = 0;
    *(s16 *)((u8 *)arg0 + 0xDE) = 0;
    *(s16 *)((u8 *)arg0 + 0xE0) = 0x1000;
    *(s16 *)((u8 *)arg0 + 0xE2) = 0;
    *(s16 *)((u8 *)arg0 + 0xE4) = 0;
    *(s16 *)((u8 *)arg0 + 0xE6) = 0;
    *(s16 *)((u8 *)arg0 + 0xE8) = 0x1000;
    RotMatrixY(*(s16 *)((u8 *)arg0 + 0xFA), mat);
    RotMatrixX(*(s16 *)((u8 *)arg0 + 0xF8), mat);

    /* Load 5 packed rotation-matrix words into GTE coef regs $0..$4. */
    {
        register s32 *mp asm("$12");
        register s32 t5 asm("$13");
        register s32 t6 asm("$14");
        register s32 t7 asm("$15");
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

    /* Pack pos_diff low halves into V0.xy via mtc2 $0, load V0.z via lwc2 $1,
     * then mvmva (rotation matrix * V0, sf=1, cm=0, vm=0, tm=3). */
    {
        register s32 *vp asm("$12");
        register s32 t5 asm("$13");
        register s32 t6 asm("$14");
        s32 *src = (s32 *)((u8 *)arg0 + 0xA8);
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

    /* Store MAC1/2/3 back to arg0+0xA8 (rotated pos in place). */
    {
        register s32 *rp asm("$12");
        s32 *dst = (s32 *)((u8 *)arg0 + 0xA8);
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(dst));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }

    (void)charId;
}
