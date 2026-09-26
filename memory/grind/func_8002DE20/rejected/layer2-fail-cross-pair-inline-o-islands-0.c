/* func_8002DE20 - manual lane (slotA2), 2026-09-26. `sandbox --disable all` == 0
 * (506/506). Construct under a pending owner question: the function-scope
 * cross_a/cross_b pair written once per same-side test (12 writes each) - see
 * memory/grind/func_8002DE20/evidence.md and docs/grind/borderline.md. */
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
    s32 *vin;
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
    s32 cross_a;  /* edge x (point a - edge start): the side point a lies on */
    s32 cross_b;  /* edge x (point b - edge start): the side point b lies on */

    obj->unkF8.vx = p0[0] - obj->unk60[0];
    obj->unkF8.vy = p0[1] - obj->unk60[1];
    obj->unkF8.vz = p0[2] - obj->unk60[2];
    vin = (s32 *)&obj->unkF8;
    /* PsyQ DMPSX inline_o.h gte_ldv0(vin), gte_rtv0() (the header's placeholder
     * .word 0x0000013f as the post-DMPSX MVMVA 0x4A486012), gte_stlvnl(out). */
    __asm__ volatile(
        "move $12, %0\n"
        "lwc2 $0, 0($12)\n"
        "lwc2 $1, 4($12)"
        : : "r"(vin) : "$12", "$13", "$14", "$15", "memory");
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : : "$12", "$13", "$14", "$15", "memory");
    obj->unkF8.vx = p1[0] - obj->unk60[0];
    obj->unkF8.vy = p1[1] - obj->unk60[1];
    obj->unkF8.vz = p1[2] - obj->unk60[2];
    __asm__ volatile(
        "move $12, %0\n"
        "swc2 $25, 0($12)\n"
        "swc2 $26, 4($12)\n"
        "swc2 $27, 8($12)"
        : : "r"(&obj->unk118[0]) : "$12", "$13", "$14", "$15", "memory");
    __asm__ volatile(
        "move $12, %0\n"
        "lwc2 $0, 0($12)\n"
        "lwc2 $1, 4($12)"
        : : "r"(vin) : "$12", "$13", "$14", "$15", "memory");
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : : "$12", "$13", "$14", "$15", "memory");
    obj->unkF8.vx = p2[0] - obj->unk60[0];
    obj->unkF8.vy = p2[1] - obj->unk60[1];
    obj->unkF8.vz = p2[2] - obj->unk60[2];
    __asm__ volatile(
        "move $12, %0\n"
        "swc2 $25, 0($12)\n"
        "swc2 $26, 4($12)\n"
        "swc2 $27, 8($12)"
        : : "r"(&obj->unk118[1]) : "$12", "$13", "$14", "$15", "memory");
    __asm__ volatile(
        "move $12, %0\n"
        "lwc2 $0, 0($12)\n"
        "lwc2 $1, 4($12)"
        : : "r"(vin) : "$12", "$13", "$14", "$15", "memory");
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : : "$12", "$13", "$14", "$15", "memory");
    __asm__ volatile(
        "move $12, %0\n"
        "swc2 $25, 0($12)\n"
        "swc2 $26, 4($12)\n"
        "swc2 $27, 8($12)"
        : : "r"(&obj->unk118[2]) : "$12", "$13", "$14", "$15", "memory");

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
    cross_a = obj->unkA8.vy * cx - obj->unkA8.vx * cy;
    cross_b = obj->unkA8.vy * x1 - obj->unkA8.vx * y1;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = obj->unkB8.vy * cx - obj->unkB8.vx * cy;
        cross_b = obj->unkB8.vy * x1 - obj->unkB8.vx * y1;
        if ((cross_a ^ cross_b) >= 0) {
            cross_a = (obj->unkB8.vy - obj->unkA8.vy) * (cx - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (cy - obj->unkA8.vy);
            cross_b = (obj->unkB8.vy - obj->unkA8.vy) * (x1 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y1 - obj->unkA8.vy);
            if ((cross_a ^ cross_b) >= 0) {
                return 1;
            }
        }
    }
    /* (x2,y2) inside the triangle */
    cross_a = obj->unkA8.vy * cx - obj->unkA8.vx * cy;
    cross_b = obj->unkA8.vy * x2 - obj->unkA8.vx * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = obj->unkB8.vy * cx - obj->unkB8.vx * cy;
        cross_b = obj->unkB8.vy * x2 - obj->unkB8.vx * y2;
        if ((cross_a ^ cross_b) >= 0) {
            cross_a = (obj->unkB8.vy - obj->unkA8.vy) * (cx - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (cy - obj->unkA8.vy);
            cross_b = (obj->unkB8.vy - obj->unkA8.vy) * (x2 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y2 - obj->unkA8.vy);
            if ((cross_a ^ cross_b) >= 0) {
                return 1;
            }
        }
    }
    /* the segment against edge (0,0)-A */
    cross_a = obj->unkA8.vy * x1 - obj->unkA8.vx * y1;
    cross_b = obj->unkA8.vy * x2 - obj->unkA8.vx * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = (y2 - y1) * (obj->unkA8.vx - x1) - (x2 - x1) * (obj->unkA8.vy - y1);
        cross_b = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    /* the segment against edge (0,0)-B */
    cross_a = obj->unkB8.vy * x1 - obj->unkB8.vx * y1;
    cross_b = obj->unkB8.vy * x2 - obj->unkB8.vx * y2;
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = (y2 - y1) * (obj->unkB8.vx - x1) - (x2 - x1) * (obj->unkB8.vy - y1);
        cross_b = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    /* the segment against edge A-B */
    cross_a = (obj->unkB8.vy - obj->unkA8.vy) * (x1 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y1 - obj->unkA8.vy);
    cross_b = (obj->unkB8.vy - obj->unkA8.vy) * (x2 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y2 - obj->unkA8.vy);
    if ((cross_a ^ cross_b) >= 0) {
        cross_a = (y2 - y1) * (obj->unkA8.vx - x1) - (x2 - x1) * (obj->unkA8.vy - y1);
        cross_b = (y2 - y1) * (obj->unkB8.vx - x1) - (x2 - x1) * (obj->unkB8.vy - y1);
        if ((cross_a ^ cross_b) >= 0) {
            return 1;
        }
    }
    return 0;
}
