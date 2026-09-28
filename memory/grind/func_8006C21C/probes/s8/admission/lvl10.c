/* func_8006C21C checkpoint s7 (Codex, 2026-09-28): score 41 / 622 insns.
 * INCOMPLETE; not approved to land. See evidence.md s7 and review-s7.md.
 * Removed rejected s6 col carrier. Chained corner colors recover the s5 score.
 * Removed the entire next/k cluster and split the tile-record pointer, without
 * increasing the score. Header/cell addresses now use truthful byte pointers;
 * descriptor color bytes are explicit. Four frame slots and two store-order
 * hunks remain. The i reuse, cells reuse, and mode holder still need their full
 * respective admission evidence; this checkpoint does not claim approval.
 * Trailing macro remains SANDBOX ONLY: fix the TU prototype at any landing.
 * s8 (2026-09-28): bar 1's vertices use PsyQ setXYWH (byte-neutral, 41/622).
 */
typedef struct {
    u8 *header;
    u8 *table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 pad20;
    s32 pad24;
    u8 has_color;
    u8 col_r, col_g, col_b;
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

#define setXYWH(p, _x0, _y0, _w, _h)     (p)->x0 = (_x0), (p)->y0 = (_y0),     (p)->x1 = (_x0) + (_w), (p)->y1 = (_y0),     (p)->x2 = (_x0), (p)->y2 = (_y0) + (_h),     (p)->x3 = (_x0) + (_w), (p)->y3 = (_y0) + (_h)
void func_8006C21C(s32 *arg0) {
    Env_8006C21C s;
    s32 j;
    s32 *table;
    Rec_8006C21C *recs;
    Rec_8006C21C *rec;
    Rec_8006C21C *tile_rec;
    Tile *tile;
    PolyG4_8006C21C *poly;
    s32 mode;
    s32 i;
    s32 level;
    s32 pl;
    s32 x;
    s32 row;
    s32 pulse;
    u8 *cells;

    mode = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.y = 0;
    s.x = 0;
    s.header = (u8 *)table[0];
    s.semi = 0;
    cells = s.header + 0xC;
    s.table = cells;
    s.out = arg0[5];
    arg0[5] = func_8007352C((s32)&s);
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, mode), 0);
    AddPrim(g_gpu_ot_ptr + 0x28, arg0[7]);
    arg0[7] += 0xC;

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
                    s.header = (u8 *)table[i + 13];
                    s.semi = 0;
                    cells = s.header + 0xC;
                    s.table = cells;
                    s.out = arg0[5];
                    arg0[5] = func_8007352C((s32)&s);
                }
            }
        }
    }
    s.header = (u8 *)table[13];
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, mode), 0);
    AddPrim(g_gpu_ot_ptr + 0x24, arg0[7]);
    arg0[7] += 0xC;

    s.x = 0;
    s.y = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.header = (u8 *)table[1];
    s.semi = 0;
    cells = s.header + 0xC;
    s.table = cells;
    s.out = arg0[5];
    arg0[5] = func_8007352C((s32)&s);
    for (i = 0; i < 6; i++) {
        s.header = (u8 *)table[i + 2];
        s.ot_idx = 10;
        s.has_color = 0;
        s.semi = 0;
        s.y = 0;
        cells = s.header + 0xC;
        s.table = cells;
        for (j = 0; j < 2; j++) {
            s.x = j ? 280 : 0;
            s.out = arg0[5];
            arg0[5] = func_8007352C((s32)&s);
        }
    }
    table = *(s32 **)(arg0[1] + 0x30);
    s.header = (u8 *)table[1];
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, mode), 0);
    AddPrim(g_gpu_ot_ptr + 0x28, arg0[7]);
    arg0[7] += 0xC;

    recs = *(Rec_8006C21C **)(*(s32 *)(D_800A34FC + 0x24) + 0x44);
    tile = (Tile *)arg0[6];
    for (i = 0; i < 11; i++) {
        tile_rec = &recs[i];
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = tile_rec->r;
            tile->g0 = tile_rec->g;
            tile->b0 = tile_rec->b;
            tile->x0 = tile_rec->x + j * 280;
            tile->y0 = tile_rec->y;
            tile->w = tile_rec->w;
            tile->h = tile_rec->h;
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

        level = *(s16 *)(D_800A34FC + j * 2 + 0x28);
        rec = &recs[level + 1];
        for (row = 0; row < 2; row++) {
            SetPolyG4(poly);
            if (level == 5) {
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
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w, -4 * row + 2);
            } else {
                SetSemiTrans(poly, 0);

                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = pulse;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = poly->r2 = 0x80;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w, -2 * row + 1);
            }
            AddPrim(g_gpu_ot_ptr + 0x20, (s32)poly);
            poly++;
            SetPolyG4(poly);
            if (level == 5) {
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
                poly->r3 = poly->r1 = 0x80;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (-4 * row + 2);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (-4 * row + 2);
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
