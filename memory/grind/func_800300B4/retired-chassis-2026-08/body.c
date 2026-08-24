void func_800300B4(u8 *arg0) {
    s32 mac_result[3];
    s32 buf2[2];
    s32 buf3[8];
    s32 *playerData;
    s32 *mat;
    s32 lookup;
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");

    playerData = (s32 *)game_GetPlayerData(arg0[6] < 1);
    mat = (s32 *)playerData[arg0[9]];

    /* Load rotation matrix into GTE cop2 control regs 0-4 */
    {
        register s32 *mp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(mp) : "r"(mat));
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

    /* Load V0 vector and run mvmva */
    {
        register s32 *vp asm("$12");
        s32 *vec_src = (s32 *)(arg0 + 0x2C);
        __asm__ volatile ("move %0, %1" : "=r"(vp) : "r"(vec_src));
        t6 = ((u16 *)vp)[2];
        t5 = ((u16 *)vp)[0];
        t6 = t6 << 16;
        t5 = t5 | t6;
        __asm__ volatile ("mtc2 %0, $0" :: "r"(t5));
        __asm__ volatile ("lwc2 $1, 8(%0)" :: "r"(vp));
    }
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");  /* mvmva 1,0,0,3,0 */

    /* Store MAC1/2/3 to mac_result via $t4 alias */
    {
        register s32 *rp asm("$12");
        __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(mac_result));
        __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
        __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
    }

    /* Add matrix translation to MAC1-3 */
    mac_result[0] += mat[5];
    mac_result[1] += mat[6];
    mac_result[2] += mat[7];

    MulMatrix0(mat, (s32 *)(arg0 + 0xC), buf3);
    func_8002F2D0(buf3, buf2);

    lookup = (&D_8008EB80)[*(s16 *)(arg0 + 2)];
    func_80049718(lookup, 1, mac_result, (s16 *)buf2);
    func_800393C8(arg0[10], lookup, mac_result, (u16 *)buf2);
}
