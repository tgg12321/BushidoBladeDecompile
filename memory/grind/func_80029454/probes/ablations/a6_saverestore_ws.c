/* Scratchpad point tables at 0x1F800000, as far as this function uses them.
 * unk00: three points per character (func_8002C61C copies [0][0..2] and
 * [1][0..2] to the two records' +0x210); unk48: two more per character
 * (copied to +0x234); unkA8: 22 points per character (func_8002A458 reads
 * 0x1F8000A8 + id * 0x108 + i * 0xC, i < 22). */
typedef struct {
    LeafPos unk00[2][3];
    LeafPos unk48[2][2];
    u8 unk78[0xA8 - 0x78];
    LeafPos unkA8[2][22];
} ScrPad;
#define SPAD ((ScrPad *)0x1F800000)

/* 1 when the box at scr+0x78 (min) / +0x84 (max) and the box at scr+0x90 (min)
 * / +0x9C (max) overlap on all three axes. */
static inline s32 box_overlap(u8 *scr) {
    return *(s32 *)(scr + 0x78) <= *(s32 *)(scr + 0x9C) && *(s32 *)(scr + 0x84) >= *(s32 *)(scr + 0x90)
        && *(s32 *)(scr + 0x7C) <= *(s32 *)(scr + 0xA0) && *(s32 *)(scr + 0x88) >= *(s32 *)(scr + 0x94)
        && *(s32 *)(scr + 0x80) <= *(s32 *)(scr + 0xA4) && *(s32 *)(scr + 0x8C) >= *(s32 *)(scr + 0x98);
}

/* Both are defined further down this file; func_8002DE20's first parameter
 * type (Unk8002DE20Obj) is declared there, hence no prototype here. */
extern s32 func_8002DAD0(u8 *obj);
extern s32 func_8002DE20();

/* Blade contact test between the two records at D_80101EC8 (stride 0x44C),
 * called by func_8002C61C (its result goes to D_800A3824). Returns -1 unless
 * both records' +0x3C are at least 4.
 *
 * The first 16 points of SPAD->unkA8 are saved and borrowed as `ws`: eight
 * points per record, two 2x2 grids (func_800290B8's `tbl` layout) built from
 * the scratchpad points unk00 / unk48 and the record's +0x210 / +0x234 points.
 *
 * First loop, per record whose +0xE is not 6 or 7 and whose +0x6A is 2, 0x1B,
 * 0x28 or 0x26: fills grid 0 (and grid 1: the +0xE 4/5 shape, or the unk48
 * points when +0x8C is set). If +0x40 is inside +0xA1..+0xA3 and func_800290B8
 * hits grid 0, or +0x8C is set, +0x40 is inside +0xA2..+0xA4 and it hits grid
 * 1, effects 1, 0x26 and 0x2D play at scr+0x100, +0x286 becomes 0x19 (+0x8C
 * set, first case) or 0xB, and +0xAD is cleared.
 *
 * Then both records' grids are rebuilt (y raised to 100000 when +0x96 is set,
 * +0x92 is 0 or +0xC is 0x1F), count[c] is the record's triangle count (2, or
 * 4 with +0x8C set), and every coordinate is halved. Triangle t of a record is
 * points 2,3,1 (t even) or 0,1,2 (t odd) of grid t >> 1, written as pointers
 * to scr+0x60.. (first triangle) or scr+0x6C.. (second).
 *
 * Each record-0 triangle that func_8002DAD0 accepts is tested against each
 * record-1 triangle (bounding boxes at scr+0x78 / scr+0x90, then
 * func_8002DE20); the rejected ones are marked in `mask` and tested again with
 * the roles swapped. A hit restores the saved points and returns
 * (record-1 grid << 1) | record-0 grid; otherwise the points are restored and
 * -1 is returned. */
s32 func_80029454(void) {
    u8 *scr = (u8 *)0x1F8002B8;
    LeafPos *ws = SPAD->unkA8[0];
    LeafPos saved[16];
    s32 count[2];
    /* Loop counters, each used by several loops: i by the two record loops and
     * the halving loop's outer loop, j by the halving loop's inner loop and both
     * passes' outer loops, n by both passes' inner loops, k by the save,
     * bounds and restore loops. */
    s32 i;
    s32 j;
    s32 n;
    u32 mask;
    s32 k;
    s32 *p;
    u8 *rec;

    if (D_80101F04 < 4) {
        return -1;
    }
    if (D_80102350 < 4) {
        return -1;
    }
    for (k = 0; k < 16; k++) {
        saved[k] = ws[k];
    }

    for (i = 0; i < 2; i++) {
        rec = (u8 *)&D_80101EC8 + i * 0x44C;
        if (*(u16 *)(rec + 0xE) == 6 || *(u16 *)(rec + 0xE) == 7) {
            continue;
        }
        if (!(*(u16 *)(rec + 0x6A) == 2 || *(u16 *)(rec + 0x6A) == 0x1B || *(u16 *)(rec + 0x6A) == 0x28
              || *(u16 *)(rec + 0x6A) == 0x26)) {
            continue;
        }
        if (*(u16 *)(rec + 0xE) == 4 || *(u16 *)(rec + 0xE) == 5) {
            ws[0] = SPAD->unk00[i][0];
            ws[1] = SPAD->unk00[i][2];
            ws[2] = *(LeafPos *)(rec + 0x210);
            ws[3] = *(LeafPos *)(rec + 0x228);
            ws[4] = SPAD->unk00[i][0];
            ws[5] = SPAD->unk00[i][1];
            ws[6] = *(LeafPos *)(rec + 0x210);
            ws[7] = *(LeafPos *)(rec + 0x21C);
        } else {
            ws[0] = SPAD->unk00[i][0];
            ws[1] = SPAD->unk00[i][1];
            ws[2] = *(LeafPos *)(rec + 0x210);
            ws[3] = *(LeafPos *)(rec + 0x21C);
            if (*(s16 *)(rec + 0x8C) != 0) {
                ws[4] = SPAD->unk48[i][0];
                ws[5] = SPAD->unk48[i][1];
                ws[6] = *(LeafPos *)(rec + 0x234);
                ws[7] = *(LeafPos *)(rec + 0x240);
            }
        }
        if (*(s16 *)(rec + 0x40) >= *(u8 *)(rec + 0xA1) && *(s16 *)(rec + 0x40) <= *(u8 *)(rec + 0xA3)
            && func_800290B8(0, *(u16 *)(rec + 0xE) == 4 || *(u16 *)(rec + 0xE) == 5, ws) != 0) {
            func_80032854(i, 1, scr + 0x100, 0);
            func_80032854(i, 0x26, scr + 0x100, 0);
            func_80032854(i, 0x2D, scr + 0x100, 0);
            *(s16 *)(rec + 0x286) = *(s16 *)(rec + 0x8C) != 0 ? 0x19 : 0xB;
            *(u8 *)(rec + 0xAD) = 0;
        } else if (*(s16 *)(rec + 0x8C) != 0 && *(s16 *)(rec + 0x40) >= *(u8 *)(rec + 0xA2)
                   && *(s16 *)(rec + 0x40) <= *(u8 *)(rec + 0xA4) && func_800290B8(1, 0, ws) != 0) {
            func_80032854(i, 1, scr + 0x100, 0);
            func_80032854(i, 0x26, scr + 0x100, 0);
            func_80032854(i, 0x2D, scr + 0x100, 0);
            *(s16 *)(rec + 0x286) = 0xB;
            *(u8 *)(rec + 0xAD) = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        LeafPos *dst = &ws[i * 8];
        rec = (u8 *)&D_80101EC8 + i * 0x44C;
        dst[0] = SPAD->unk00[i][0];
        dst[1] = SPAD->unk00[i][1];
        dst[2] = *(LeafPos *)(rec + 0x210);
        dst[3] = *(LeafPos *)(rec + 0x21C);
        count[i] = 2;
        if (*(s16 *)(rec + 0x96) != 0 || *(s16 *)(rec + 0x92) == 0 || *(s16 *)(rec + 0xC) == 0x1F) {
            dst[0].y = 100000;
            dst[1].y = 100000;
            dst[2].y = 100000;
            dst[3].y = 100000;
        }
        if (*(s16 *)(rec + 0x8C) != 0) {
            dst[4] = SPAD->unk48[i][0];
            dst[5] = SPAD->unk48[i][1];
            dst[6] = *(LeafPos *)(rec + 0x234);
            dst[7] = *(LeafPos *)(rec + 0x240);
            count[i] += 2;
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 4; j++) {
            p = (s32 *)((u8 *)ws + (i * 0x60 + j * 0x18));
            p[0] >>= 1;
            p[1] >>= 1;
            p[2] >>= 1;
            p[3] >>= 1;
            p[4] >>= 1;
            p[5] >>= 1;
        }
    }

    mask = 0;
    for (j = 0; j < count[0]; j++) {
        switch (j) {
        case 0:
            *(LeafPos **)(scr + 0x60) = &ws[2];
            *(LeafPos **)(scr + 0x64) = &ws[3];
            *(LeafPos **)(scr + 0x68) = &ws[1];
            break;
        case 1:
            *(LeafPos **)(scr + 0x60) = &ws[0];
            *(LeafPos **)(scr + 0x64) = &ws[1];
            *(LeafPos **)(scr + 0x68) = &ws[2];
            break;
        case 2:
            *(LeafPos **)(scr + 0x60) = &ws[6];
            *(LeafPos **)(scr + 0x64) = &ws[7];
            *(LeafPos **)(scr + 0x68) = &ws[5];
            break;
        case 3:
            *(LeafPos **)(scr + 0x60) = &ws[4];
            *(LeafPos **)(scr + 0x64) = &ws[5];
            *(LeafPos **)(scr + 0x68) = &ws[6];
            break;
        }
        if (func_8002DAD0(scr) == 0) {
            mask |= 1 << j;
            continue;
        }
        *(LeafPos *)(scr + 0x84) = **(LeafPos **)(scr + 0x60);
        *(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);
        for (k = 1; k < 3; k++) {
            if (((LeafPos **)(scr + 0x60))[k]->x < *(s32 *)(scr + 0x78)) {
                *(s32 *)(scr + 0x78) = ((LeafPos **)(scr + 0x60))[k]->x;
            } else if (*(s32 *)(scr + 0x84) < ((LeafPos **)(scr + 0x60))[k]->x) {
                *(s32 *)(scr + 0x84) = ((LeafPos **)(scr + 0x60))[k]->x;
            }
            if (((LeafPos **)(scr + 0x60))[k]->y < *(s32 *)(scr + 0x7C)) {
                *(s32 *)(scr + 0x7C) = ((LeafPos **)(scr + 0x60))[k]->y;
            } else if (*(s32 *)(scr + 0x88) < ((LeafPos **)(scr + 0x60))[k]->y) {
                *(s32 *)(scr + 0x88) = ((LeafPos **)(scr + 0x60))[k]->y;
            }
            if (((LeafPos **)(scr + 0x60))[k]->z < *(s32 *)(scr + 0x80)) {
                *(s32 *)(scr + 0x80) = ((LeafPos **)(scr + 0x60))[k]->z;
            } else if (*(s32 *)(scr + 0x8C) < ((LeafPos **)(scr + 0x60))[k]->z) {
                *(s32 *)(scr + 0x8C) = ((LeafPos **)(scr + 0x60))[k]->z;
            }
        }
        for (n = 0; n < count[1]; n++) {
            switch (n) {
            case 0:
                *(LeafPos **)(scr + 0x6C) = &ws[10];
                *(LeafPos **)(scr + 0x70) = &ws[11];
                *(LeafPos **)(scr + 0x74) = &ws[9];
                break;
            case 1:
                *(LeafPos **)(scr + 0x6C) = &ws[8];
                *(LeafPos **)(scr + 0x70) = &ws[9];
                *(LeafPos **)(scr + 0x74) = &ws[10];
                break;
            case 2:
                *(LeafPos **)(scr + 0x6C) = &ws[14];
                *(LeafPos **)(scr + 0x70) = &ws[15];
                *(LeafPos **)(scr + 0x74) = &ws[13];
                break;
            case 3:
                *(LeafPos **)(scr + 0x6C) = &ws[12];
                *(LeafPos **)(scr + 0x70) = &ws[13];
                *(LeafPos **)(scr + 0x74) = &ws[14];
                break;
            }
            *(LeafPos *)(scr + 0x9C) = **(LeafPos **)(scr + 0x6C);
            *(LeafPos *)(scr + 0x90) = *(LeafPos *)(scr + 0x9C);
            for (k = 1; k < 3; k++) {
                if (((LeafPos **)(scr + 0x6C))[k]->x < *(s32 *)(scr + 0x90)) {
                    *(s32 *)(scr + 0x90) = ((LeafPos **)(scr + 0x6C))[k]->x;
                } else if (*(s32 *)(scr + 0x9C) < ((LeafPos **)(scr + 0x6C))[k]->x) {
                    *(s32 *)(scr + 0x9C) = ((LeafPos **)(scr + 0x6C))[k]->x;
                }
                if (((LeafPos **)(scr + 0x6C))[k]->y < *(s32 *)(scr + 0x94)) {
                    *(s32 *)(scr + 0x94) = ((LeafPos **)(scr + 0x6C))[k]->y;
                } else if (*(s32 *)(scr + 0xA0) < ((LeafPos **)(scr + 0x6C))[k]->y) {
                    *(s32 *)(scr + 0xA0) = ((LeafPos **)(scr + 0x6C))[k]->y;
                }
                if (((LeafPos **)(scr + 0x6C))[k]->z < *(s32 *)(scr + 0x98)) {
                    *(s32 *)(scr + 0x98) = ((LeafPos **)(scr + 0x6C))[k]->z;
                } else if (*(s32 *)(scr + 0xA4) < ((LeafPos **)(scr + 0x6C))[k]->z) {
                    *(s32 *)(scr + 0xA4) = ((LeafPos **)(scr + 0x6C))[k]->z;
                }
            }
            if (box_overlap(scr)
                && func_8002DE20(scr, *(s32 **)(scr + 0x6C), *(s32 **)(scr + 0x70), *(s32 **)(scr + 0x74)) != 0) {
                for (k = 0; k < 16; k++) {
                    ws[k] = saved[k];
                }
                return ((n >> 1) * 2) | (j >> 1);
            }
        }
    }
    if (mask == 0) {
        for (k = 0; k < 16; k++) {
            ws[k] = saved[k];
        }
        return -1;
    }

    for (j = 0; j < count[1]; j++) {
        switch (j) {
        case 0:
            *(LeafPos **)(scr + 0x60) = &ws[10];
            *(LeafPos **)(scr + 0x64) = &ws[11];
            *(LeafPos **)(scr + 0x68) = &ws[9];
            break;
        case 1:
            *(LeafPos **)(scr + 0x60) = &ws[8];
            *(LeafPos **)(scr + 0x64) = &ws[9];
            *(LeafPos **)(scr + 0x68) = &ws[10];
            break;
        case 2:
            *(LeafPos **)(scr + 0x60) = &ws[14];
            *(LeafPos **)(scr + 0x64) = &ws[15];
            *(LeafPos **)(scr + 0x68) = &ws[13];
            break;
        case 3:
            *(LeafPos **)(scr + 0x60) = &ws[12];
            *(LeafPos **)(scr + 0x64) = &ws[13];
            *(LeafPos **)(scr + 0x68) = &ws[14];
            break;
        }
        if (func_8002DAD0(scr) == 0) {
            continue;
        }
        *(LeafPos *)(scr + 0x84) = **(LeafPos **)(scr + 0x60);
        *(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);
        for (k = 1; k < 3; k++) {
            if (((LeafPos **)(scr + 0x60))[k]->x < *(s32 *)(scr + 0x78)) {
                *(s32 *)(scr + 0x78) = ((LeafPos **)(scr + 0x60))[k]->x;
            } else if (*(s32 *)(scr + 0x84) < ((LeafPos **)(scr + 0x60))[k]->x) {
                *(s32 *)(scr + 0x84) = ((LeafPos **)(scr + 0x60))[k]->x;
            }
            if (((LeafPos **)(scr + 0x60))[k]->y < *(s32 *)(scr + 0x7C)) {
                *(s32 *)(scr + 0x7C) = ((LeafPos **)(scr + 0x60))[k]->y;
            } else if (*(s32 *)(scr + 0x88) < ((LeafPos **)(scr + 0x60))[k]->y) {
                *(s32 *)(scr + 0x88) = ((LeafPos **)(scr + 0x60))[k]->y;
            }
            if (((LeafPos **)(scr + 0x60))[k]->z < *(s32 *)(scr + 0x80)) {
                *(s32 *)(scr + 0x80) = ((LeafPos **)(scr + 0x60))[k]->z;
            } else if (*(s32 *)(scr + 0x8C) < ((LeafPos **)(scr + 0x60))[k]->z) {
                *(s32 *)(scr + 0x8C) = ((LeafPos **)(scr + 0x60))[k]->z;
            }
        }
        for (n = 0; n < count[0]; n++) {
            if (!(mask & (1 << n))) {
                continue;
            }
            switch (n) {
            case 0:
                *(LeafPos **)(scr + 0x6C) = &ws[2];
                *(LeafPos **)(scr + 0x70) = &ws[3];
                *(LeafPos **)(scr + 0x74) = &ws[1];
                break;
            case 1:
                *(LeafPos **)(scr + 0x6C) = &ws[0];
                *(LeafPos **)(scr + 0x70) = &ws[1];
                *(LeafPos **)(scr + 0x74) = &ws[2];
                break;
            case 2:
                *(LeafPos **)(scr + 0x6C) = &ws[6];
                *(LeafPos **)(scr + 0x70) = &ws[7];
                *(LeafPos **)(scr + 0x74) = &ws[5];
                break;
            case 3:
                *(LeafPos **)(scr + 0x6C) = &ws[4];
                *(LeafPos **)(scr + 0x70) = &ws[5];
                *(LeafPos **)(scr + 0x74) = &ws[6];
                break;
            }
            *(LeafPos *)(scr + 0x9C) = **(LeafPos **)(scr + 0x6C);
            *(LeafPos *)(scr + 0x90) = *(LeafPos *)(scr + 0x9C);
            for (k = 1; k < 3; k++) {
                if (((LeafPos **)(scr + 0x6C))[k]->x < *(s32 *)(scr + 0x90)) {
                    *(s32 *)(scr + 0x90) = ((LeafPos **)(scr + 0x6C))[k]->x;
                } else if (*(s32 *)(scr + 0x9C) < ((LeafPos **)(scr + 0x6C))[k]->x) {
                    *(s32 *)(scr + 0x9C) = ((LeafPos **)(scr + 0x6C))[k]->x;
                }
                if (((LeafPos **)(scr + 0x6C))[k]->y < *(s32 *)(scr + 0x94)) {
                    *(s32 *)(scr + 0x94) = ((LeafPos **)(scr + 0x6C))[k]->y;
                } else if (*(s32 *)(scr + 0xA0) < ((LeafPos **)(scr + 0x6C))[k]->y) {
                    *(s32 *)(scr + 0xA0) = ((LeafPos **)(scr + 0x6C))[k]->y;
                }
                if (((LeafPos **)(scr + 0x6C))[k]->z < *(s32 *)(scr + 0x98)) {
                    *(s32 *)(scr + 0x98) = ((LeafPos **)(scr + 0x6C))[k]->z;
                } else if (*(s32 *)(scr + 0xA4) < ((LeafPos **)(scr + 0x6C))[k]->z) {
                    *(s32 *)(scr + 0xA4) = ((LeafPos **)(scr + 0x6C))[k]->z;
                }
            }
            if (box_overlap(scr)
                && func_8002DE20(scr, *(s32 **)(scr + 0x6C), *(s32 **)(scr + 0x70), *(s32 **)(scr + 0x74)) != 0) {
                for (k = 0; k < 16; k++) {
                    ws[k] = saved[k];
                }
                return ((j >> 1) * 2) | (n >> 1);
            }
        }
    }
    for (k = 0; k < 16; k++) {
        ws[k] = saved[k];
    }
    return -1;
}
