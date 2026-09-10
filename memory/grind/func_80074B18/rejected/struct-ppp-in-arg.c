/* REJECTED (s1): struct pointers, p++ inside the AddPrim arg, t++ after the call.
 * Sandbox 42. The plain 'AddPrim(...); p += 0x10; t += 0xC;' order (v2/v6) is what matches:
 * sched2 hoists the four biv increments above the jal on its own. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} Tile16;
typedef struct {
    u16 x, y, w, h;
    u8 r, g, b, pad;
} TileDef12;

void func_80074B18(s32 *arg0, s32 arg1, s32 arg2) {
    Tile16 *p;
    TileDef12 *t;
    s16 i;
    s16 j;
    s32 n;
    s32 ot;

    n = 5;
    if (arg2 != 0) {
        n = 8;
    }
    p = (Tile16 *)arg0[5];
    for (i = 0; i < *(u8 *)(D_800A36A0 + 0x65) + 3; i++) {
        t = *(TileDef12 **)(*(u8 **)(D_800A36A0 + 4) + 0x3C);
        for (j = 0; j < n; j++) {
            SetTile((GameObj *)p);
            p->r0 = t->r;
            p->g0 = t->g;
            p->b0 = t->b;
            p->w = t->w;
            p->h = t->h;
            SetSemiTrans((GameObj *)p, 0);
            if (arg2 != 0) {
                p->x0 = t->x + arg1 * 240;
                p->y0 = t->y + i * 34 + 0x2B;
            } else {
                p->x0 = t->x + arg1 * 240;
                p->y0 = t->y + i * 17 + 0x7C;
            }
            ot = 0xB;
            if (arg1 != 0) {
                ot = 0x15;
            }
            AddPrim(D_800A374C + ot * 4, (GameObj *)p++);
            t++;
        }
    }
    arg0[5] = (s32)p;
}
