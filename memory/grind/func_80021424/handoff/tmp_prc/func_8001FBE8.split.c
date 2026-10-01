void func_8001FBE8(void) {
    PracticeMenuRec *rec;
    PracticeMenuRec *sel;
    StatusEvt *ent;
    u8 *data;
    u8 *snd;
    s32 lo;
    s32 hi;
    s32 dz;
    s32 i;
    u16 kind;
    s32 pos[3];

    if (D_800A376E != 0) {
        D_800A376E = 0;
        D_800A38E8 = 0xFF;
        if (g_practice_menu_table[0].unk_96 != 0) {
            return;
        }
        if (g_practice_menu_table[1].unk_96 != 0) {
            return;
        }
        func_80021A98(D_800A38AE, (u8 *)D_800A36D8, D_800A381C);
        if (D_800A36CA & 0x1000) {
            g_practice_menu_table[D_800A38AE == 0].unk_4C = 1;
        }
        func_80021A98(D_800A38AE == 0, (u8 *)D_800A36D8, D_800A381C);
        g_practice_menu_table[1].unk_7A = 2;
        g_practice_menu_table[0].unk_7A = 2;
        return;
    }
    if (D_800A3758 != 0xFF) {
        sel = &g_practice_menu_table[D_800A3758];
        if (D_800A3769 != 0) {
            sel->unk_286 = 1;
            sel->unk_94 = 0;
        } else {
            sel->unk_286 = 0;
            sel->unk_94 = 1;
        }
        if (sel->unk_96 != 0) {
            sel->unk_286 += 2;
        }
        sel->unk_74 = sel->unk_B8.vy;
        sel->unk_00->unk_286 = 1;
        sel->unk_00->unk_94 = 0;
        sel->unk_00->unk_74 = sel->unk_00->unk_B8.vy;
        if (sel->unk_00->unk_96 != 0) {
            sel->unk_00->unk_286 += 2;
        }
        D_800A3758 = 0xFF;
        return;
    }
    if (g_practice_menu_table[0].unk_286 != -1) {
        return;
    }
    if (g_practice_menu_table[1].unk_286 != -1) {
        return;
    }
    for (i = 0; i < 2; i++) {
        rec = &g_practice_menu_table[i];
        if (rec->unk_7A == 0) {
            continue;
        }
        ent = (StatusEvt *)func_8001FAE4(rec->unk_50);
        if (ent == 0) {
            continue;
        }
        data = ent->b;
        lo = data[0] * 20;
        hi = data[1] * 20;
        dz = rec->unk_B8.vy - rec->unk_00->unk_B8.vy;
        if (func_8001FB34((s32 *)rec, data[3] & 0x80) == 0) {
            continue;
        }
        if (D_800A387C < lo) {
            continue;
        }
        if (hi < D_800A387C) {
            continue;
        }
        if (dz <= -100 || dz >= 100) {
            continue;
        }
        kind = rec->unk_00->unk_6A;
        if (kind != 0x15 && kind != 0x2C && kind != 0xE && kind != 0x19) {
            continue;
        }
        D_800A38AE = i;
        D_800A376E = 0;
        D_800A3758 = 0xFF;
        D_800A371C = data[2] * 20;
        D_800A38E8 = data[3] & 0x7F;
        snd = func_80021424(rec, ent->id, &rec->unk_5E);
        func_80021A98(i, snd, rec->unk_5E);
        rec->unk_00->unk_4C = 1;
        rec->unk_00->unk_5E = rec->unk_5E;
        func_80021A98(i == 0, snd, rec->unk_5E);
        rec->unk_7A = 2;
        rec->unk_00->unk_7A = 2;
        rec->unk_00->unk_86 = rec->unk_00->unk_84;
        rec->unk_00->unk_272 += 1;
        pos[0] = (rec->unk_F4.x + rec->unk_00->unk_F4.x) / 2;
        pos[1] = (rec->unk_F4.y + rec->unk_00->unk_F4.y) / 2;
        pos[2] = (rec->unk_F4.z + rec->unk_00->unk_F4.z) / 2;
        func_80032854(i, 0x10, pos, 0);
        return;
    }
}
