void func_80022580(s32 idx, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    PracticeMenuRec *p;
    Vec3i32 other;
    s32 level;
    s32 ang;
    s32 i;
    s32 slot;

    p = &g_practice_menu_table[idx];
    D_800A3758 = 0xFF;
    D_800A376E = 0;
    p->unk_3C = 0;
    p->unk_00 = (idx != 0) ? &g_practice_menu_table[0] : &g_practice_menu_table[1];
    p->unk_04 = idx;
    p->unk_06 = arg1;
    p->unk_0C = arg2;
    p->unk_0A = D_8008D538[arg2];
    p->unk_0E = arg3;

    if (D_800A38DC == 5 || (D_800A38DC == 2 && D_800A389A == 0)) {
        p->unk_12 = D_8008EB38[p->unk_0E];
    } else if (idx == 1 && D_800A38DC == 3) {
        p->unk_12 = D_8008EB28[p->unk_0E][D_8008D9EC[p->unk_00->unk_0A] == 0];
    } else if (D_800A38DC == 0 && D_800A385C != 0 && idx == 0) {
        p->unk_12 = 0x32;
    } else {
        p->unk_12 = D_8008EB28[p->unk_0E][D_8008D9EC[p->unk_0A]];
    }

    if (idx == 1 && D_800A38DC == 3) {
        p->unk_1A = D_80094C68[D_800A38DE];
    } else {
        p->unk_1A = D_80094C68[D_8008D578[p->unk_0C]];
    }
    p->unk_1C = D_8008DE34[p->unk_0A][p->unk_0E];
    p->unk_1E = D_8008DF78[p->unk_0A][p->unk_0E];
    p->unk_20 = p->unk_1E;
    p->unk_08 = 0x1000;

    if (D_800A38DC == 3 && idx == 1) {
        p->unk_84 = func_800224E0((s32 *)p);
        level = (D_800A38E2 - 1) / 10 + 1;
        p->unk_1C = level * 96 + ((level == 9) ? 0xA00 : 0x800);
        if (level == 9) {
            p->unk_20 = 0x1080;
        } else {
            p->unk_20 = level * 64 + 0xC00;
        }
        if (level == 9) {
            p->unk_08 = 0xE00;
        } else {
            p->unk_08 = level * 80 + 0x150;
        }
    } else {
        p->unk_84 = D_8008DD5C[p->unk_0A][p->unk_0E];
    }

    p->unk_88 = -1;
    p->unk_8A = 0;
    p->unk_8E = -1;
    p->unk_90 = 0;
    slot = (D_800A3670 != 0) ? D_800A38DF : 0;

    switch (D_800A38DC) {
    case 0:
        if (arg4 != 0) {
            other = p->unk_00->unk_D8;
            func_80022224(idx, &p->unk_D8.x, &other.x);
        } else {
        c0_calls:
            func_80021D10(idx, &p->unk_D8.x, slot);
            func_80021D10(idx == 0, &other.x, slot);
        }
        break;
    case 2:
    case 3:
        if (arg4 != 0) {
            other = p->unk_00->unk_D8;
            func_80021DB0(idx, &p->unk_D8, &other.x);
        } else {
            func_80021D10(idx, &p->unk_D8.x, D_800A38E0);
            func_80021D10(idx == 0, &other.x, D_800A38E0);
        }
        break;
    default:
        goto c0_calls;
    }

    p->unk_E8.x = 0;
    p->unk_E8.y = -0x384;
    p->unk_E8.z = 0;
    p->unk_F4.x = p->unk_D8.x + p->unk_E8.x;
    p->unk_F4.y = p->unk_D8.y + p->unk_E8.y;
    p->unk_F4.z = p->unk_D8.z + p->unk_E8.z;
    other.x += p->unk_E8.x;
    other.y += p->unk_E8.y;
    other.z += p->unk_E8.z;
    p->unk_B8.vx = p->unk_F4.x;
    p->unk_B8.vy = p->unk_D8.y;
    p->unk_B8.vz = p->unk_F4.z;
    p->unk_C8 = p->unk_B8;
    p->unk_1F8 = p->unk_E8;
    p->unk_104.vx = 0;
    p->unk_104.vy = 0;
    p->unk_104.vz = 0;
    p->unk_24C = p->unk_104;
    p->unk_114.vx = 0;
    p->unk_114.vy = 0;
    p->unk_114.vz = 0;
    p->unk_124.vx = 0;
    p->unk_124.vy = 0;
    p->unk_124.vz = 0;
    p->unk_134.vx = 0;
    p->unk_134.vy = 0;
    p->unk_134.vz = 0;
    p->unk_144 = 0;
    p->unk_148 = p->unk_B8.vy;
    p->unk_14C = 0;
    p->unk_150 = 0;
    p->unk_152 = 0;
    p->unk_14E = 0;
    p->unk_156 = 0xC00;
    p->unk_158 = 0xF80;
    p->unk_15A = 0xC00;
    p->unk_15E = 0xC00;
    p->unk_160 = 0xC00;
    p->unk_162 = 0xC00;
    p->unk_1E8 = 0;
    p->unk_1E6 = 0;
    p->unk_1EA = 0;
    p->unk_268 = 0;
    p->unk_168 = p->unk_F4;
    p->unk_174 = p->unk_F4;
    p->unk_180 = p->unk_F4;
    p->unk_18C = p->unk_F4;
    p->unk_1C8.vx = 0;
    p->unk_1C8.vy = 0;
    p->unk_1C8.vz = 0;
    ang = ratan2(other.x - p->unk_F4.x, other.z - p->unk_F4.z);
    p->unk_1C8.vy = p->unk_1D8 = ang;
    if (p->unk_0C == 0x1F) {
        p->unk_1C8.vy = ang + 0x800;
    }
    p->unk_1D0 = p->unk_1C8;
    func_800206B0(idx, p->unk_1A);
    if (D_800A38DC != 0 || idx != 0 || D_800A3907 < 2 || D_800A3670 != 0) {
        func_80022568((s16 *)p);
    }
    p->unk_274 = D_8008E3C0[p->unk_0A];
    for (i = 0; i < 4; i++) {
        p->unk_276[i] = D_8008E3F8[p->unk_0A][i];
        p->unk_27E[i] = D_8008E4D0[p->unk_0A][i];
    }
    p->unk_A0 = 8;
    p->unk_7C = 0;
    p->unk_286 = -1;
    p->unk_31A = 0;
    p->unk_1DC = 0;
    p->unk_72 = 0;
    p->unk_96 = 0;
    p->unk_B1 = 7;
    p->unk_B2 = 0;
    p->unk_34C = 0;
    func_8003047C(p);
    p->unk_14 = p->unk_332;
    if (D_800A38DC == 5 || (D_800A38DC == 2 && D_800A389A == 0)) {
        if (p->unk_0A == 1 || p->unk_0A == 3 || p->unk_0A == 4 || p->unk_0A == 9 || p->unk_0A == 0x11) {
            p->unk_332 = p->unk_14 = 0x11;
        } else {
            p->unk_14 = -1;
            p->unk_330 = 0;
        }
    } else if (D_800A38DC == 3 && idx == 1 && D_800A384C != 4) {
        p->unk_14 = -1;
        p->unk_330 = 0;
    } else if (D_800A38DC == 3 && idx == 1 && D_800A384C == 4) {
        if (p->unk_00->unk_0A == 1 || p->unk_00->unk_0A == 3 || p->unk_00->unk_0A == 4 ||
            p->unk_00->unk_0A == 9 || p->unk_00->unk_0A == 0x11) {
            p->unk_332 = p->unk_14 = p->unk_00->unk_14;
            p->unk_330 = 1;
        }
    } else {
        if (p->unk_0C == 0x1D) {
            p->unk_14 = 0x1F;
            p->unk_34A = 2;
        } else if (p->unk_0C == 0xE) {
            p->unk_14 = 0x1E;
            p->unk_34A = 2;
        } else {
            p->unk_34A = 0xA;
        }
        p->unk_34B = 9;
    }
    if (D_800A38DC == 5 || D_800A38DC == 2 || D_800A38DC == 3 || (D_800A38DC == 0 && D_800A385C != 0)) {
        p->unk_34D = 0;
    } else {
        p->unk_34D = 2;
    }
    p->unk_350 = 0;
}
