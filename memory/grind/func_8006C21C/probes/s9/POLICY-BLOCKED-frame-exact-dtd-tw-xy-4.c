/* s9 POLICY-BLOCKED probe (2026-09-28): frame-only zero-valued s16 locals (dtd, tw, xpos, ypos) read only at
   phases 1-4; sandbox 4/622 (vars 128 = target frame; only the r3 store order remains). NOT landable. */
/* func_8006C21C checkpoint s7 (Codex, 2026-09-28): score 41 / 622 insns.
 * INCOMPLETE; not approved to land. See evidence.md s7 and review-s7.md.
 * Removed rejected s6 col carrier. Chained corner colors recover the s5 score.
 * Removed the entire next/k cluster and split the tile-record pointer, without
 * increasing the score. Header/cell addresses now use truthful byte pointers;
 * descriptor color bytes are explicit. Four frame slots and two store-order
 * hunks remain. The i reuse, cells reuse, and mode holder still need their full
 * respective admission evidence; this checkpoint does not claim approval.
 * Trailing macro remains SANDBOX ONLY: fix the TU prototype at any landing.
 * s8 (2026-09-28): bar 1's vertices use PsyQ setXYWH (byte-neutral, 41/622);
 * mode set once and used at all three calls; i renamed `work` (Ruling 11 E).
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
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
       func_8006E480's second argument at all three call sites. Set once and
       live across every call, so global.c gives the once-set constant pseudo
       callee-save $s5 (target: `addu $s5,$zero,$zero`, then `addu $a1,$s5,$zero`
       at the phase-3 and phase-5 calls; cse folds the phase-1 read to 0 inside
       the entry block). Literal 0 at every call measured worse; receipts in
       memory/grind/func_8006C21C/admission.md "mode". Same shape as the
       siblings func_800753D8 (`zero`) and func_8007636C (`mode`). */
    s32 mode;
    /* Holds several values (Ruling 11, ordinary-c-judge-decidable.md): the
       phase-2 unlock-bit index, the phase-4 sprite index, the phase-6 tile
       row, and the phase-8 gauge level read per player. One variable is what
       the target's allocation requires (global.c: the merged pseudo outranks
       `j` for $fp); proof in memory/grind/func_8006C21C/admission.md "work". */
    s32 work;
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
            for (work = 0; work < 4; work++) {
                if (*(u8 *)(D_800A3524 + *(s16 *)(D_800A34FC + pl * 2 + 0x28) + 0x17) &
                    ((1 << work) << (pl * 4))) {
                    s.header = (u8 *)table[work + 13];
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
    for (work = 0; work < 6; work++) {
        s.header = (u8 *)table[work + 2];
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
    for (work = 0; work < 11; work++) {
        tile_rec = &recs[work];
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

        work = *(s16 *)(D_800A34FC + j * 2 + 0x28);
        rec = &recs[work + 1];
        for (row = 0; row < 2; row++) {
            SetPolyG4(poly);
            if (work == 5) {
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
            if (work == 5) {
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
