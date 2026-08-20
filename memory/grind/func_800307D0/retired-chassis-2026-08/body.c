s32 func_800307D0(s32 *a0) {
    s32 count;
    s32 s1;
    s32 v0;
    s32 v1;
    s32 *s2;
    s32 s3;
    s32 *a2;

    count = *(s16 *)((u8 *)a0 + 0x330);
    if (count == 0) {
        return -1;
    }
    s1 = 0;
    if (count < 2) {
        goto do_sll;
    }
    if (*(s16 *)((u8 *)a0 + 0x88) == -1) {
        v0 = s1 << 1;
        goto after_sll;
    }
    v0 = *(s16 *)((u8 *)a0 + 0x332);
    v1 = *(s16 *)((u8 *)a0 + 0x14);
    s1 = (u32)(v0 ^ v1) < 1;
do_sll:
    v0 = s1 << 1;
after_sll:
    s2 = (s32 *)((u8 *)a0 + v0);
    s3 = *(s16 *)((u8 *)s2 + 0x332);
    a2 = func_80030580(a0, s3);
    v0 = s1;
    if (v0 < *(s16 *)((u8 *)a0 + 0x330) - 1) {
        s32 *ptr = s2;
        do {
            *(u16 *)((u8 *)ptr + 0x332) = *(u16 *)((u8 *)ptr + 0x334);
            v0++;
            ptr = (s32 *)((u8 *)ptr + 2);
        } while (v0 < *(s16 *)((u8 *)a0 + 0x330) - 1);
    }

    *(u16 *)(0, (u8 *)a0 + 0x330) = *(u16 *)((u8 *)a0 + 0x330) - 1;
    v1 = *(s16 *)((u8 *)a2 + 2);
    if (v1 == 0xE) {
        s32 a0_arg = D_800A36F2 ^ 0xE;
        func_80032854(a0_arg != 0, 0x2F, (u8 *)a2 + 0x2C, 0);
    } else {
        s32 a0_arg = v1 ^ D_800A36F2;
        func_80032854(a0_arg != 0, 0x2A, (u8 *)a2 + 0x2C, 0);
    }
    return s3;
}
