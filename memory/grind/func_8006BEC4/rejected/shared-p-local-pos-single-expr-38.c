extern s32 D_800A36AC;
extern s32 D_800A374C;
extern s32 D_800A34FC;
extern s32 D_800A3900;
extern u8 *D_800A36DC;
extern u8 *D_800A36E0;
extern u8 *D_800A36E4;
extern u8 D_800F11E0[];
extern u8 D_800F1438[];
extern u8 D_800F1498[];
extern void func_8006BD28(s32, s32, s32 *, s32);
extern s32 SetTile(void *);
extern s32 SetSemiTrans(void *, s32);
extern s32 AddPrim(s32, void *);
void func_8006BEC4(s32 arg0, s32 arg1) {
    s32 sp10[12];
    s32 par;
    s16 *pos;
    s16 i;
    s32 r;
    s32 w;
    s32 h;
    s32 x0;
    u8 *p;

    D_800A3900 = 0;
    par = D_800A36AC & 1;
    D_800A36E4 = D_800F11E0 + par * 0x12C;
    D_800A36E0 = D_800F1438 + par * 0x30;
    D_800A36DC = D_800F1498 + par * 0x40;
    func_8006BD28(arg0, 0, sp10, 0);
    h = 0;
    pos = (s16 *)(*(s32 *)(*(s32 *)(D_800A34FC + 0x24) + 0x48) + arg0 * 4);
    if (arg1 != -1) {
        w = arg1 ? 0x1F : 0x28;
        h = 0x10;
        D_800A3900 = (pos[1] + 0x10) >> 1;
        func_8006BD28(0x12, D_800A3900, sp10, arg1);
        x0 = (arg1 & 1) * 0x3A + 0x113;
        for (i = 0; i < 3; i++) {
            SetTile(D_800A36DC);
            if (i == 0) {
                r = 0xFF;
                SetSemiTrans(D_800A36DC, 0);
            } else {
                r = 0xFF - ((i - 1) << 7);
                SetSemiTrans(D_800A36DC, 1);
            }
            D_800A36DC[4] = r;
            D_800A36DC[5] = 0;
            D_800A36DC[6] = 0;
            p = D_800A36DC;
            *(s16 *)(p + 0xA) = D_800A3900 - (i - 0x7C);
            *(s16 *)(p + 8) = x0;
            *(s16 *)(p + 0xC) = w;
            *(s16 *)(p + 0xE) = 1;
            AddPrim(D_800A374C + 0x20, p);
            D_800A36DC += 0x10;
        }
    }
    SetTile(D_800A36DC);
    D_800A36DC[4] = 0;
    D_800A36DC[5] = 0;
    D_800A36DC[6] = 0;
    p = D_800A36DC;
    *(s16 *)(p + 8) = 0x140 - (pos[0] >> 1);
    *(s16 *)(p + 0xA) = 0x78 - (pos[1] >> 1);
    *(s16 *)(p + 0xC) = pos[0];
    *(s16 *)(p + 0xE) = pos[1] + h;
    SetSemiTrans(p, 1);
    AddPrim(D_800A374C + 0x20, D_800A36DC);
    D_800A36DC += 0x10;
}
