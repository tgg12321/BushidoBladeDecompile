typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

extern s32 func_8006E480(s32, s32);
extern s32 func_8007352C(s32 *);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 D_800A3608;
extern s32 *D_800A3610;
extern s32 D_800A374C;

typedef struct {
    s32 a;
    s32 b;
    s32 c;
    s32 d;
    s32 e;
    s32 f;
    s32 g;
    s32 h;
    s32 i;
    s32 j;
    u8 cd_flag;
    u8 r;
    u8 g_;
    u8 b_;
} S78654;

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    s32 *ot;
    s32 zero;

    zero = 0;
    ot = arg0 + 5;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(*ot, 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), *ot);
        *ot = *ot + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(*ot, 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), *ot);
    var_s0++;
    *ot = *ot + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
