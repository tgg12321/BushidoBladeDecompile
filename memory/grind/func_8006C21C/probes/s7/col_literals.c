typedef struct {
    s32 *header;
    s32 table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 pad20;
    s32 pad24;
    u8 has_color;
} Env_8006C21C;

typedef struct {
    s16 x, y, w, h;
    u8 r, g, b, pad;
} Rec_8006C21C;

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
} PolyG4_8006C21C;

void func_8006C21C(s32 *arg0) {
    Env_8006C21C s;
    s32 j;
    s32 *table;
    Rec_8006C21C *recs;
    Rec_8006C21C *rec;
    Rec_8006C21C *next;
    Tile *tile;
    PolyG4_8006C21C *poly;
    s32 mode;
    s32 i;
    s32 pl;
    s32 x;
    s32 row;
    s32 pulse;
    s32 cells;

    s.ot_idx = 10;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.y = 0;
    s.x = 0;
    s.header = (s32 *)table[0];
    s.semi = 0;
    cells = (s32)s.header + 0xC;
    s.table = cells;
    s.out = arg0[5];
    arg0[5] = func_8007352C((s32)&s);
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x28, arg0[7]);
    arg0[7] += 0xC;

    mode = 0;
    s.ot_idx = 9;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.y = 0;
    for (pl = 0; pl < 2; pl++) {
        s.x = pl * 280;
        if (*(s16 *)(D_800A34FC + pl * 2 + 0x28) < 3) {
            for (i = 0; i < 4; i++) {
                if (*(u8 *)(D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17) &
                    ((1 << i) << (pl * 4))) {
                    s.header = (s32 *)table[i + 13];
                    s.semi = 0;
                    cells = (s32)s.header + 0xC;
                    s.table = cells;
                    s.out = arg0[5];
                    arg0[5] = func_8007352C((s32)&s);
                }
            }
        }
    }
    s.header = (s32 *)table[13];
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, mode), 0);
    AddPrim(g_gpu_ot_ptr + 0x24, arg0[7]);
    arg0[7] += 0xC;

    s.x = 0;
    s.y = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.header = (s32 *)table[1];
    s.semi = 0;
    cells = (s32)s.header + 0xC;
    s.table = cells;
    s.out = arg0[5];
    arg0[5] = func_8007352C((s32)&s);
    for (i = 0; i < 6; i++) {
        s.header = (s32 *)table[i + 2];
        s.ot_idx = 10;
        s.has_color = 0;
        s.semi = 0;
        s.y = 0;
        cells = (s32)s.header + 0xC;
        s.table = cells;
        for (j = 0; j < 2; j++) {
            s.x = j ? 280 : 0;
            s.out = arg0[5];
            arg0[5] = func_8007352C((s32)&s);
        }
    }
    table = *(s32 **)(arg0[1] + 0x30);
    s.header = (s32 *)table[1];
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, mode), 0);
    AddPrim(g_gpu_ot_ptr + 0x28, arg0[7]);
    arg0[7] += 0xC;

    recs = *(Rec_8006C21C **)(*(s32 *)(D_800A34FC + 0x24) + 0x44);
    tile = (Tile *)arg0[6];
    for (i = 0; i < 11; i++) {
        rec = &recs[i];
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = rec->r;
            tile->g0 = rec->g;
            tile->b0 = rec->b;
            tile->x0 = rec->x + j * 280;
            tile->y0 = rec->y;
            tile->w = rec->w;
            tile->h = rec->h;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + 0x30, (s32)tile);
            tile++;
        }
    }
    arg0[6] = (s32)tile;

    pulse = ((rcos((D_800A3518 << 7) & 0xF80) * 32) >> 12) + 0xD0;
    poly = (PolyG4_8006C21C *)arg0[4];
    x = 0;
    for (j = 0; j < 2; j++) {
        s32 k;

        i = *(s16 *)(D_800A34FC + j * 2 + 0x28);
        rec = &recs[i + 1];
        next = rec + 1;
        for (row = 0, k = -1; row < 2; k++, row++) {
            SetPolyG4(poly);
            if (i == 5) {
                SetSemiTrans(poly, 1);

                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r1 = pulse;
                poly->g1 = pulse;
                poly->b1 = pulse;
                poly->r2 = 0;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = 0;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = next[k].x + x;
                poly->y0 = next[k].y - k;
                poly->x1 = next[k].w + (next[k].x + x);
                poly->y1 = next[k].y - k;
                poly->x2 = next[k].x + x;
                poly->y2 = next[k].y - k + (2 + row * -4);
                poly->x3 = next[k].w + (next[k].x + x);
                poly->y3 = next[k].y - k + (2 + row * -4);
            } else {
                SetSemiTrans(poly, 0);

                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = pulse;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r2 = 0x80;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = 0x80;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = next[k].x + x;
                poly->y0 = next[k].y - k;
                poly->x1 = next[k].w + (next[k].x + x);
                poly->y1 = next[k].y - k;
                poly->x2 = next[k].x + x;
                poly->y2 = next[k].y - k + (1 + row * -2);
                poly->x3 = next[k].w + (next[k].x + x);
                poly->y3 = next[k].y - k + (1 + row * -2);
            }
            AddPrim(g_gpu_ot_ptr + 0x20, (s32)poly);
            poly++;
            SetPolyG4(poly);
            if (i == 5) {
                SetSemiTrans(poly, 1);

                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r2 = pulse;
                poly->g2 = pulse;
                poly->b2 = pulse;
                poly->r1 = 0;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = 0;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y3 = rec[1].y - 1;
            } else {
                SetSemiTrans(poly, 0);

                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = pulse;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r1 = 0x80;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = 0x80;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (2 + row * -4);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (2 + row * -4);
                poly->y3 = rec[1].y - 1;
            }
            AddPrim(g_gpu_ot_ptr + 0x20, (s32)poly);
            poly++;
        }
        SetDrawMode(arg0[7], 1, 0, 0x40, 0);
        AddPrim(g_gpu_ot_ptr + 0x20, arg0[7]);
        arg0[7] += 0xC;
        x += 280;
    }
    arg0[4] = (s32)poly;
}
#define func_8006C21C func_8006C21C_sandbox_decl /* SANDBOX-ONLY: masks the later (s32) extern; landing fixes that extern */
