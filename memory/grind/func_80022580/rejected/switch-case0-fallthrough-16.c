typedef struct { s32 x, y, z; } Vec3i32;
typedef struct { s32 vx, vy, vz, pad; } Vec4i32;
typedef struct { s16 vx, vy, vz, pad; } SVec4i16;

typedef struct PR2 {
    struct PR2 *unk_00; /* the other record: [1] for record 0, else [0] (func_80022580) */
    s16 unk_04;                    /* this record's own index (func_80022580) */
    s16 unk_06;                    /* != 0: func_8001BE20 hands pad input to func_80055B60 */
    s16 unk_08;
    s16 unk_0A;                    /* class idx: row of D_8008E5CC / D_8008E6A4, index of D_8008D9EC */
    s16 unk_0C;
    s16 unk_0E;                    /* column of D_8008E5CC / D_8008E6A4 */
    u8  unk_10[0x12 - 0x10];
    s16 unk_12;
    s16 unk_14;                    /* -1 == none, else index of D_8008EB80 */
    u8  unk_16[0x1A - 0x16];
    s16 unk_1A;
    s16 unk_1C;
    s16 unk_1E;
    s16 unk_20;
    u8  unk_22[0x3C - 0x22];
    s32 unk_3C;
    u8  unk_40[0x5E - 0x40];
    s16 unk_5E;                    /* 0/1, set alongside func_80021A98 */
    u8  unk_60[0x72 - 0x60];
    s16 unk_72;
    u8  unk_74[0x7C - 0x74];
    s32 unk_7C;
    u8  unk_80[0x84 - 0x80];
    s16 unk_84;
    u8  unk_86[0x88 - 0x86];
    s16 unk_88;
    s16 unk_8A;
    u8  unk_8C[0x8E - 0x8C];
    s16 unk_8E;
    s16 unk_90;
    u8  unk_92[0x96 - 0x92];
    s16 unk_96;
    u8  unk_98[0xA0 - 0x98];
    u8  unk_A0;
    u8  unk_A1[0xB1 - 0xA1];
    u8  unk_B1;
    u8  unk_B2;
    u8  unk_B3[0xB8 - 0xB3];
    Vec4i32 unk_B8;
    Vec4i32 unk_C8;
    Vec3i32 unk_D8;
    u8  unk_E4[0xE8 - 0xE4];
    Vec3i32 unk_E8;
    Vec3i32 unk_F4;
    u8  unk_100[0x104 - 0x100];
    Vec4i32 unk_104;
    Vec4i32 unk_114;
    Vec4i32 unk_124;
    Vec4i32 unk_134;
    s32 unk_144;
    s32 unk_148;
    s16 unk_14C;
    s16 unk_14E;
    s16 unk_150;
    s16 unk_152;
    u8  unk_154[0x156 - 0x154];
    s16 unk_156;
    s16 unk_158;
    s16 unk_15A;
    u8  unk_15C[0x15E - 0x15C];
    s16 unk_15E;
    s16 unk_160;
    s16 unk_162;
    u8  unk_164[0x168 - 0x164];
    Vec3i32 unk_168;
    Vec3i32 unk_174;
    Vec3i32 unk_180;
    Vec3i32 unk_18C;
    u8  unk_198[0x1C8 - 0x198];
    SVec4i16 unk_1C8;
    SVec4i16 unk_1D0;
    s16 unk_1D8;
    u8  unk_1DA[0x1DC - 0x1DA];
    s16 unk_1DC;
    u8  unk_1DE[0x1E6 - 0x1DE];
    s16 unk_1E6;
    s16 unk_1E8;
    s16 unk_1EA;
    u8  unk_1EC[0x1F8 - 0x1EC];
    Vec3i32 unk_1F8;
    u8  unk_204[0x24C - 0x204];
    Vec4i32 unk_24C;
    u8  unk_25C[0x268 - 0x25C];
    s32 unk_268;
    u8  unk_26C[0x274 - 0x26C];
    s16 unk_274;
    s16 unk_276[4];
    s16 unk_27E[4];
    s16 unk_286;
    u8  unk_288[0x31A - 0x288];
    s16 unk_31A;
    u8  unk_31C[0x330 - 0x31C];
    s16 unk_330;
    s16 unk_332;
    u8  unk_334[0x34A - 0x334];
    u8  unk_34A;
    u8  unk_34B;
    u8  unk_34C;
    u8  unk_34D;
    u8  unk_34E;                   /* written by func_8001BE20 for the OTHER record */
    u8  unk_34F[0x350 - 0x34F];
    s16 unk_350;
    u8  unk_352[0x44C - 0x352];
} PR2;                 /* sizeof == 0x44C */
#define g_practice_menu_table ((PR2 *)&D_80101EC8)
#define D_8008EB38 (&D_8008EB38)
#define D_8008E3C0 (&D_8008E3C0)
#define func_80021DB0(a, b, c) func_80021DB0(a, (Vec3_21DB0 *)(b), c)
extern u8 D_8008DD5C[][8];
extern u16 D_8008DE34[][6];
extern u16 D_8008DF78[][6];
extern u16 D_8008E3F8[][4];
extern u16 D_8008E4D0[][4];
extern u8 D_8008EB28[][2];
void func_80022580(s32 idx, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    PR2 *p;
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
            break;
        }
    default:
        func_80021D10(idx, &p->unk_D8.x, slot);
        func_80021D10(idx == 0, &other.x, slot);
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
#undef g_practice_menu_table
#undef D_8008EB38
#undef D_8008E3C0
#undef func_80021DB0
