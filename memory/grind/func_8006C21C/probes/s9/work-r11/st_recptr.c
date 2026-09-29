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

#define setXYWH(p, _x0, _y0, _w, _h)                                   \
    (p)->x0 = (_x0), (p)->y0 = (_y0),                                    \
    (p)->x1 = (_x0) + (_w), (p)->y1 = (_y0),                             \
    (p)->x2 = (_x0), (p)->y2 = (_y0) + (_h),                             \
    (p)->x3 = (_x0) + (_w), (p)->y3 = (_y0) + (_h)
void func_8006C21C(s32 *arg0) {
    Env_8006C21C s;
    s32 j;
    s32 *table;
    Rec_8006C21C *recs;
    Rec_8006C21C *rec;
    Rec_8006C21C *tile_rec;
    Tile *tile;
    PolyG4_8006C21C *poly;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
       func_8006E480's second argument at all three call sites. Set once and
       live across every call, so global.c gives the once-set constant pseudo
       callee-save $s5 (target: `addu $s5,$zero,$zero`, then `addu $a1,$s5,$zero`
       at the phase-3 and phase-5 calls; cse folds the phase-1 read to 0 inside
       the entry block). Literal 0 at every call measured worse; receipts in
       memory/grind/func_8006C21C/admission.md "mode". Same shape as the
       siblings func_800753D8 (`zero`) and func_8007636C (`mode`). */
    s32 mode;
    s32 sprite;
    s32 trow;
    /* FAKE: per-branch constant holder (named-local-fake-exception, owner
       ruling 2026-09-28 Q27 (B)) -- the bar's far-corner red, 0 on the
       pulsing bar and 0x80 otherwise, set and read inside each arm. Set in
       four arms, it is not a loop.c movable, so the else-arm 0x80 is not
       matched with its twin and hoisted out of the row loop; the target
       loads `li $v0,0x80` in each else arm. Literal and chained forms
       measured worse: memory/grind/func_8006C21C/evidence.md s2, s5, s8. */
    s32 col;
    /* FAKE: always-zero narrow locals (named-local-fake-exception, owner
       ruling 2026-09-28 Q27 (A)) -- SetDrawMode's dither and texture-window
       arguments and the descriptor position, read up to the phase-4 head.
       combine folds the sign extension of each known-zero read after a label
       and distribute_notes leaves a `(use)` of the dead extension temp; the
       four never-allocated temps take the target's four untouched frame
       slots (0x60-0x78, frame 0xC0). A read from phase 5 on keeps the local
       live across the phase-4 calls (callee-saved reg, +insns), so later
       sites write a literal 0. Receipts: memory/grind/func_8006C21C/
       evidence.md s5, s6, s8, s9 and probes/s9. */
    s16 dtd;
    s16 xpos;
    s16 ypos;
    s16 tw;
    s32 pl;
    s32 x;
    s32 row;
    s32 pulse;
    /* the sprite sheet's 8-byte cell array, which starts just past the sheet's
       12-byte header (SprtHdrA / SprtEntA, read by func_8007352C). Ruling 9:
       one meaning, header + 0xC at every write; memory/grind/func_8006C21C/
       admission.md "cells". */
    u8 *cells;

    mode = 0;
    dtd = 0;
    xpos = 0;
    ypos = 0;
    tw = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.y = ypos;
    s.x = xpos;
    s.header = (u8 *)table[0];
    s.semi = 0;
    cells = s.header + 0xC;
    s.table = cells;
    s.out = arg0[5];
    arg0[5] = func_8007352C((s32)&s);
    SetDrawMode(arg0[7], 1, dtd, func_8006E480((s32)s.header, mode), tw);
    AddPrim(g_gpu_ot_ptr + 0x28, arg0[7]);
    arg0[7] += 0xC;

    s.ot_idx = 9;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.y = ypos;
    for (pl = 0; pl < 2; pl++) {
        s.x = pl * 280;
        if (*(s16 *)(D_800A34FC + pl * 2 + 0x28) < 3) {
            s32 bit;
            for (bit = 0; bit < 4; bit++) {
                if (*(u8 *)(D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17) &
                    ((1 << bit) << (pl * 4))) {
                    s.header = (u8 *)table[bit + 13];
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
    SetDrawMode(arg0[7], 1, dtd, func_8006E480((s32)s.header, mode), tw);
    AddPrim(g_gpu_ot_ptr + 0x24, arg0[7]);
    arg0[7] += 0xC;

    s.x = xpos;
    s.y = ypos;
    s.ot_idx = 10;
    s.has_color = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.header = (u8 *)table[1];
    s.semi = 0;
    cells = s.header + 0xC;
    s.table = cells;
    s.out = arg0[5];
    arg0[5] = func_8007352C((s32)&s);
    for (sprite = 0; sprite < 6; sprite++) {
        s.header = (u8 *)table[sprite + 2];
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
    for (trow = 0; trow < 11; trow++) {
        tile_rec = &recs[trow];
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
        s32 level = *(s16 *)(D_800A34FC + j * 2 + 0x28);
        rec = &recs[level + 1];
        for (row = 0; row < 2; row++) {
            SetPolyG4(poly);
            if (rec == &recs[6]) {
                SetSemiTrans(poly, 1);
                col = 0;
                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r1 = pulse;
                poly->g1 = pulse;
                poly->b1 = pulse;
                poly->r2 = col;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = col;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w, -4 * row + 2);
            } else {
                SetSemiTrans(poly, 0);
                col = 0x80;
                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = pulse;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r2 = col;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = col;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w, -2 * row + 1);
            }
            AddPrim(g_gpu_ot_ptr + 0x20, (s32)poly);
            poly++;
            SetPolyG4(poly);
            if (rec == &recs[6]) {
                SetSemiTrans(poly, 1);
                col = 0;
                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r2 = pulse;
                poly->g2 = pulse;
                poly->b2 = pulse;
                poly->r1 = col;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = col;
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
                col = 0x80;
                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = pulse;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r1 = col;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = col;
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
