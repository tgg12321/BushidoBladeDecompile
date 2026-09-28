typedef struct {
    s16 type;
    s16 used;
    s32 x;
    s32 y;
    s32 z;
} HitRec;

s32 func_8002E6B0(s32 *arg0, s32 *arg1, s32 *arg2, s32 *arg3);
s32 func_8002FC80(VECTOR *a0, VECTOR *a1, VECTOR *a2);
void func_80033550(LeafPos *arg0);
void func_80044B30(s32 a0, s32 a1);

s32 func_800290B8(s32 idx, s32 flag, LeafPos *tbl) {
    u8 *scr = (u8 *)0x1F8002B8;
    HitRec *rec;
    LeafPos *a;
    LeafPos *b;
    LeafPos *c;
    s32 i;
    s32 n;
    s32 j;
    s32 k;

    *(LeafPos *)(scr + 0x84) = tbl[idx * 4];
    *(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);

    for (i = 1; i < 4; i++) {
        j = i / 2;
        k = i & 1;
        n = idx * 4 + j * 2 + k;
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
        if (*(s32 *)(scr + 0x88) < tbl[n].y) {
            *(s32 *)(scr + 0x88) = tbl[n].y;
        }
    }

    rec = (HitRec *)func_8004678C();
    for (j = 0; rec->type != 0; j++, rec++) {
        if (rec->used != 0) continue;
        if (rec->y > *(s32 *)(scr + 0x88)) continue;
        if (rec->x < *(s32 *)(scr + 0x78)) continue;
        if (*(s32 *)(scr + 0x84) < rec->x) continue;
        if (rec->z < *(s32 *)(scr + 0x80)) continue;
        if (*(s32 *)(scr + 0x8C) < rec->z) continue;

        *(s32 *)(scr + 0x100) = rec->x;
        *(s32 *)(scr + 0x108) = rec->z;
        for (k = 0; k < 2; k++) {
            if (k == 0) {
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
                    k = -1;
                    continue;
                }
                func_80044B30(j, func_8002FC80((VECTOR *)a, (VECTOR *)b, (VECTOR *)c));
                func_80033550(a);
                rec->used = 1;
                return 0;
            }
        }
        if (flag != 0 && idx != 0) goto hit;
    }
    return 0;

hit:
    *(s32 *)(scr + 0x104) = (*(s32 *)(scr + 0x7C) + *(s32 *)(scr + 0x88)) / 2;
    return 1;
}
