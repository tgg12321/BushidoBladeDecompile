void func_8002FF20(u8 *arg0, u8 arg1) {
    s32 mat_local[8];
    s32 *playerData;
    s32 *s2_ptr;
    s32 *rot_mat;
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");

    arg0[8] = 1;
    arg0[9] = arg1;
    playerData = (s32 *)game_GetPlayerData(arg0[6] < 1);
    rot_mat = (s32 *)((u8 *)arg0 + 0xC);
    s2_ptr = (s32 *)playerData[arg0[9]];

    /* 3x3 identity matrix at arg0+0xC..arg0+0x1D (9 s16 entries).
     * The final entry (arg0+0x1C) lands in the jal func_8007F87C delay slot. */
    *(s16 *)((u8 *)arg0 + 0xC) = 0x1000;
    *(s16 *)((u8 *)arg0 + 0xE) = 0;
    *(s16 *)((u8 *)arg0 + 0x10) = 0;
    *(s16 *)((u8 *)arg0 + 0x12) = 0;
    *(s16 *)((u8 *)arg0 + 0x14) = 0x1000;
    *(s16 *)((u8 *)arg0 + 0x16) = 0;
    *(s16 *)((u8 *)arg0 + 0x18) = 0;
    *(s16 *)((u8 *)arg0 + 0x1A) = 0;
    *(s16 *)((u8 *)arg0 + 0x1C) = 0x1000;
    RotMatrixX(*(s16 *)((u8 *)arg0 + 0x54), rot_mat);
    RotMatrixY(*(s16 *)((u8 *)arg0 + 0x56), rot_mat);
    RotMatrixZ(*(s16 *)((u8 *)arg0 + 0x58), rot_mat);
    func_8002EECC(s2_ptr, mat_local);
    MulMatrix0(mat_local, rot_mat, rot_mat);

    /* Subtract opponent reference position from self position. */
    *(s32 *)((u8 *)arg0 + 0x2C) -= s2_ptr[5];
    *(s32 *)((u8 *)arg0 + 0x30) -= s2_ptr[6];
    *(s32 *)((u8 *)arg0 + 0x34) -= s2_ptr[7];

    /* Load 5 packed rotation-matrix words into GTE coef regs $0..$4. */
    {
        register s32 *mp asm("$12");
        s32 *src = mat_local;
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
        s32 *src = (s32 *)((u8 *)arg0 + 0x2C);
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

    /* Store MAC1/2/3 back to arg0+0x2C (rotated pos in place). */
    {
        register s32 *rp asm("$12");
        s32 *dst = (s32 *)((u8 *)arg0 + 0x2C);
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(dst));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }

    /* Halve x, y, z (signed arithmetic shift). */
    *(s32 *)((u8 *)arg0 + 0x2C) >>= 1;
    *(s32 *)((u8 *)arg0 + 0x30) >>= 1;
    *(s32 *)((u8 *)arg0 + 0x34) >>= 1;
}
