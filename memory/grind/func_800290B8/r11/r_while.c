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
    s32 i;
    s32 mark;

    *(LeafPos *)(scr + 0x84) = quads[side * 4];
    *(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);
    for (i = 1; i < 4; i++) {
        s32 row = i / 2;
        s32 col = i & 1;
        s32 vtx = side * 4 + row * 2 + col;

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
    mark = 0;
    while (e->type != 0) {
        s32 tri;

        if (e->done != 0) goto next;
        if (e->y > *(s32 *)(scr + 0x88)) goto next;
        if (e->x < *(s32 *)(scr + 0x78)) goto next;
        if (*(s32 *)(scr + 0x84) < e->x) goto next;
        if (e->z < *(s32 *)(scr + 0x80)) goto next;
        if (*(s32 *)(scr + 0x8C) < e->z) goto next;
        *(s32 *)(scr + 0x100) = e->x;
        *(s32 *)(scr + 0x108) = e->z;
        for (tri = 0; tri < 2; tri++) {
            if (tri == 0) {
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
                    tri = -1;
                    continue;
                }
                func_80044B30(mark, func_8002FC80((VECTOR *)a, (VECTOR *)b, (VECTOR *)c));
                func_80033550(a);
                e->done = 1;
                return 0;
            }
        }
        if (swap == 0 || side == 0) goto next;
    found:
        *(s32 *)(scr + 0x104) = (*(s32 *)(scr + 0x7C) + *(s32 *)(scr + 0x88)) / 2;
        return 1;
    next:
        mark++;
        e++;
    }
    return 0;
}
