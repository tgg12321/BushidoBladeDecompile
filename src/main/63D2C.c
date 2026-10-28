/* 5 game functions. .text 0x8007352C (ROM 0x63D2C). Start boundary: G8 (the end
 * of 5ED34.c's -G8 unit). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

extern void AddPrim(void *, void *);
extern const u8 D_800159A0[16];

s32 gpu_AddSpriteCells(Unk8007352CRec *draw_desc) {
    Unk8009B0E0Record *sheet_header = draw_desc->header;
    SPRT *sprt_cursor = (SPRT *)draw_desc->sprt_out;
    Unk8009B400Record *e;
    s32 clut;
    s16 i;
    s32 x0, y0, x1, y1;

    clut = GetClut(sheet_header->cx, sheet_header->cy);
    for (i = sheet_header->count - 1; i >= 0; i--) {
        e = draw_desc->table + i;
        x0 = e->x + draw_desc->x;
        y0 = e->y + draw_desc->y;
        x1 = x0 + e->w;
        y1 = y0 + e->h;
        if (x1 > 0 && x0 < 0x280 && y0 < 0xF0 && y1 > 0) {
            SetSprt(sprt_cursor);
            sprt_cursor->clut = clut;
            sprt_cursor->x0 = x0;
            sprt_cursor->y0 = y0;
            sprt_cursor->u0 = e->u + sheet_header->ubase;
            sprt_cursor->v0 = e->v + sheet_header->vbase;
            sprt_cursor->w = e->w;
            sprt_cursor->h = e->h;
            if (draw_desc->has_color) {
                SetShadeTex(sprt_cursor, 0);
                sprt_cursor->r0 = draw_desc->col_r;
                sprt_cursor->g0 = draw_desc->col_g;
                sprt_cursor->b0 = draw_desc->col_b;
            } else {
                SetShadeTex(sprt_cursor, 1);
            }
            SetSemiTrans(sprt_cursor, draw_desc->semi);
            if (draw_desc->ot_idx >= 0x1006) {
                draw_desc->ot_idx = 1;
                ((void (*)())font_Printf)(D_800159A0);
            }
            AddPrim(g_gpu_ot_ptr + draw_desc->ot_idx, sprt_cursor);
            sprt_cursor++;
        }
    }
    return (s32)sprt_cursor;
}

s32 gpu_AddScaledSpriteQuads(Unk8007352CRec *draw_desc, s32 mode) {
    Unk8009B400Record *e = draw_desc->table;
    Unk8009B0E0Record *sheet_header = draw_desc->header;
    POLY_FT4 *p;
    s16 i;
    u32 tpage;
    u32 clut;
    s16 du0, du1, dv0, dv1;
    u16 u_base, v_base;
    s16 ox = e->x, oy = e->y;
    s32 sx, sy;
    s32 u, v;

    if (mode == 4) {
        tpage = (sheet_header->tp0 & 0xFE1F) + (sheet_header->tp1 << 7) + 0x20;
    } else if (mode == 5) {
        tpage = (sheet_header->tp0 & 0xFE1F) + (sheet_header->tp1 << 7) + 0x40;
    } else if (mode == 6) {
        tpage = (sheet_header->tp0 & 0xFE1F) + (sheet_header->tp1 << 7) + 0x60;
    } else {
        tpage = (sheet_header->tp0 & 0xFE1F) + (sheet_header->tp1 << 7);
    }
    clut = GetClut(sheet_header->cx, sheet_header->cy);
    u_base = sheet_header->ubase;
    v_base = sheet_header->vbase;
    if (mode == 1) {
        du1 = 1;
        dv0 = 0;
    } else if (mode == 2) {
        du0 = 0;
        dv1 = -1;
    } else if (mode == 3) {
        du1 = 1;
        dv1 = -1;
    } else {
        du0 = 0;
        dv0 = 0;
    }
    p = (POLY_FT4 *)draw_desc->ft4_out;
    for (i = 0; i < sheet_header->count; i++) {
        if (mode == 1) {
            du0 = e->w - 1;
            dv1 = e->h;
        } else if (mode == 2) {
            du1 = e->w;
            dv0 = e->h - 1;
        } else if (mode == 3) {
            du0 = e->w - 1;
            dv0 = e->h - 1;
        } else {
            dv1 = e->h;
            du1 = e->w;
            if (dv1 == 1) {
                dv1 = 0;
            }
        }
        SetPolyFT4(p);
        p->tpage = tpage;
        p->clut = clut;
        sx = draw_desc->scale_x;
        sy = draw_desc->scale_y;
        p->x0 = ox + draw_desc->x + (((e->x - ox) * sx) >> 8);
        p->y0 = oy + draw_desc->y + (((e->y - oy) * sy) >> 8);
        p->x1 =
            ox + draw_desc->x + (((e->x - ox) * sx) >> 8) + ((e->w * sx) >> 8);
        p->y1 = oy + draw_desc->y + (((e->y - oy) * sy) >> 8);
        p->x2 = ox + draw_desc->x + (((e->x - ox) * sx) >> 8);
        p->y2 =
            oy + draw_desc->y + (((e->y - oy) * sy) >> 8) + ((e->h * sy) >> 8);
        p->x3 =
            ox + draw_desc->x + (((e->x - ox) * sx) >> 8) + ((e->w * sx) >> 8);
        p->y3 =
            oy + draw_desc->y + (((e->y - oy) * sy) >> 8) + ((e->h * sy) >> 8);
        u = u_base + e->u;
        v = v_base + e->v;
        p->u0 = u + du0;
        p->v0 = v + dv0;
        p->u1 = u + du1;
        p->v1 = v + dv0;
        p->u2 = u + du0;
        p->v2 = v + dv1;
        p->u3 = u + du1;
        p->v3 = v + dv1;
        if (draw_desc->has_color) {
            SetShadeTex(p, 0);
            p->r0 = draw_desc->col_r;
            p->g0 = draw_desc->col_g;
            p->b0 = draw_desc->col_b;
        } else {
            SetShadeTex(p, 1);
        }
        SetSemiTrans(p, draw_desc->semi);
        if (draw_desc->ot_idx >= 0x1006) {
            draw_desc->ot_idx = 1;
        }
        AddPrim(g_gpu_ot_ptr + draw_desc->ot_idx, p);
        p++;
        e++;
    }
    draw_desc->ft4_out = (s32)p;
    return (s32)p;
}

extern VECTOR D_8009BCD4;

s32 gpu_AddRotatedSpriteQuads(env, angle, mode)
Unk8007352CRec *env;
s16 angle;
s32 mode;
{
    s16 i;
    u32 tpage;
    u32 clut;
    /* FAKE: du0 / dv0 stay 0 in this walker (gpu_AddScaledSpriteQuads sets them
       per mirror mode); u / v written without them: score 33. */
    s16 du0, dv0;
    s16 du1, dv1;
    u16 ub, vb;
    Unk8009B0E0Record *hdr;
    SVECTOR vec[4];
    SVECTOR ang;
    VECTOR out[4];
    MATRIX mtx;
    POLY_FT4 *p;
    Unk8009B400Record *e;
    s16 j;
    s16 minx, miny, maxx, maxy;
    s16 cx, cy;
    s32 u, v;
    s32 x, y;

    hdr = env->header;
    e = env->table;
    tpage = (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7);
    clut = GetClut(hdr->cx, hdr->cy);
    minx = e->x;
    miny = e->y;
    maxx = minx + ((e->w * env->scale_x) >> 8);
    maxy = miny + ((e->h * env->scale_y) >> 8);
    for (i = 1; i < hdr->count; i++) {
        e++;
        if (minx > e->x) {
            minx = e->x;
        }
        if (maxx < e->x + ((e->w * env->scale_x) >> 8)) {
            maxx = e->x + ((e->w * env->scale_x) >> 8);
        }
        if (miny > e->y) {
            miny = e->y;
        }
        if (maxy < e->y + ((e->h * env->scale_y) >> 8)) {
            maxy = e->y + ((e->h * env->scale_y) >> 8);
        }
    }
    cy = (miny + maxy) / 2;
    cx = (minx + maxx) / 2;
    e = env->table;
    p = (POLY_FT4 *)env->ft4_out;
    du0 = 0;
    dv0 = 0;
    ub = hdr->ubase;
    vb = hdr->vbase;
    for (i = 0; i < hdr->count; i++) {
        du1 = e->w;
        dv1 = e->h;
        SetPolyFT4(p);
        p->tpage = tpage;
        p->clut = clut;
        ang.vy = 0;
        ang.vx = 0;
        ang.vz = angle;
        RotMatrix(&ang, &mtx);
        ScaleMatrixL(&mtx, &D_8009BCD4);
        SetRotMatrix(&mtx);
        for (j = 0; j < 4; j++) {
            vec[j].vx =
                (((e->x - cx) + e->w * (j & 1)) * env->scale_x >> 8) * 2;
            if (mode == 1 || mode == 3) {
                vec[j].vx = -vec[j].vx;
            }
            vec[j].vy =
                (((e->y - cy) + ((e->h * (j & 2)) >> 1)) * env->scale_y >> 8) *
                2;
            if (mode == 2 || mode == 3) {
                vec[j].vy = -vec[j].vy;
            }
            ApplyRotMatrix(&vec[j], &out[j]);
        }
        x = env->x;
        y = env->y;
        p->x0 = cx + out[0].vx + x;
        p->y0 = cy + out[0].vy + y;
        p->x1 = cx + out[1].vx + x;
        p->y1 = cy + out[1].vy + y;
        p->x2 = cx + out[2].vx + x;
        p->y2 = cy + out[2].vy + y;
        p->x3 = cx + out[3].vx + x;
        p->y3 = cy + out[3].vy + y;
        u = ub + e->u;
        v = vb + e->v;
        p->u0 = u + du0;
        p->v0 = v + dv0;
        p->u1 = u + du1;
        p->v1 = v + dv0;
        p->u2 = u + du0;
        p->v2 = v + dv1;
        p->u3 = u + du1;
        p->v3 = v + dv1;
        if (env->has_color) {
            SetShadeTex(p, 0);
            p->r0 = env->col_r;
            p->g0 = env->col_g;
            p->b0 = env->col_b;
        } else {
            SetShadeTex(p, 1);
        }
        SetSemiTrans(p, env->semi);
        if (env->ot_idx >= 0x1006) {
            env->ot_idx = 1;
        }
        AddPrim(g_gpu_ot_ptr + env->ot_idx, p);
        p++;
        e++;
    }
    env->ft4_out = (s32)p;
    return (s32)p;
}

void gpu_DrawThreeSheetsWithOptionalTileAndOpaqueQuads(
    Unk8006EACCRec *draw_context, s32 arg1) {
    Unk8007352CRec s;
    s32 sheet_index;
    Unk8009B0E0Record **sheet_headers;
    TILE *tile_cursor;
    POLY_F4 *flat_quad_cursor;

    if (arg1 == 0) {
        tile_cursor = draw_context->primitive_cursors.tile_prims;
        SetTile(tile_cursor);
        gpu_SetTileRgbFromBlockFlag(tile_cursor);
        tile_cursor->x0 = 0x3F;
        tile_cursor->y0 = 0x30;
        tile_cursor->w = 0x202;
        tile_cursor->h = 0xB0;
        SetSemiTrans(tile_cursor, 1);
        AddPrim(g_gpu_ot_ptr + 0x1E, tile_cursor);
        tile_cursor++;
        draw_context->primitive_cursors.tile_prims = tile_cursor;
    }
    s.ot_idx = 0x1F;
    s.has_color = 0;
    s.semi = 0;
    sheet_headers = draw_context->resource_root.v80076FF8->unk_38;
    s.x = 0;
    s.y = 0;
    for (sheet_index = 0; sheet_index < 3; sheet_index++) {
        s.header = sheet_headers[sheet_index];
        s.table = s.header->cells;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    }

    s.header = *sheet_headers;
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x1F, draw_context->primitive_cursors.unk_14);
    flat_quad_cursor = draw_context->primitive_cursors.flat_quad_prims;
    draw_context->primitive_cursors.unk_14++;
    SetPolyF4(flat_quad_cursor);
    gpu_SetPolyF4RgbFromBlockFlag(flat_quad_cursor);
    flat_quad_cursor->x0 = 0;
    flat_quad_cursor->y0 = 0xB9;
    flat_quad_cursor->x1 = 0x122;
    flat_quad_cursor->y1 = 0;
    flat_quad_cursor->x2 = 0;
    flat_quad_cursor->y2 = 0xEF;
    flat_quad_cursor->x3 = 0x122;
    flat_quad_cursor->y3 = 0xEF;
    SetSemiTrans(flat_quad_cursor, 0);
    AddPrim(g_gpu_ot_ptr + 0x20, flat_quad_cursor);
    flat_quad_cursor++;

    SetPolyF4(flat_quad_cursor);
    gpu_SetPolyF4RgbFromBlockFlag(flat_quad_cursor);
    flat_quad_cursor->x0 = 0x15E;
    flat_quad_cursor->y0 = 0;
    flat_quad_cursor->x1 = 0x27F;
    flat_quad_cursor->y1 = 0;
    flat_quad_cursor->x2 = 0x15E;
    flat_quad_cursor->y2 = 0xEF;
    flat_quad_cursor->x3 = 0x27F;
    flat_quad_cursor->y3 = 0x36;
    SetSemiTrans(flat_quad_cursor, 0);
    AddPrim(g_gpu_ot_ptr + 0x20, flat_quad_cursor);
    flat_quad_cursor++;

    SetPolyF4(flat_quad_cursor);
    gpu_SetPolyF4RgbFromBlockFlag(flat_quad_cursor);
    flat_quad_cursor->x0 = 0x122;
    flat_quad_cursor->y0 = 0;
    flat_quad_cursor->x1 = 0x15E;
    flat_quad_cursor->y1 = 0;
    flat_quad_cursor->x2 = 0x122;
    flat_quad_cursor->y2 = 0xEF;
    flat_quad_cursor->x3 = 0x15E;
    flat_quad_cursor->y3 = 0xEF;
    SetSemiTrans(flat_quad_cursor, 0);
    AddPrim(g_gpu_ot_ptr + 0x20, flat_quad_cursor);
    flat_quad_cursor++;

    draw_context->primitive_cursors.flat_quad_prims = flat_quad_cursor;
}

void menu_DrawSheetsWithSelectedPulseOffsetDimmedOthersAndGreyTiles(
    Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    s16 mask;
    s16 sheet_index;
    Unk8009B0E0Record **table;
    s16 tile_rect[4];

    sheet_index = 0;
    mask = (1 << SELWORK->f3C[0]) + (1 << (SELWORK->f65 + 5)) +
           (1 << (SELWORK->f67 + 8)) + (1 << (SELWORK->f66 + 9));
    s.ot_idx = 2;
    table = draw_context->resource_root.v80076FF8->unk_34;
    do {
        s.x = 0;
        s.y = 0;
        s.semi = 0;
        if ((mask >> sheet_index) & 1) {
            if (sheet_index < 5) {
                s.col_b =
                    ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) -
                    0x80;
                s.y = SELWORK->f40[0][1];
            } else if (sheet_index < 8) {
                if (SELWORK->f3C[0] == 0) {
                    s.col_b =
                        ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >>
                         12) -
                        0x80;
                    s.x = SELWORK->f40[0][0];
                    s.y = SELWORK->f40[0][1];
                } else {
                    s.col_b = 0x80;
                }
            } else if (sheet_index < 10) {
                if (SELWORK->f3C[0] == 1) {
                    s.col_b =
                        ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >>
                         12) -
                        0x80;
                    s.x = SELWORK->f40[0][0];
                    s.y = SELWORK->f40[0][1];
                } else {
                    s.col_b = 0x80;
                }
            } else if (sheet_index < 14) {
                if (SELWORK->f3C[0] == 2) {
                    s.col_b =
                        ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >>
                         12) -
                        0x80;
                    s.x = SELWORK->f40[0][0];
                    s.y = SELWORK->f40[0][1];
                } else {
                    s.col_b = 0x80;
                }
            }
            s.has_color = 1;
            s.col_g = s.col_b;
            s.col_r = s.col_b;
        } else {
            s.col_b = 0x40;
            s.col_g = 0x40;
            s.col_r = 0x40;
            if (sheet_index < 5) {
                s.has_color = 0;
                s.semi = 1;
            } else if (sheet_index < 14) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
        }
        if ((u16)(sheet_index - 10) >= 4 ||
            g_sel_sheet_code_pairs[SELWORK->f67][0] + 9 == sheet_index ||
            g_sel_sheet_code_pairs[SELWORK->f67][1] + 9 == sheet_index) {
            s.header = table[sheet_index];
            s.table = s.header->cells;
            s.sprt_out = draw_context->primitive_cursors.sprite_prims;
            draw_context->primitive_cursors.sprite_prims =
                gpu_AddSpriteCells(&s);
        }
        sheet_index++;
    } while (sheet_index < 15);

    s.header = table[0];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 2, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    tile_rect[2] = 0x108;
    tile_rect[0] = 0xBC;
    tile_rect[1] = 0x25;
    tile_rect[3] = 1;
    /* The original passes its own context base here, unadjusted:
     * gpu_AddThreeGreyTilesStackedUp reads +0x18 as its TILE cursor, which in
     * this context is the DR_MODE cursor. */
    gpu_AddThreeGreyTilesStackedUp((s32 *)draw_context, (u16 *)tile_rect, 2);
}

/* Tentative definitions (COMMON) of the small data this file reaches
 * gp-relative (Q65). */
u8 *g_sel_screen_work;

/* .data 0x8009BCD4..0x8009BCE4, this file's whole .data (its one reader is
 * gpu_AddRotatedSpriteQuads; d_8C41C.c's g_entry_menu_transition_offsets ends
 * at 0x8009BCD4, d_8C4E4.c's g_sel_entry_availability_and_claims follows): the
 * ScaleMatrixL scale, x and z 1.0, y 0.5 (4.12). Defined after every function,
 * so no body sees a definition where it saw an extern. */
VECTOR D_8009BCD4 = {0x1000, 0x800, 0x1000, 0};
