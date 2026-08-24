void func_80019310(volatile s32 *arg0, s32 *arg1) {
    s32 sp_buf[6];
    register s32 *t0 asm("t0") = arg1;
    register s32 t1 asm("t1");
    register s32 *t4 asm("t4");
    register s32 t5 asm("t5");
    register s32 t6 asm("t6");
    register s32 t7 asm("t7");

    /* SetRotMatrix */
    t1 = arg0[1];
    t4 = (s32 *)t1;
    t5 = t4[0];
    t6 = t4[1];
    __asm__ volatile (".word 0x48CD0000" :: "r"(t5));
    __asm__ volatile (".word 0x48CE0800" :: "r"(t6));
    t5 = t4[2];
    t6 = t4[3];
    t7 = t4[4];
    __asm__ volatile (".word 0x48CD1000" :: "r"(t5));
    __asm__ volatile (".word 0x48CE1800" :: "r"(t6));
    __asm__ volatile (".word 0x48CF2000" :: "r"(t7));

    /* SetTransVector - reload */
    t1 = arg0[1];
    t4 = (s32 *)t1;
    t5 = t4[5];
    t6 = t4[6];
    __asm__ volatile (".word 0x48CD2800" :: "r"(t5));
    t7 = t4[7];
    __asm__ volatile (".word 0x48CE3000" :: "r"(t6));
    __asm__ volatile (".word 0x48CF3800" :: "r"(t7));

    {
        s16 count = *(s16 *)((u8 *)t0 + 4);
        s32 *output = (s32 *)t0[3];
        s32 i = 0;
        if (count > 0) {
            do {
                s32 *base = (s32 *)t0[0];
                s32 *p = (s32 *)((u8 *)base + (i << 3));
                __asm__ volatile ("move %0, %1" : "=r"(t4) : "r"(p));
                __asm__ volatile (".word 0xC9800000" :: "r"(t4));
                __asm__ volatile (".word 0xC9810004" :: "r"(t4));
                __asm__ volatile ("nop");
                __asm__ volatile ("nop");
                __asm__ volatile (".word 0x4A480012");
                t4 = sp_buf;
                __asm__ volatile (".word 0xE9990000" :: "r"(t4));
                __asm__ volatile (".word 0xE99A0004" :: "r"(t4));
                __asm__ volatile (".word 0xE99B0008" :: "r"(t4));

                output[0] = sp_buf[0] << 7;
                output[1] = sp_buf[1] << 7;
                i++;
                output[2] = sp_buf[2] << 7;
                output[3] = 0;
                output[4] = 0;
                output[5] = 0;
                output = (s32 *)((u8 *)output + 0x40);
            } while (i < *(s16 *)((u8 *)t0 + 4));
        }
    }

    {
        typedef struct { s32 a[8]; } Copy8;
        s32 *src = (s32 *)arg0[1];
        *(Copy8 *)((u8 *)t0 + 0x14) = *(Copy8 *)src;
    }
}
