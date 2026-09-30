/* The 0x2C-byte draw descriptor func_8007352C consumes (EnvA layout). */
typedef struct {
    Unk8009B0E0Record *header;
    Unk8009B400Record *table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 pad20, pad24;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Env5C8A8;

/* PsyQ libgpu TILE primitive. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} Tile5C8A8;

extern Unk8009B400Record D_8009B194[3];
extern Unk8009B400Record D_8009B1AC[2];
extern Unk8009B400Record D_8009B1BC[2];
extern Unk8009B400Record D_8009B20C[6];
extern Unk8009B400Record D_8009B23C[12];
extern Unk8009B400Record D_8009B29C[2];
extern Unk8009B400Record *D_8009B2AC[4];
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
s32 func_8005C8A8(s32 mode, s32 arg1, s32 arg2, s32 ot) {
    Env5C8A8 s;
    Tile5C8A8 *tile;
    s32 cur;
    s16 sel;
    u16 y_base;
    s32 mode_off;
    s32 size[1];
    s16 top;
    s16 i;
    s16 j;
    s16 x;
    s16 y;
    /* FAKE: always-zero narrow local (named-local-fake-exception, owner
       ruling 2026-09-28 Q27 (A)) -- the descriptor x position, written 0
       once and read as a plain value at case 0's first draw. The read sits
       after the case-0 label, where cse no longer knows the value; combine
       folds its sign extension (the pseudo is set once, to 0) and
       distribute_notes leaves a `(use)` of the dead extension temp at the
       label. The temp is never allocated and reload gives it the target's
       one untouched frame slot (sp+0x78, frame 0xB8, zero insns). A single
       read gives exactly that slot; reading it at every s.x write costs
       insns and slots, so the other writes stay literal 0. Receipts:
       memory/grind/func_8005C8A8/evidence.md s3 and probes/s3/. */
    s16 xpos;

    tile = (Tile5C8A8 *)arg2;
    cur = arg2 + 0xF0;
    y_base = 0;
    xpos = 0;
    sel = *(s16 *)&arg1;
    mode_off = arg2 + 0x4D8;
    size[0] = 0x4F0;
    top = (0xF0 - D_8009B2BC[mode].h) / 2;
    s.col_b = 0x40;
    s.col_g = 0x40;
    s.col_r = 0x40;
    s.has_color = 0;

    switch (mode) {
    case 2:
        s.header = &D_8009B0E0[2];
        s.table = D_8009B184;
        y_base = 0x33;
        s.y = top + 0x73;
        s.x = 0;
        s.semi = 0;
        s.out = cur;
        s.ot_idx = ot;
        D_8009B184[0].unk0 = (0x280 - D_8009B2BC[2].w) / 2;
        D_8009B184[1].unk0 = (D_8009B2BC[2].w + 0x280) / 2 - 0xC;
        cur = func_8007352C((s32)&s);
        s.header = &D_8009B0E0[8];
        s.table = D_8009B20C;
        s.x = 0;
        s.y = 0;
        s.out = cur;
        s.ot_idx = ot;
        cur = func_8007352C((s32)&s);
        for (j = 0; j < 3; j++) {
            if (sel == j + 3) {
                s.semi = 0;
            } else {
                s.semi = 1;
            }
            s.header = &D_8009B14C;
            s.table = &D_8009B23C[j * s.header->count];
            s.x = 0;
            s.y = 0;
            s.out = cur;
            s.ot_idx = ot;
            cur = func_8007352C((s32)&s);
            for (i = 0; i < 2; i++) {
                if (((arg1 >> (j + 16)) & 1) == i) {
                    s.semi = 0;
                } else {
                    s.semi = 1;
                }
                s.header = &D_8009B158;
                s.table = &D_8009B29C[i];
                s.x = 0;
                s.y = j * 15;
                s.out = cur;
                s.ot_idx = ot;
                cur = func_8007352C((s32)&s);
            }
        }
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = (0x280 - D_8009B2BC[mode].w) / 2;
            tile->y0 = top + 0x73 + i;
            tile->w = D_8009B2BC[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot * 4, (s32)tile);
            tile++;
        }
        /* fallthrough */
    case 0:
        if (sel == 0) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.table = D_8009B1AC;
        s.header = &D_8009B0E0[4];
        s.x = xpos;
        s.out = cur;
        s.y = y_base + top;
        s.ot_idx = ot;
        cur = func_8007352C((s32)&s);
        s.header = &D_8009B0E0[3];
        s.has_color = 0;
        s.table = D_8009B194;
        s.x = 0;
        s.y = top;
        s.semi = 0;
        s.out = cur;
        s.ot_idx = ot;
        cur = func_8007352C((s32)&s);
        top += 0xE;
        s.header = &D_8009B0E0[2];
        s.table = D_8009B184;
        s.x = 0;
        s.y = top;
        s.semi = 0;
        s.out = cur;
        s.ot_idx = ot;
        D_8009B184[0].unk0 = (0x280 - D_8009B2BC[mode].w) / 2;
        D_8009B184[1].unk0 = (D_8009B2BC[mode].w + 0x280) / 2 - 0xC;
        cur = func_8007352C((s32)&s);
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = (0x280 - D_8009B2BC[mode].w) / 2;
            tile->y0 = top + i;
            tile->w = D_8009B2BC[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot * 4, (s32)tile);
            tile++;
        }
        break;
    case 1:
        if (sel == 0) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.header = &D_8009B0E0[5];
        s.table = D_8009B1BC;
        s.x = 0;
        s.y = top;
        s.out = cur;
        s.ot_idx = ot;
        cur = func_8007352C((s32)&s);
        break;
    }

    s.has_color = 0;
    for (j = 2; j < 4; j++) {
        if (sel == j - 1) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.header = &D_8009B0E0[4 + j];
        s.table = D_8009B2AC[j];
        s.x = 0;
        s.y = y_base + top;
        s.out = cur;
        s.ot_idx = ot;
        cur = func_8007352C((s32)&s);
    }

    s.has_color = 0;
    s.header = &D_8009B0E0[0];
    s.table = D_8009B164[0];
    s.x = 0;
    s.y = (0xF0 - D_8009B2BC[mode].h) / 2;
    s.semi = 0;
    s.out = cur;
    s.ot_idx = ot;
    D_8009B164[0][0].unk0 = (0x280 - D_8009B2BC[mode].w) / 2;
    D_8009B164[0][1].unk0 = (D_8009B2BC[mode].w + 0x280) / 2 - 0xC;
    cur = func_8007352C((s32)&s);
    s.header = &D_8009B0E0[1];
    s.table = D_8009B164[1];
    s.y = (D_8009B2BC[mode].h + 0xF0) / 2;
    s.out = cur;
    D_8009B164[1][0].unk0 = (0x280 - D_8009B2BC[mode].w) / 2;
    D_8009B164[1][1].unk0 = (D_8009B2BC[mode].w + 0x280) / 2 - 0xC;
    func_8007352C((s32)&s);

    for (j = 0; j < 2; j++) {
        x = (0x280 - D_8009B2BC[mode].w) / 2;
        if (j != 0) {
            y = (0xF0 - D_8009B2BC[mode].h) / 2;
        } else {
            y = (D_8009B2BC[mode].h + 0xF0) / 2;
        }
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = x;
            tile->y0 = y + i;
            tile->w = D_8009B2BC[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot * 4, (s32)tile);
            tile++;
        }
        if (j != 0) {
            x = (0x280 - D_8009B2BC[mode].w) / 2;
        } else {
            x = (D_8009B2BC[mode].w + 0x280) / 2;
        }
        y = (0xF0 - D_8009B2BC[mode].h) / 2;
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (j != 0) {
                if (i != 0) {
                    tile->r0 = 0x3C;
                    tile->g0 = 0x3C;
                    tile->b0 = 0x3C;
                } else {
                    tile->r0 = 0x52;
                    tile->g0 = 0x52;
                    tile->b0 = 0x52;
                }
            } else {
                if (i != 0) {
                    tile->r0 = 0x52;
                    tile->g0 = 0x52;
                    tile->b0 = 0x52;
                } else {
                    tile->r0 = 0x3C;
                    tile->g0 = 0x3C;
                    tile->b0 = 0x3C;
                }
            }
            tile->x0 = x + i * 2;
            tile->y0 = y;
            tile->w = 2;
            tile->h = D_8009B2BC[mode].h + 2;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + ot * 4, (s32)tile);
            tile++;
        }
    }

    SetTile(tile);
    tile->r0 = 0;
    tile->g0 = 0;
    tile->b0 = 0;
    tile->x0 = (0x280 - D_8009B2BC[mode].w) / 2;
    tile->y0 = (0xF0 - D_8009B2BC[mode].h) / 2;
    tile->w = D_8009B2BC[mode].w;
    tile->h = D_8009B2BC[mode].h;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + ot * 4, (s32)tile);
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B0E0[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + ot * 4, mode_off);
    return size[0];
}
