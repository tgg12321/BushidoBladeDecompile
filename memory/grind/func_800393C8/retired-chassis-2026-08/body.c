void func_800393C8(u8 arg0, u8 arg1, s32 *arg2, u16 *arg3) {
    extern s16 D_800A3714;
    extern u8 D_800A3209;
    s32 frame_pad[2];
    register u8 mode asm("t4") = arg0;
    register u8 *slot asm("a0") = (u8 *)D_800F68E0;
    register s32 i asm("t2") = 0;
    register u8 *tail asm("t1") = slot + 0xE;
    s32 next;
    s32 value;

    if ((u32)&frame_pad[0] == 0) {
        D_800A3209 = D_800A3209;
    }

loop:
    {
        register s32 state asm("t0") = *(s16 *)slot;
        s32 v0 = -1;

        if (state == v0) {
            goto miss;
        }
        v0 = state - 1;

        {
            register u8 age asm("t3") = *(u8 *)(tail - 0xC);
            s32 v1 = age & 0xFF;

            if (v1 != v0) {
                goto miss;
            }
            v0 = (u32)v1 < 0xFF;
            if (v0 == 0) {
                goto miss;
            }

            {
                u32 raw_rot = *(u16 *)(tail - 4) << 16;
                s16 rot = raw_rot >> 16;

                if ((raw_rot >> 28) != mode) {
                    goto miss;
                }
                if (*(s16 *)(tail - 0xA) != arg2[0]) {
                    goto miss;
                }
                if (*(s16 *)(tail - 8) != arg2[1]) {
                    goto miss;
                }
                if (*(s16 *)(tail - 6) != arg2[2]) {
                    goto miss;
                }
                if (((rot - *(s16 *)(arg3 + 0)) & 0xFFF) != 0) {
                    goto miss;
                }
                if (((*(s16 *)(tail - 2) - *(s16 *)(arg3 + 1)) & 0xFFF) != 0) {
                    goto miss;
                }
                if (((*(s16 *)(tail + 0) - *(s16 *)(arg3 + 2)) & 0xFFF) == 0) {
                    *(u8 *)(slot + 2) = age + 1;
                    goto end;
                }
            }
        }
    }

miss:
    i++;
    tail += 0x10;
    if (i < 0xB4) {
        slot += 0x10;
        goto loop;
    }

    value = D_800A3714;
    slot = (u8 *)D_800F68E0 + value * 0x10;

    if (value < 0xB4) {
find_free:
        if (*(s16 *)slot != -1) {
            next = value + 1;
            D_800A3714 = next;
            slot += 0x10;
            value = (s16)next;
            if (value < 0xB4) {
                goto find_free;
            }
        }
    }

    value = mode << 12;
    next = D_800A3714;
    if (next == 0xB4) {
        D_800A3209++;
        goto end;
    }

    *(s16 *)(slot + 0) = 0;
    *(u8 *)(slot + 3) = arg1;
    *(u8 *)(slot + 2) = 0;
    *(s16 *)(slot + 4) = arg2[0];
    *(s16 *)(slot + 6) = arg2[1];
    *(s16 *)(slot + 8) = arg2[2];
    *(u16 *)(slot + 0xA) = (arg3[0] & 0xFFF) | value;
    *(u16 *)(slot + 0xC) = arg3[1];
    *(u16 *)(slot + 0xE) = arg3[2];

end:
    return;
}
