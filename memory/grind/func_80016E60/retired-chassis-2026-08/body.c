void func_80016E60(u8 *arg0) {
    u8 *ot[2];
    u8 *env;
    register s32 select asm("s1");
    register s32 special asm("s2");
    register s32 limit asm("s3");
    register s32 fb_base asm("s4");
    register u8 *saved_arg0 asm("s5");

    saved_arg0 = arg0;
    select = 0;
    special = 0;
    if (D_800A38DC == 2) {
        register s32 mode asm("v0");
        mode = D_800A389A;
        special = mode < 1;
    }
    limit = 3;
    if (special != 0) {
        limit = 6;
    }

    D_800A36B0 = 1;
    func_8005C650(3, 0x7F, 0x7F);
    fb_base = *(&D_800A3770 + (D_800A36AC & 1));

    while (1) {
        s32 idx = D_800A36AC & 1;
        D_800A38B4 = fb_base + (idx * 0x9A00);
        D_800A374C = (u8 *)&ot[idx];
        env = &D_800F7438 + (idx * 0x4090);

        ClearOTagR(D_800A374C, 1);
        func_80019568();
        if (special != 0) {
            func_8005C8A8(2, select | (D_800A3788 << 16), D_800A38B4, 0);
        } else {
            register s32 zero asm("a0");
            zero = 0;
            func_8005C8A8(zero, select, D_800A38B4, 0);
        }
        func_80036940();
        func_8005C6D0();
        DrawSync(0);
        VSync(2);
        PutDispEnv(env + 0x5C);
        PutDrawEnv(env);
        DrawOTag(saved_arg0 + 0x408C);
        DrawOTag(D_800A374C);
        D_800A36AC++;

        if (D_80102794 & 0x100010) {
            func_8005C650(1, 0x7F, 0x7F);
            select = 0;
            break;
        }
        if (D_80102794 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            break;
        }
        if (D_80102794 & 0x10001000) {
            register s32 next asm("a1");
            func_8005C650(0, 0x7F, 0x7F);
            next = select - 1;
            if (select == 0) {
                next = limit - 1;
            }
            select = next;
            goto after_move;
        }
        if (D_80102794 & 0x40004000) {
            s32 next;
            func_8005C650(0, 0x7F, 0x7F);
            next = 0;
            if (select != (limit - 1)) {
                next = select + 1;
            }
            select = next;
        }

    after_move:
        if ((special != 0) && (select >= 3)) {
            if (D_80102794 & 0x80008000) {
                func_8005C650(0, 0x7F, 0x7F);
                {
                    register s32 shift asm("v0") = select - 3;
                    register s32 mask asm("v1");
                    register u8 bits asm("a0");
                    mask = 1;
                    bits = D_800A3788;
                    __asm__ volatile(
                        "sllv %0, %0, %2"
                        : "=r"(mask)
                        : "0"(mask), "r"(shift));
                    bits |= mask;
                    D_800A3788 = bits;
                }
            } else if (D_80102794 & 0x20002000) {
                func_8005C650(0, 0x7F, 0x7F);
                {
                    register s32 shift asm("v0") = select - 3;
                    register s32 mask asm("v1");
                    register u8 bits asm("v0");
                    mask = 1;
                    __asm__ volatile(
                        "sllv %0, %0, %2"
                        : "=r"(mask)
                        : "0"(mask), "r"(shift));
                    bits = D_800A3788;
                    mask = ~mask;
                    D_800A3788 = bits & mask;
                }
            }
        }
    }

    if (select != 0) {
        if (select == 1) {
            D_800A31DA = 1;
            D_800A3834 = 8;
            func_800372C0();
        } else if (select == 2) {
            D_800A3834 = 8;
            func_800372C0();
        }
    }

    DrawSync(0);
    ResetRCnt(0xF2000001);
    D_800A36B0 = 1;
}
