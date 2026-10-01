void func_8001E404(void) {
    /* FAKE: unwritten leading pad ([[dead-vars-local-array]] re-scoped carve-out, owner ruling 2026-08-17): reconstructs the original frame's 8-byte allocated-but-untouched leading region (outgoing-args partition 24 vs 16, proven by frame-term forensics in memory/grind/func_8001E404/); SOTN-master precedent: volatile u32 pad[4]; // FAKE at st/sel/stream.c:80. Sanctioned for func_8001E404/func_8001E6E4/func_8003CF84 ONLY. */
    volatile u32 pre_pad[2];
    Rec44 local;
    Rec44 *s2;

    if (D_800A38BA != 0) {
        s32 v3 = D_800A36FA;
        if (v3 == 1) {
            if (g_practice_menu_table[0].unk_96 != 0 || g_practice_menu_table[1].unk_96 != 0) {
                D_800A36FA = 2;
            }
        }
        if (D_800A36FA == 2) goto s2_default;
        if (g_practice_menu_table[0].unk_6A == 0x11 || g_practice_menu_table[1].unk_6A == 0x11) {
            s2 = &D_800F6608;
            D_800A36FA = 1;
        } else {
            s2 = &D_800F5328;
            D_800A36FA = 0;
        }
        goto done_s2;
    s2_default:
        s2 = &D_800F6608;
    done_s2:

        func_8003F218(D_800A36FA < 1);

        {
            s32 fov = 0x2D;
            if (D_800A36FA == 0) {
                fov = 0x50;
            }
            SetGeomScreen(math_FovToScreenDist(fov));
        }

        if (D_800A36FA == 0) {
            func_80041688(D_800A36F6, 1);
            func_80041688(D_800A36F6 == 0, 0);
        } else {
            func_80041688(0, 0);
            func_80041688(1, 0);
        }
        goto common_tail;
    }
    s2 = &D_800F6608;
common_tail:

    if (D_800A3834 == 1) {
        local.w0 = s2->w0 + D_800FF5C8;
        local.w4 = s2->w4 + D_800FF5CC;
        local.w8 = s2->w8 + D_800FF5D0;
        local.h10 = (u16)s2->h10 + (u16)D_800FF5D8;
        local.h12 = (u16)s2->h12 + (u16)D_800FF5DA;
        local.h14 = (u16)s2->h14 + (u16)D_800FF5DC;
        local.w18 = s2->w18 + D_800FF5E0;
    } else {
        local = *s2;
    }

    func_80046BF4(&local.w0, (s32 *)&local.h10, local.w18);
    {
        s32 *p20 = &s2->w20;
        func_8001A538(&local.w0, p20);
        func_80061064((s32 *)&local.h10, p20);
    }
    func_8003F3D4(s2->h30[0]);
    func_8003F3D4(s2->h30[1]);
    D_800A36B4 = (s32)s2;
}
