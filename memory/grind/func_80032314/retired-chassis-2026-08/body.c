void func_80032314(void) {
    register u8 *t0 asm("t0") = &D_80104E88;
    register s32 t1 asm("t1") = 0;
    register u8 *a3 asm("a3") = &D_80104E88 + 2;
    u8 *ent;
    s32 state;
    s32 a0;

loop:
    if (*t0 == 0) goto next;
    {
        s32 v1_v = (*(u8 *)(a3 + 1) == 0);
        v1_v = v1_v * 0x44C;
        ent = v1_v + &D_80101EC8;
    }
    state = *(u16 *)(ent + 0x6A);
    a0 = state & 0xFFFF;
    if (a0 == 4) goto next;
    if (a0 == 0x14) goto next;
    if (a0 == 0xF) goto next;
    if ((u32)(state - 0x1C) < 2) goto next;
    if ((u32)(state - 0x1E) < 2) goto next;
    if ((u32)(state - 0x20) < 2) goto next;
    if (a0 == 0x11) goto next;
    {
        s32 dx = *(s32 *)(ent + 0xF4) - *(s32 *)(a3 + 2);
        s32 dy = *(s32 *)(ent + 0xF8) - *(s32 *)(a3 + 6);
        s32 dz = *(s32 *)(ent + 0xFC) - *(s32 *)(a3 + 0xA);
        register u32 dist_sq asm("a0") = (u32)(dx * dx + dy * dy + dz * dz);
        u32 log2_val;
        if (dist_sq < 0x400) {
            log2_val = (u32)(*(&D_8008D118 + dist_sq)) >> 3;
        } else {
            s32 clz = 0;
            s32 sp_tmp;
            register s32 t4_v asm("t4");
            if ((s32)dist_sq >= 0) {
                t4_v = (s32)dist_sq;
                __asm__ volatile (".word 0x488CF000" :: "r"(t4_v));
                __asm__ volatile ("nop");
                __asm__ volatile ("nop");
                t4_v = (s32)&sp_tmp;
                __asm__ volatile (".word 0xE99F0000" :: "r"(t4_v));
                clz = sp_tmp;
            }
            {
                u32 v0_m = (u32)(clz & -2);
                u32 v1_m = 0x16 - v0_m;
                u32 hi;
                u32 idx = dist_sq >> v1_m;
                hi = (u32)(u8)*(&D_8008D118 + idx);
                v0_m = 0x13 - (v1_m >> 1);
                log2_val = (hi << 16) >> v0_m;
            }
        }
        {
            s32 v1 = *a3;
            s32 v0 = v1 << 4;
            v0 = v0 - v1;
            v0 = v0 << 1;
            v0 = v0 + 0x1F4;
            if (log2_val < (u32)v0) {
                *(s16 *)(ent + 0x286) = 5;
                *t0 = 0;
            }
        }
    }
next:
    t1 += 1;
    a3 += 0x2C;
    t0 += 0x2C;
    if (t1 < 4) goto loop;
}
