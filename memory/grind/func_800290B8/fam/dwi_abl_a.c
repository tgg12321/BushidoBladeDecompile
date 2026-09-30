/* Marker list returned by func_8004678C (16-byte records, type 0 ends the
 * list; func_8002906C above clears every `done` field). */
typedef struct {
    s16 type;
    s16 done;
    s32 x, y, z;
} Marker_290B8;

extern s32 func_8002E6B0(s32 *, s32 *, s32 *, s32 *);
extern s32 func_8002FC80(VECTOR *, VECTOR *, VECTOR *);
extern void func_80033550(LeafPos *);
extern void func_80044B30(s32, s32);

s32 func_800290B8(s32 side, s32 swap, LeafPos *quads) {
    u8 *scr = (u8 *)0x1F8002B8;
    Marker_290B8 *e;
    LeafPos *a, *b, *c;
    s32 i, vtx, row;
    /* tmp_a holds two values: the corner row (i / 2) in the bounding-box loop,
     * then the marker index in the marker loop. tmp_b holds two values: the
     * corner column (i & 1), then the triangle index (0/1) of the quad being
     * tested. Shared per ordinary-c-judge-decidable Ruling 11; allocator
     * proof in memory/grind/func_800290B8/ruling11.md. */
    s32 tmp_a;
    s32 tmp_b;

    *(LeafPos *)(scr + 0x84) = quads[side * 4];
    *(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);
    for (i = 1; i < 4; i++) {
        row = i / 2;
        tmp_b = i & 1;
        vtx = side * 4 + row * 2 + tmp_b;
        if (quads[vtx].x < *(s32 *)(scr + 0x78)) {
            *(s32 *)(scr + 0x78) = quads[vtx].x;
        } else if (*(s32 *)(scr + 0x84) < quads[vtx].x) {
            *(s32 *)(scr + 0x84) = quads[vtx].x;
        }
        if (quads[vtx].z < *(s32 *)(scr + 0x80)) {
            *(s32 *)(scr + 0x80) = quads[vtx].z;
        } else if (*(s32 *)(scr + 0x8C) < quads[vtx].z) {
            *(s32 *)(scr + 0x8C) = quads[vtx].z;
        }
        if (quads[vtx].y > *(s32 *)(scr + 0x88)) {
            *(s32 *)(scr + 0x88) = quads[vtx].y;
        }
    }

    e = (Marker_290B8 *)func_8004678C();
    do { tmp_a = 0; } while (0); /* FAKE */
    for (; e->type != 0; tmp_a++, e++) {
        if (e->done != 0) continue;
        if (e->y > *(s32 *)(scr + 0x88)) continue;
        if (e->x < *(s32 *)(scr + 0x78)) continue;
        if (*(s32 *)(scr + 0x84) < e->x) continue;
        if (e->z < *(s32 *)(scr + 0x80)) continue;
        if (*(s32 *)(scr + 0x8C) < e->z) continue;
        *(s32 *)(scr + 0x100) = e->x;
        *(s32 *)(scr + 0x108) = e->z;
        for (tmp_b = 0; tmp_b < 2; tmp_b++) {
            if (tmp_b == 0) {
                a = &quads[side * 4];
                b = &quads[side * 4 + 1];
                c = &quads[side * 4 + 2];
            } else {
                a = &quads[side * 4 + 1];
                b = &quads[side * 4 + 2];
                c = &quads[side * 4 + 3];
            }
            if (func_8002E6B0(&a->x, &b->x, &c->x, (s32 *)(scr + 0x100))) {
                if (e->type != 2) goto found;
                if (swap != 0 && side == 0) {
                    side = 1;
                    tmp_b = -1;
                    continue;
                }
                func_80044B30(tmp_a, func_8002FC80((VECTOR *)a, (VECTOR *)b, (VECTOR *)c));
                func_80033550(a);
                e->done = 1;
                return 0;
            }
        }
        if (swap == 0 || side == 0) continue;
    found:
        *(s32 *)(scr + 0x104) = (*(s32 *)(scr + 0x7C) + *(s32 *)(scr + 0x88)) / 2;
        return 1;
    }
    return 0;
}
