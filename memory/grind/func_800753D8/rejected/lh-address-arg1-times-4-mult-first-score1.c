typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S_753D8;

extern u8 *D_800A36A0;
extern s32 D_800A374C;
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 rsin(s32);

void func_800753D8(s32 *arg0, s32 arg1) {
    S_753D8 s;
    s32 *tbl;
    s16 i;
    s32 q;
    u8 *base;
    s32 c;
    s32 zero;

    zero = 0;
    s.sp28 = 0;
    if (arg1 != 0) {
        s.sp2C = 0x16;
    } else {
        s.sp2C = 0xC;
    }
    tbl = *(s32 **)(arg0[0] + 0x2C);
    s.sp18 = tbl[arg1 + 2];
    s.sp30 = arg1 * 240;
    q = s.sp18 + 0xC;
    s.sp1C = q;
    base = D_800A36A0;
    s.sp34 = *(u8 *)(base + arg1 + 0x68) * 90 + *(s16 *)(base + arg1 * 4 + 0x42);
    c = ((rsin(((*(u16 *)(base + 0x34) & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
    s.sp43 = c;
    s.sp42 = c;
    s.sp41 = c;
    s.sp40 = 1;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    s.sp40 = 0;
    tbl = *(s32 **)(arg0[0] + 0x14);
    i = 0;
    s.sp18 = tbl[0];
    s.sp30 = arg1 * 240 + 0x9D;
    s.sp34 = 0x36;
    q = s.sp18 + 0xC;
    s.sp1C = q;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    s.sp34 = 0x90;
    s.sp1C += *(u8 *)(s.sp18 + 2) << 3;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(D_800A374C + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;
    tbl = *(s32 **)(arg0[0] + 0x2C);
    do {
        s.sp18 = tbl[i];
        s.sp30 = arg1 * 240;
        s.sp34 = 0;
        q = s.sp18 + 0xC;
        if (arg1 != 0) {
            s.sp2C = 0x16;
        } else {
            s.sp2C = 0xC;
        }
        s.sp1C = q;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
        AddPrim(D_800A374C + s.sp2C * 4, arg0[6]);
        i++;
        arg0[6] += 0xC;
    } while (i < 2);
}
