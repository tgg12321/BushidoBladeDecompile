s32 func_800645B0(void) {
    register s32 s3 asm("$19");
    s32 mask;
    s32 a0;
    s32 s0;
    s32 idx2;
    s32 cont;
    s3 = 0;
    D_800F10EC = 1;
    do {
        a0 = 0;
loop_inner:
        s0 = s3 + a0;
        {
            register s32 one asm("$3") = 1;
            mask = one << s0;
        }
        a0 += 1;
        if (!(D_800A3444 & mask)) {
            idx2 = s0 << 1;
            *((s32 *)(((s32)(&D_800F0D78)) + ((idx2 + s0) << 2))) = (((s32 *)D_800A347C)[0] + (rand() & 0xFF)) - 0x7F;
            s3 += 4;
            *((s32 *)(((s32)(&D_800F0D7C)) + ((idx2 + s0) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
            *((s32 *)(((s32)(&videoDec)) + ((idx2 + s0) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
            {
                s32 last = rand();
                s32 bits = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                D_800A3444 = bits | mask;
            }
            do { } while (0);
            cont = s3 < 0xF;
        } else {
            if (a0 >= 4) {
                s3 += 4;
                cont = s3 < 0xF;
            } else {
                goto loop_inner;
            }
        }
    } while (cont != 0);
    return 1;
}
