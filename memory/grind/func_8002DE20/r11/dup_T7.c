/* func_8002DE20 - manual lane (slotE), 2026-09-26. Ordinary C plus GTE islands,
 * each the separate header statements of a PsyQ Run-time Library Release 4.3
 * macro, character for character (engine/gtemacro.py PINNED): inline_o.h
 * gte_ldv0 :16-20, gte_rtv0 :426-430, gte_stlvnl :904-909, and gtemac.h
 * gte_ApplyRotMatrix :354-357. One deviation: gte_rtv0's DMPSX placeholder
 * `.word 0x0000013f` is carried as the post-DMPSX word 0x4A486012 (no DMPSX pass
 * in this build; owner-granted, tools/grinder/owner_cluster_grants.txt).
 * cross_a / cross_b: Ruling 11 (see their declaration). */
extern s32 D_800A314C;

/* Layout of the object func_80029454 passes in, as far as this function uses it. */
typedef struct {
    u8 unk0[0x60];
    s32 *unk60;             /* 0x60: origin position */
    u8 unk64[0xA8 - 0x64];
    VECTOR unkA8;           /* 0xA8: triangle corner A, relative to the origin */
    VECTOR unkB8;           /* 0xB8: triangle corner B, relative to the origin */
    u8 unkC8[0xF8 - 0xC8];
    SVECTOR unkF8;          /* 0xF8: GTE input vector */
    u8 unk100[0x118 - 0x100];
    Vec3i unk118[3];        /* 0x118: the three rotated points */
} Unk8002DE20Obj;

/* Rotates the three points p0/p1/p2 (relative to the origin) by the current GTE
 * rotation matrix, cuts the rotated triangle with the plane z = 0, and returns 1
 * if the resulting segment (x1,y1)-(x2,y2) touches the triangle (0,0) / A / B in
 * the x-y plane: either endpoint inside it (same side of every edge as the
 * centroid), or the segment crossing one of its edges. */
s32 func_8002DE20(Unk8002DE20Obj *obj, s32 *p0, s32 *p1, s32 *p2)
{
    s32 i;
    s32 max_i;
    s32 min_i;
    s32 mid_i;
    s32 min_z;
    s32 max_z;
    s32 z;
    s32 dz_a;
    s32 dz_b;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 cx;
    s32 cy;
    s32 cross_a1;
    s32 cross_a4;
    s32 cross_a7;
    s32 cross_a9;
    s32 cross_a11;
    s32 cross_b1;
    s32 cross_b4;
    s32 cross_b7;
    s32 cross_b9;
    s32 cross_ab2; /* edge A-B x ((x2,y2) - A): its own value, see ruling11.md */

    obj->unkF8.vx = p0[0] - obj->unk60[0];
    obj->unkF8.vy = p0[1] - obj->unk60[1];
    obj->unkF8.vz = p0[2] - obj->unk60[2];
    /* gte_ldv0(&obj->unkF8): inline_o.h 4.3 :16-20 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unkF8):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,0($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    /* gte_rtv0(): inline_o.h 4.3 :426-430; post-DMPSX word 0x4A486012 for the
     * header's placeholder `.word 0x0000013f` (no DMPSX pass in this build) */
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    obj->unkF8.vx = p1[0] - obj->unk60[0];
    obj->unkF8.vy = p1[1] - obj->unk60[1];
    obj->unkF8.vz = p1[2] - obj->unk60[2];
    /* gte_stlvnl(&obj->unk118[0]): inline_o.h 4.3 :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unk118[0]):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,0($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
    /* gte_ldv0(&obj->unkF8): inline_o.h 4.3 :16-20 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unkF8):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,0($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    /* gte_rtv0(): inline_o.h 4.3 :426-430; post-DMPSX word 0x4A486012 for the
     * header's placeholder `.word 0x0000013f` (no DMPSX pass in this build) */
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    obj->unkF8.vx = p2[0] - obj->unk60[0];
    obj->unkF8.vy = p2[1] - obj->unk60[1];
    obj->unkF8.vz = p2[2] - obj->unk60[2];
    /* gte_stlvnl(&obj->unk118[1]): inline_o.h 4.3 :904-909 */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unk118[1]):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,0($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");
    /* gte_ApplyRotMatrix(&obj->unkF8, &obj->unk118[2]): gtemac.h 4.3 :354-357,
     * i.e. gte_ldv0 / gte_rtv0 / gte_stlvnl as above */
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unkF8):"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $0,0($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("lwc2  $1,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
    __asm__ volatile (".word 0x4A486012": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("move  $12,%0": :"r"(&obj->unk118[2]):"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $25,0($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $26,4($12)": : :"$12","$13","$14","$15","memory");
    __asm__ volatile ("swc2  $27,8($12)": : :"$12","$13","$14","$15","memory");

    max_i = 0;
    min_i = 0;
    max_z = obj->unk118[0].z;
    min_z = max_z;
    for (i = 1; i < 3; i++) {
        z = obj->unk118[i].z;
        if (z < min_z) {
            min_i = i;
            min_z = z;
        } else if (max_z < z) {
            max_i = i;
            max_z = z;
        }
    }
    if (min_z > 0 || max_z < 0) {
        return 0;
    }
    if (min_z == 0 && max_z == 0) {
        min_i = 0;
        max_i = 1;
        obj->unk118[0].z--;
        obj->unk118[1].z++;
    }

    /* where the min->max edge crosses z = 0 */
    dz_a = obj->unk118[max_i].z - obj->unk118[min_i].z;
    if (dz_a == 0) {
        D_800A314C++;
        dz_a = 1;
    }
    x1 = obj->unk118[min_i].x + (-obj->unk118[min_i].z * (obj->unk118[max_i].x - obj->unk118[min_i].x)) / dz_a;
    y1 = obj->unk118[min_i].y + (-obj->unk118[min_i].z * (obj->unk118[max_i].y - obj->unk118[min_i].y)) / dz_a;

    /* where the edge through the middle point crosses z = 0 */
    mid_i = 3 - min_i - max_i;
    if (obj->unk118[mid_i].z >= 0) {
        dz_b = obj->unk118[mid_i].z - obj->unk118[min_i].z;
        if (dz_b == 0) {
            D_800A314C++;
            dz_b = 1;
        }
        x2 = obj->unk118[min_i].x + (-obj->unk118[min_i].z * (obj->unk118[mid_i].x - obj->unk118[min_i].x)) / dz_b;
        y2 = obj->unk118[min_i].y + (-obj->unk118[min_i].z * (obj->unk118[mid_i].y - obj->unk118[min_i].y)) / dz_b;
    } else {
        dz_b = obj->unk118[max_i].z - obj->unk118[mid_i].z;
        if (dz_b == 0) {
            D_800A314C++;
            dz_b = 1;
        }
        x2 = obj->unk118[mid_i].x + (-obj->unk118[mid_i].z * (obj->unk118[max_i].x - obj->unk118[mid_i].x)) / dz_b;
        y2 = obj->unk118[mid_i].y + (-obj->unk118[mid_i].z * (obj->unk118[max_i].y - obj->unk118[mid_i].y)) / dz_b;
    }

    cx = (obj->unkA8.vx + obj->unkB8.vx) / 3;
    cy = (obj->unkA8.vy + obj->unkB8.vy) / 3;

    /* (x1,y1) inside the triangle */
    cross_a1 = obj->unkA8.vy * cx - obj->unkA8.vx * cy;
    cross_b1 = obj->unkA8.vy * x1 - obj->unkA8.vx * y1;
    if ((cross_a1 ^ cross_b1) >= 0) {
        s32 cross_a2;
        s32 cross_b2;
        cross_a2 = obj->unkB8.vy * cx - obj->unkB8.vx * cy;
        cross_b2 = obj->unkB8.vy * x1 - obj->unkB8.vx * y1;
        if ((cross_a2 ^ cross_b2) >= 0) {
            s32 cross_a3;
            s32 cross_b3;
            cross_a3 = (obj->unkB8.vy - obj->unkA8.vy) * (cx - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (cy - obj->unkA8.vy);
            cross_b3 = (obj->unkB8.vy - obj->unkA8.vy) * (x1 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y1 - obj->unkA8.vy);
            if ((cross_a3 ^ cross_b3) >= 0) {
                return 1;
            }
        }
    }
    /* (x2,y2) inside the triangle */
    cross_a4 = obj->unkA8.vy * cx - obj->unkA8.vx * cy;
    cross_b4 = obj->unkA8.vy * x2 - obj->unkA8.vx * y2;
    if ((cross_a4 ^ cross_b4) >= 0) {
        s32 cross_a5;
        s32 cross_b5;
        cross_a5 = obj->unkB8.vy * cx - obj->unkB8.vx * cy;
        cross_b5 = obj->unkB8.vy * x2 - obj->unkB8.vx * y2;
        if ((cross_a5 ^ cross_b5) >= 0) {
            s32 cross_a6;
            s32 cross_b6;
            cross_a6 = (obj->unkB8.vy - obj->unkA8.vy) * (cx - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (cy - obj->unkA8.vy);
            cross_b6 = (obj->unkB8.vy - obj->unkA8.vy) * (x2 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y2 - obj->unkA8.vy);
            if ((cross_a6 ^ cross_b6) >= 0) {
                return 1;
            }
            cross_a7 = obj->unkA8.vy * x1 - obj->unkA8.vx * y1; /* FAKE: duplicated into the arm */
            cross_b7 = obj->unkA8.vy * x2 - obj->unkA8.vx * y2; /* FAKE: duplicated into the arm */
        } else {
            cross_a7 = obj->unkA8.vy * x1 - obj->unkA8.vx * y1; /* FAKE: duplicated into the arm */
            cross_b7 = obj->unkA8.vy * x2 - obj->unkA8.vx * y2; /* FAKE: duplicated into the arm */
        }
    } else {
        cross_a7 = obj->unkA8.vy * x1 - obj->unkA8.vx * y1; /* FAKE: duplicated into the arm */
        cross_b7 = obj->unkA8.vy * x2 - obj->unkA8.vx * y2; /* FAKE: duplicated into the arm */
    }
    /* the segment against edge (0,0)-A */
    if ((cross_a7 ^ cross_b7) >= 0) {
        s32 cross_a8;
        s32 cross_b8;
        cross_a8 = (y2 - y1) * (obj->unkA8.vx - x1) - (x2 - x1) * (obj->unkA8.vy - y1);
        cross_b8 = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a8 ^ cross_b8) >= 0) {
            return 1;
        }
    }
    /* the segment against edge (0,0)-B */
    cross_a9 = obj->unkB8.vy * x1 - obj->unkB8.vx * y1;
    cross_b9 = obj->unkB8.vy * x2 - obj->unkB8.vx * y2;
    if ((cross_a9 ^ cross_b9) >= 0) {
        s32 cross_a10;
        s32 cross_b10;
        cross_a10 = (y2 - y1) * (obj->unkB8.vx - x1) - (x2 - x1) * (obj->unkB8.vy - y1);
        cross_b10 = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a10 ^ cross_b10) >= 0) {
            return 1;
        }
    }
    /* the segment against edge A-B */
    cross_a11 = (obj->unkB8.vy - obj->unkA8.vy) * (x1 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y1 - obj->unkA8.vy);
    cross_ab2 = (obj->unkB8.vy - obj->unkA8.vy) * (x2 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y2 - obj->unkA8.vy);
    if ((cross_a11 ^ cross_ab2) >= 0) {
        s32 cross_a12;
        s32 cross_b12;
        cross_a12 = (y2 - y1) * (obj->unkA8.vx - x1) - (x2 - x1) * (obj->unkA8.vy - y1);
        cross_b12 = (y2 - y1) * (obj->unkB8.vx - x1) - (x2 - x1) * (obj->unkB8.vy - y1);
        if ((cross_a12 ^ cross_b12) >= 0) {
            return 1;
        }
    }
    return 0;
}
