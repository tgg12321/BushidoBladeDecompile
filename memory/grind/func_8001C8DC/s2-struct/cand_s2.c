void func_8001C8DC(void) {
    u8 prev;
    s32 snd;
    s16 t;
    u8 *p;

    if (D_80101F5E != 0 || D_801023AA != 0) {
        D_800A382E++;
    } else if (D_800A38DC == 0 && D_800A385C != 0 && D_80101F7A == 1) {
        func_8003B56C(3);
    }
    if (D_800A382E < 0x3D) goto end;

    switch (D_800A38DC) {
    case 0:
        if (D_80101F5E != 0) break;
        if (D_800A3680 != 0) {
            if (--D_800A3680 == 0) {
                func_8005B58C();
                if (D_800A37C6 != 0) {
                    D_800A37C6 = 0;
                    break;
                }
                if (D_800A3894 != 0) {
                    func_8003B484(D_800A3894 + 6);
                    if (D_800A3836 != 0xFF) {
                        func_8003B534(2);
                    } else {
                        func_8003B534(5);
                    }
                } else {
                    func_8003B5A4();
                }
            } else {
                func_8001C51C();
                func_8001C820();
            }
            goto end;
        }
        if (D_800A385C == 0) break;
        func_8003B56C(2);
        goto end;
    case 3:
        if (D_80101F5E != 0) break;
        if (func_80033DF4() == 0) goto end;
        D_800A390D = 1;
        gpu_ResetGraphMode1();
        func_80040510(1, D_800A38DE, 0);
        prev = D_800A38E0;
        switch (D_800A38E2) {
        case 0x1F:
            D_800A38E0 = 1;
            func_8004659C(1);
            break;
        case 0x33:
            D_800A38E0 = 2;
            func_8004659C(2);
            break;
        case 0x51:
            D_800A38E0 = 3;
            func_8004659C(3);
            break;
        }
        if (D_800A3728 = prev != D_800A38E0) {
            func_8001C624();
            snd = func_80021904(0);
            g_practice_menu_table[0].unk_5E = 0;
            func_80021A98(0, (u8 *)snd, 0);
        }
        func_8001C51C();
        if (D_800A384C < 4) {
            func_80041BF4(D_800A38EC, D_800A38ED, D_800A38EE);
        }
        goto end;
    case 2:
        if ((t = D_80101F5E) == 0 || D_801023AA == 0) {
            p = (u8 *)&D_800A37D2;
            p[t != 0]++;
        }
        D_800A3670 = 1;
        D_800A38DF = func_80022408((s32 *)((u8 *)&D_80101FBC + (s32)D_800A3748 * 0x44C));
        D_800A3834 = 0;
        goto end;
    case 4:
    case 6:
        if ((t = D_80101F5E) == 0 || D_801023AA == 0) {
            p = (u8 *)&D_800A37D2;
            p[t != 0]++;
        }
        break;
    case 5:
        goto end;
    }
    D_800A3834 = 4;
end:
    if (D_800A37D2.p0 >= 100) {
        D_800A37D2.p0 = 99;
    }
    if (D_800A37D2.p1 >= 100) {
        D_800A37D2.p1 = 99;
    }
}

/* Tail word after func_8001C8DC's seven-entry compiler-generated switch table. */
const u32 D_800100E0[1] = { 0x00000000 };
