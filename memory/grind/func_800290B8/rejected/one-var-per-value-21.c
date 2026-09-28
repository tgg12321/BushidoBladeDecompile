/* One 16-byte entry of the list func_8004678C returns (terminated by a zero
 * `type`); func_8002906C clears every entry's `used`. */
typedef struct {
    s16 type;
    s16 used;
    s32 x;
    s32 y;
    s32 z;
} PosRec;

s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3);
s32 func_8002FC80(VECTOR *a0, VECTOR *a1, VECTOR *a2);
void func_80033550(LeafPos *arg0);
extern void func_80044B30(s32 a0, s32 a1);

/* `tbl` holds two 2x2 grids of points (entry idx * 4 + row * 2 + col).
 * Builds the x/z bounds and the top y of grid `idx` in the scratchpad record
 * at 0x1F8002B8 (min at +0x78, max at +0x84), then walks the PosRec list for
 * an unused entry inside those bounds and tests it (at +0x100) against the
 * grid's two triangles (points 0,1,2 and 1,2,3) with func_8002E6B0.
 * A hit on a type-2 entry: when `flag` is set and idx is 0, idx becomes 1
 * and the triangle tests restart on grid 1 (the bounds are not rebuilt);
 * otherwise it calls func_80044B30 with the entry's list index and
 * func_8002FC80's result for that triangle, calls func_80033550, marks the
 * entry used and returns 0. A hit on any other type, or both triangles
 * missing while `flag` is set and idx is not 0, stores at +0x104 the average
 * of +0x7C (the y of point idx * 4, which the scan never updates) and the top
 * y, and returns 1. Returns 0 when the list runs out.
 * func_8002FC80 reads only vx/vy/vz, so the 12-byte points pass as VECTOR. */
s32 func_800290B8(s32 idx, s32 flag, LeafPos *tbl) {
    u8 *scr = (u8 *)0x1F8002B8;
    PosRec *rec;
    LeafPos *a;
    LeafPos *b;
    LeafPos *c;
    s32 i;
    s32 n;
    s32 temp;

    *(LeafPos *)(scr + 0x84) = tbl[idx * 4];
    *(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);

    for (i = 1; i < 4; i++) {
        s32 row;
        s32 col;

        row = i / 2;
        col = i & 1;
        n = idx * 4 + row * 2 + col;
        if (tbl[n].x < *(s32 *)(scr + 0x78)) {
            *(s32 *)(scr + 0x78) = tbl[n].x;
        } else if (*(s32 *)(scr + 0x84) < tbl[n].x) {
            *(s32 *)(scr + 0x84) = tbl[n].x;
        }
        if (tbl[n].z < *(s32 *)(scr + 0x80)) {
            *(s32 *)(scr + 0x80) = tbl[n].z;
        } else if (*(s32 *)(scr + 0x8C) < tbl[n].z) {
            *(s32 *)(scr + 0x8C) = tbl[n].z;
        }
        if (tbl[n].y > *(s32 *)(scr + 0x88)) {
            *(s32 *)(scr + 0x88) = tbl[n].y;
        }
    }

    rec = (PosRec *)func_8004678C();
    for (temp = 0; rec->type != 0; temp++, rec++) {
        s32 temp2;

        if (rec->used != 0) continue;
        if (rec->y > *(s32 *)(scr + 0x88)) continue;
        if (rec->x < *(s32 *)(scr + 0x78)) continue;
        if (*(s32 *)(scr + 0x84) < rec->x) continue;
        if (rec->z < *(s32 *)(scr + 0x80)) continue;
        if (*(s32 *)(scr + 0x8C) < rec->z) continue;

        *(s32 *)(scr + 0x100) = rec->x;
        *(s32 *)(scr + 0x108) = rec->z;
        for (temp2 = 0; temp2 < 2; temp2++) {
            if (temp2 == 0) {
                a = &tbl[idx * 4];
                b = &tbl[idx * 4 + 1];
                c = &tbl[idx * 4 + 2];
            } else {
                a = &tbl[idx * 4 + 1];
                b = &tbl[idx * 4 + 2];
                c = &tbl[idx * 4 + 3];
            }
            if (func_8002E6B0((s32 *)a, (s32 *)b, (s32 *)c, (s32 *)(scr + 0x100)) != 0) {
                if (rec->type != 2) goto hit;
                if (flag != 0 && idx == 0) {
                    idx = 1;
                    temp2 = -1;
                    continue;
                }
                func_80044B30(temp, func_8002FC80((VECTOR *)a, (VECTOR *)b, (VECTOR *)c));
                func_80033550(a);
                rec->used = 1;
                return 0;
            }
        }
        if (flag == 0 || idx == 0) continue;
    hit:
        *(s32 *)(scr + 0x104) = (*(s32 *)(scr + 0x7C) + *(s32 *)(scr + 0x88)) / 2;
        return 1;
    }
    return 0;
}
