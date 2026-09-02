void func_80016E60(u8 *arg0, s32 arg1) {
    u8 *ot[2];
    u8 *env;
    s32 select;
    s32 special;
    s32 limit;
    u32 fb_base;
    s32 idx;
    u32 pad;
    s32 next;

    select = 0;
    special = 0;
    if (D_800A38DC == 2) {
        special = D_800A389A < 1;
    }
    limit = 3;
    if (special != 0) {
        limit = 6;
    }

    D_800A36B0 = 1;
    func_8005C650(3, 0x7F, 0x7F);
    fb_base = (&D_800A3770)[D_800A36AC & 1];

loop:
    {
        idx = D_800A36AC & 1;
        D_800A38B4 = fb_base + (idx * 0x9A00);
        D_800A374C = (u8 *)&ot[idx];
        env = &D_800F7438 + (idx * 0x4090);

        ClearOTagR(D_800A374C, 1);
        func_80019568();
        if (special != 0) {
            func_8005C8A8(2, select | (D_800A3788 << 16), D_800A38B4, 0);
        } else {
            func_8005C8A8(0, select, D_800A38B4, 0);
        }
        func_80036940();
        func_8005C6D0();
        DrawSync(0);
        VSync(2);
        PutDispEnv(env + 0x5C);
        PutDrawEnv(env);
        DrawOTag(arg0 + 0x408C);
        DrawOTag(D_800A374C);
        D_800A36AC++;

        pad = D_80102794;
        if (pad & 0x100010) {
            func_8005C650(1, 0x7F, 0x7F);
            select = 0;
            goto done;
        }
        if (pad & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            goto done;
        }
        if (pad & 0x10001000) {
            func_8005C650(0, 0x7F, 0x7F);
            next = select - 1;
            if (select == 0) {
                next = limit - 1;
            }
            select = next;
        } else if (pad & 0x40004000) {
            func_8005C650(0, 0x7F, 0x7F);
            next = 0;
            if (select != limit - 1) {
                next = select + 1;
            }
            select = next;
        }

        if ((special != 0) && (select >= 3)) {
            if (D_80102794 & 0x80008000) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3788 |= 1 << (select - 3);
            } else if (D_80102794 & 0x20002000) {
                func_8005C650(0, 0x7F, 0x7F);
                D_800A3788 &= ~(1 << (select - 3));
            }
        }
        goto loop;
    }

done:
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
