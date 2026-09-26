typedef struct {
    u8 rgb[3];
    u8 flags;
    u8 unk_4[8];
} SaveCfg;
extern SaveCfg g_default_color_r;
typedef struct {
    u16 size[2];
    u8 unk_0[2];
    u8 unk_2[2];
    u8 unk_4[2];
    u8 unk_6[2];
    u8 count_a;
    u8 count_b;
    u8 flag_a;
    u8 flag_b;
} LessonParams;
extern LessonParams g_practice_lesson_size_a;
#define LP g_practice_lesson_size_a
extern s16 D_800A3174[2];
extern u8 D_800A3178[];
extern u8 D_800A3180[];
extern u8 D_800A3188[];
extern u8 D_800A3190[];
extern u8 D_800A3198[];
extern u8 D_800A31A0[];
extern u8 D_800A31A8[];
extern u8 D_800A31B0[];
extern u8 D_800A31B8[];
extern u8 D_800A31C0[];
extern u8 D_800A31C8[];
extern u8 D_800A31D0[];
extern u8 D_80010834[];
extern u8 D_80010840[];

void func_80034708(void) {
    s32 i;
    u8 *off;
    u8 *on;

    D_800A37B8++;
    rand();
    off = D_800A3178;
    on = D_800A3180;
    func_8003D52C(D_800A3188, (s32)(D_800A3174[0] == 0 ? on : off), (s8)LP.unk_0[0]);
    func_8003D52C(D_800A3190, (s32)(D_800A3174[1] == 0 ? on : off), (s8)LP.unk_0[1]);
    func_8003D52C(D_800A3188, (s32)(D_800A3174[0] == 1 ? on : off), (s8)LP.unk_2[0]);
    func_8003D52C(D_800A3190, (s32)(D_800A3174[1] == 1 ? on : off), (s8)LP.unk_2[1]);
    func_8003D52C(D_800A3198, (s32)(D_800A3174[0] == 2 ? on : off), (s8)LP.unk_4[0]);
    func_8003D52C(D_800A3190, (s32)(D_800A3174[1] == 2 ? on : off), (s8)LP.unk_4[1]);
    func_8003D52C(D_800A31A0, (s32)(D_800A3174[0] == 3 ? on : off), LP.size[0] >> 8);
    func_8003D52C(D_800A31A8, (s32)(D_800A3174[1] == 3 ? on : off), LP.size[1] >> 8);
    func_8003D52C(D_800A31B0, (s32)(D_800A3174[0] == 4 ? on : off), (s8)LP.count_a);
    func_8003D52C(D_800A31B0, (s32)(D_800A3174[0] == 5 ? on : off), (s8)LP.count_b);
    func_8003D52C(D_800A31B8, (s32)(D_800A3174[0] == 6 ? on : off), (s8)LP.flag_a);
    func_8003D52C(D_80010834, (s32)(D_800A3174[0] == 7 ? on : off), (s8)LP.flag_b);
    func_8003D52C(D_80010840, (s32)(D_800A3174[0] == 8 ? on : off), g_default_color_r.flags & 1);
    func_8003D52C(D_800A31C0, (s32)(D_800A3174[0] == 9 ? on : off), (g_default_color_r.flags >> 1) & 1);
    func_8003D52C(D_800A31C8, (s32)(D_800A3174[0] == 10 ? on : off), D_800A36F9);
    func_8003D52C(D_800A31D0, (s32)(D_800A3174[0] == 11 ? on : off), D_800A3690);

    for (i = 0; i < 2; i++) {
        if (D_80102788.pressed & (0x1000 << (i * 16))) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3174[i] > 0) {
                D_800A3174[i]--;
            } else {
                D_800A3174[i] = (i != 0) ? 3 : 11;
            }
        } else if (D_80102788.pressed & (0x4000 << (i * 16))) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3174[i] < ((i != 0) ? 3 : 11)) {
                D_800A3174[i]++;
            } else {
                D_800A3174[i] = 0;
            }
        } else if (D_80102788.pressed & (0x8000 << (i * 16))) {
            func_8005C650(4, 0x3F, 0x3F);
            switch (D_800A3174[i]) {
            case 0:
                LP.unk_0[i]--;
                break;
            case 1:
                LP.unk_2[i]--;
                break;
            case 2:
                LP.unk_4[i]--;
                break;
            case 3:
                LP.size[i] -= 0x80;
                break;
            case 4:
                LP.count_a--;
                break;
            case 5:
                LP.count_b--;
                break;
            case 6:
                LP.flag_a--;
                break;
            case 7:
                LP.flag_b--;
                break;
            case 8:
                g_default_color_r.flags ^= 1;
                break;
            case 9:
                g_default_color_r.flags ^= 2;
                break;
            case 10:
                D_800A36F9--;
                break;
            case 11:
                D_800A3690--;
                break;
            }
        } else if (D_80102788.pressed & (0x2000 << (i * 16))) {
            func_8005C650(4, 0x3F, 0x3F);
            switch (D_800A3174[i]) {
            case 0:
                LP.unk_0[i]++;
                break;
            case 1:
                LP.unk_2[i]++;
                break;
            case 2:
                LP.unk_4[i]++;
                break;
            case 3:
                LP.size[i] += 0x80;
                break;
            case 4:
                LP.count_a++;
                break;
            case 5:
                LP.count_b++;
                break;
            case 6:
                LP.flag_a++;
                break;
            case 7:
                LP.flag_b++;
                break;
            case 8:
                g_default_color_r.flags ^= 1;
                break;
            case 9:
                g_default_color_r.flags ^= 2;
                break;
            case 10:
                D_800A36F9++;
                break;
            case 11:
                D_800A3690++;
                break;
            }
        }
        LP.unk_0[i] = ((s8)LP.unk_0[i] + 33) % 33;
        LP.unk_2[i] = ((s8)LP.unk_2[i] + 8) % 8;
        LP.unk_4[i] = ((s8)LP.unk_4[i] + 2) % 2;
    }
    LP.count_a = ((s8)LP.count_a + 38) % 38;
    LP.count_b = ((s8)LP.count_b + 7) % 7;
    LP.flag_a &= 1;
    LP.flag_b &= 1;
    D_800A36F9 = (D_800A36F9 + 4) % 4;
    D_800A3690 &= 1;
    if (D_80102788.pressed & 0x08000800) {
        func_8005C650(1, 0x7F, 0x7F);
        func_800344B4();
    }
}
