void func_8001C8DC(void) {
    u8 prev;
    s32 snd;

    if (g_practice_menu_table[0].unk_96 != 0 || g_practice_menu_table[1].unk_96 != 0) {
        D_800A382E++;
    } else if (D_800A38DC == 0 && D_800A385C != 0 && g_practice_menu_table[0].unk_B2 == 1) {
        func_8003B56C(3);
    }
    if (D_800A382E < 0x3D) goto end;

    switch (D_800A38DC) {
    case 0:
        if (g_practice_menu_table[0].unk_96 != 0) break;
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
        if (g_practice_menu_table[0].unk_96 != 0) break;
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
        {
            s16 t;
            u8 *p;
            if ((t = g_practice_menu_table[0].unk_96) == 0 || g_practice_menu_table[1].unk_96 == 0) {
                /* FAKE: second handle to D_800A37D2 (pointer-alias-fake-exception, owner Q63): the
                 * target sets the pair's base in its own register before the index (v0 base, v1
                 * index); the index written without p computes the index first and loses that seat
                 * (17 on this body, 19 on the s1 body; ledger evidence.md s1 item 3, s2 item 4). */
                p = &D_800A37D2;
                /* FAKE: indexes past D_800A37D2 into D_800A37D3 (owner Q63, this byte pair only):
                 * the target also reaches each byte by its own symbol, which no single array or
                 * struct gives (ledger evidence.md s1 item 1, s2 item 3). */
                p[t != 0]++;
            }
        }
        D_800A3670 = 1;
        D_800A38DF = func_80022408(&g_practice_menu_table[D_800A3748].unk_F4.x);
        D_800A3834 = 0;
        goto end;
    case 4:
    case 6:
        {
            s16 t;
            u8 *p;
            if ((t = g_practice_menu_table[0].unk_96) == 0 || g_practice_menu_table[1].unk_96 == 0) {
                /* FAKE: second handle, as above (owner Q63). */
                p = &D_800A37D2;
                /* FAKE: indexes past D_800A37D2 into D_800A37D3, as above (owner Q63). */
                p[t != 0]++;
            }
        }
        break;
    case 5:
        goto end;
    }
    D_800A3834 = 4;
end:
    if (D_800A37D2 >= 100) {
        D_800A37D2 = 99;
    }
    if (D_800A37D3 >= 100) {
        D_800A37D3 = 99;
    }
}
