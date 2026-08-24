void func_800204C0(u8 *arg0) {
    long temp_s1;
    s32 sp10[3];
    s16 sp20[3];
    s32 tx, ty, tz;
    s32 mul;
    s16 idx;
    s32 pd;

    temp_s1 = *(s16 *)(arg0 + 0x4);
    if (*(s16 *)(arg0 + 0x350) != 0) {
        *(u16 *)(arg0 + 0x350) += 1;
        if ((*(u16 *)(arg0 + 0x350) & 7) == 2) {
            pd = game_GetPlayerData(temp_s1);
            idx = *(s16 *)(arg0 + 0x352);

            /* Load 5 packed rotation-matrix words from *(s32 **)(pd + idx*4)
             * into GTE coef regs $0..$4. */
            {
                register s32 *mp asm("$12");
                register s32 t5 asm("$13");
                register s32 t6 asm("$14");
                register s32 t7 asm("$15");
                register s32 *src asm("$3") = (s32 *)*(s32 *)(pd + idx * 4);
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

            /* Pack arg0+0x354 vec3 (3 shorts) into V0 via mtc2 $0/lwc2 $1,
             * then mvmva (rotation matrix * V0, sf=1, mx=0, v=0, cv=3, lm=0). */
            {
                register s32 *vp asm("$12");
                register s32 t5 asm("$13");
                register s32 t6 asm("$14");
                s32 *src = (s32 *)(arg0 + 0x354);
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

            { s32 cc = *(s16 *)(arg0 + 0x350); mul = ((0x96 - cc) << 12) / 150; do {} while(0); }

            /* Store MAC1/2/3 to sp10. */
            {
                register s32 *rp asm("$12");
                s32 *dst = sp10;
                __asm__ volatile ("move %0, %1" : "=r"(rp) : "r"(dst));
                __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(rp));
                __asm__ volatile ("swc2 $26, 4(%0)" :: "r"(rp));
                __asm__ volatile ("swc2 $27, 8(%0)" :: "r"(rp));
            }
            tx = (sp10[0] * mul) / 0x1000;
            sp20[0] = tx;
            ty = (sp10[1] * mul) / 0x1000;
            sp20[1] = ty;
            tz = (sp10[2] * mul) / 0x1000;
            sp20[2] = tz;
            if ((s16)ty >= 0x801) {
                sp20[0] = -tx;
                sp20[1] = -ty;
                sp20[2] = -tz;
            }
            func_80032854(temp_s1, 4, (u8 *)0x1F8000A8 + temp_s1 * 0x108 + *(s16 *)(arg0 + 0x352) * 0xC, sp20);
        }
        if (*(s16 *)(arg0 + 0x350) >= 0x96) {
            *(s16 *)(arg0 + 0x350) = 0;
        }
        *(s16 *)(arg0 + 0x350) = 0;
    }
}
