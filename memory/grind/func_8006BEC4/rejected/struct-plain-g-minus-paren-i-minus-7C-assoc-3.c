typedef struct Tile {
    s32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} Tile;
extern s32 D_800A36AC;
extern s32 D_800A374C;
extern s32 D_800A34FC;
extern s32 D_800A3900;
extern Tile *D_800A36DC;
extern u8 *D_800A36E0;
extern u8 *D_800A36E4;
extern u8 D_800F11E0[];
extern u8 D_800F1438[];
extern Tile D_800F1498[];
extern void func_8006BD28(s32, s32, s32 *, s32);
extern s32 SetTile(void *);
extern s32 SetSemiTrans(void *, s32);
extern s32 AddPrim(s32, void *);
void func_8006BEC4(s32 arg0, s32 arg1) {
    s32 sp10[12];
    s32 par;
    Vec2s16 *pos;
    s16 i;
    s32 r;
    s32 w;
    s32 h;
    s32 x0;

    D_800A3900 = 0;
    par = D_800A36AC & 1;
    D_800A36E4 = D_800F11E0 + par * 0x12C;
    D_800A36E0 = D_800F1438 + par * 0x30;
    D_800A36DC = D_800F1498 + par * 4;
    func_8006BD28(arg0, 0, sp10, 0);
    h = 0;
    pos = *(Vec2s16 **)(*(s32 *)(D_800A34FC + 0x24) + 0x48);
    pos += arg0;
    if (arg1 != -1) {
        w = arg1 ? 0x1F : 0x28;
        h = 0x10;
        D_800A3900 = (pos->y + 0x10) >> 1;
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
            D_800A36DC->r0 = r;
            D_800A36DC->g0 = 0;
            D_800A36DC->b0 = 0;
            {
            Tile *p = D_800A36DC;
            p->y0 = D_800A3900 - (i - 0x7C);
            p->x0 = x0;
            p->w = w;
            p->h = 1;
            AddPrim(D_800A374C + 0x20, p);
            }
            D_800A36DC++;
        }
    }
    SetTile(D_800A36DC);
    D_800A36DC->r0 = 0;
    D_800A36DC->g0 = 0;
    D_800A36DC->b0 = 0;
    {
    Tile *p = D_800A36DC;
    p->x0 = 0x140 - (pos->x >> 1);
    p->y0 = 0x78 - (pos->y >> 1);
    p->w = pos->x;
    p->h = pos->y + h;
    SetSemiTrans(p, 1);
    }
    AddPrim(D_800A374C + 0x20, D_800A36DC);
    D_800A36DC++;
}
