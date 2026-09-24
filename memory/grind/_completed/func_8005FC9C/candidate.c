extern s32 D_800A36AC;
extern u8 g_disp_fb_base;
extern s32 D_800A3278;
extern s32 D_8009B698;
extern s32 D_8009B6B0;
extern s32 SetDrawArea();
extern s32 SetPolyG4();
extern s32 SetSemiTrans(void *, s32);

typedef struct {
    s16 x, y, w, h;
} RectFC9C;

typedef struct {
    RectFC9C clip;
} EnvFC9C;

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 r3, g3, b3, p3;
    s16 x3, y3;
} PolyG4FC9C;

typedef struct {
    s32 *p0;
    s32 *p1;
    s32 in_tex;
    s32 pad0C;
    s32 zero10;
    s32 arg2;
    s32 width;
    s32 height;
    s32 pad20;
    s32 pad24;
    s8 byte28;
} SFC9C;

s32 func_8005FC9C(s32 arg0, s32 arg1)
{
    SFC9C s;
    RectFC9C r;
    RectFC9C *clip;
    EnvFC9C *env;
    s32 cur_tex;
    s32 mode_off;
    PolyG4FC9C *poly;
    s32 area;
    s32 end_off;
    s16 j;
    s16 i;
    s16 off;
    s16 x;
    u8 c;

    cur_tex = arg0;
    mode_off = arg0 + 0x280;
    poly = (PolyG4FC9C *)(arg0 + 0x28C);
    area = arg0 + 0x2D4;
    end_off = arg0 + 0x2F8;
    j = 0;
    env = (EnvFC9C *)(&g_disp_fb_base + (D_800A36AC & 1) * 0x4090);
    r.x = env->clip.x;
    r.y = env->clip.y;
    r.w = env->clip.w;
    r.h = env->clip.h;
    clip = &env->clip;
    SetDrawArea(area, &r);
    AddPrim(D_800A374C + arg1 * 4, area);
    area = arg0 + 0x2E0;
    s.byte28 = 0;
    s.zero10 = 0;
    s.width = 0;
    s.arg2 = arg1;
    s.p1 = &D_8009B6B0;
    off = (D_800A3278 - 0xB4) * 24;
    do {
        if (D_800A3278 >= 0xB5) {
            SetPolyG4(poly);
            SetSemiTrans(poly, 1);
            if (j != 0) {
                x = off + 0x140;
                r.x = clip->x + x;
                r.y = clip->y;
                r.w = clip->w / 2 - off;
                r.h = clip->h;
                poly->x0 = x;
                poly->y0 = 0;
                poly->x1 = x;
                poly->y1 = 0xF0;
                poly->x2 = off + 0x154;
                poly->y2 = 0;
                poly->x3 = off + 0x154;
                poly->y3 = 0xF0;
            } else {
                r.x = clip->x;
                r.y = clip->y;
                r.w = clip->w / 2 - off;
                r.h = clip->h;
                poly->x0 = 0x140 - off;
                poly->y0 = 0;
                poly->x1 = 0x140 - off;
                poly->y1 = 0xF0;
                poly->x2 = 0x12C - off;
                poly->y2 = 0;
                poly->x3 = 0x12C - off;
                poly->y3 = 0xF0;
            }
            c = ~((off * 255) / 320);
            poly->r0 = c;
            poly->g0 = 0;
            poly->b0 = 0;
            poly->r1 = c;
            poly->g1 = 0;
            poly->b1 = 0;
            poly->r2 = 0;
            poly->g2 = 0;
            poly->b2 = 0;
            poly->r3 = 0;
            poly->g3 = 0;
            poly->b3 = 0;
            AddPrim(D_800A374C + arg1 * 4, poly);
            poly++;
        }
        for (i = 0; i < 2; i++) {
            s.p0 = (s32 *)((u8 *)&D_8009B698 + i * 12);
            s.height = i << 6;
            s.in_tex = cur_tex;
            cur_tex = func_8007352C((s32)&s);
        }
        if (D_800A3278 >= 0xB5) {
            SetDrawArea(area, &r);
            AddPrim(D_800A374C + arg1 * 4, area);
            area += 0xC;
        }
        j++;
    } while (j < 2);
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B698, 0x20), 0);
    AddPrim(D_800A374C + arg1 * 4, mode_off);
    if (off <= 0x140) {
        D_800A3278++;
    }
    return end_off - arg0;
}
