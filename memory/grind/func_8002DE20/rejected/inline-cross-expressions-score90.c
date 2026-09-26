extern s32 D_800A314C;
typedef struct {
    u8 unk0[0x60];
    s32 *unk60;
    u8 unk64[0xA8 - 0x64];
    VECTOR unkA8;
    VECTOR unkB8;
    u8 unkC8[0xF8 - 0xC8];
    SVECTOR unkF8;
    u8 unk100[0x118 - 0x100];
    Vec3i unk118[3];
} KiWareObj;

s32 func_8002DE20(KiWareObj *obj, s32 *p0, s32 *p1, s32 *p2)
{
    s32 *vin;
    s32 i;
    s32 max_i;
    s32 min_i;
    s32 mid_i;
    s32 min_z;
    s32 max_z;
    s32 z;
    s32 dz;
    s32 x1;
    s32 y1;
    s32 x2;
    s32 y2;
    s32 cx;
    s32 cy;

    obj->unkF8.vx = p0[0] - obj->unk60[0];
    obj->unkF8.vy = p0[1] - obj->unk60[1];
    obj->unkF8.vz = p0[2] - obj->unk60[2];
    vin = (s32 *)&obj->unkF8;
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

    dz = obj->unk118[max_i].z - obj->unk118[min_i].z;
    if (dz == 0) {
        D_800A314C++;
        dz = 1;
    }
    x1 = obj->unk118[min_i].x + (-obj->unk118[min_i].z * (obj->unk118[max_i].x - obj->unk118[min_i].x)) / dz;
    y1 = obj->unk118[min_i].y + (-obj->unk118[min_i].z * (obj->unk118[max_i].y - obj->unk118[min_i].y)) / dz;

    mid_i = 3 - min_i - max_i;
    if (obj->unk118[mid_i].z >= 0) {
        dz = obj->unk118[mid_i].z - obj->unk118[min_i].z;
        if (dz == 0) {
            D_800A314C++;
            dz = 1;
        }
        x2 = obj->unk118[min_i].x + (-obj->unk118[min_i].z * (obj->unk118[mid_i].x - obj->unk118[min_i].x)) / dz;
        y2 = obj->unk118[min_i].y + (-obj->unk118[min_i].z * (obj->unk118[mid_i].y - obj->unk118[min_i].y)) / dz;
    } else {
        dz = obj->unk118[max_i].z - obj->unk118[mid_i].z;
        if (dz == 0) {
            D_800A314C++;
            dz = 1;
        }
        x2 = obj->unk118[mid_i].x + (-obj->unk118[mid_i].z * (obj->unk118[max_i].x - obj->unk118[mid_i].x)) / dz;
        y2 = obj->unk118[mid_i].y + (-obj->unk118[mid_i].z * (obj->unk118[max_i].y - obj->unk118[mid_i].y)) / dz;
    }

    cx = (obj->unkA8.vx + obj->unkB8.vx) / 3;
    cy = (obj->unkA8.vy + obj->unkB8.vy) / 3;

    if (((obj->unkA8.vy * cx - obj->unkA8.vx * cy) ^ (obj->unkA8.vy * x1 - obj->unkA8.vx * y1)) >= 0) {
        if (((obj->unkB8.vy * cx - obj->unkB8.vx * cy) ^ (obj->unkB8.vy * x1 - obj->unkB8.vx * y1)) >= 0) {
            if ((((obj->unkB8.vy - obj->unkA8.vy) * (cx - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (cy - obj->unkA8.vy)) ^ ((obj->unkB8.vy - obj->unkA8.vy) * (x1 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y1 - obj->unkA8.vy))) >= 0) {
                return 1;
            }
        }
    }
    if (((obj->unkA8.vy * cx - obj->unkA8.vx * cy) ^ (obj->unkA8.vy * x2 - obj->unkA8.vx * y2)) >= 0) {
        if (((obj->unkB8.vy * cx - obj->unkB8.vx * cy) ^ (obj->unkB8.vy * x2 - obj->unkB8.vx * y2)) >= 0) {
            if ((((obj->unkB8.vy - obj->unkA8.vy) * (cx - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (cy - obj->unkA8.vy)) ^ ((obj->unkB8.vy - obj->unkA8.vy) * (x2 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y2 - obj->unkA8.vy))) >= 0) {
                return 1;
            }
        }
    }
    if (((obj->unkA8.vy * x1 - obj->unkA8.vx * y1) ^ (obj->unkA8.vy * x2 - obj->unkA8.vx * y2)) >= 0) {
        if ((((y2 - y1) * (obj->unkA8.vx - x1) - (x2 - x1) * (obj->unkA8.vy - y1)) ^ ((y2 - y1) * -x1 - (x2 - x1) * -y1)) >= 0) {
            return 1;
        }
    }
    if (((obj->unkB8.vy * x1 - obj->unkB8.vx * y1) ^ (obj->unkB8.vy * x2 - obj->unkB8.vx * y2)) >= 0) {
        if ((((y2 - y1) * (obj->unkB8.vx - x1) - (x2 - x1) * (obj->unkB8.vy - y1)) ^ ((y2 - y1) * -x1 - (x2 - x1) * -y1)) >= 0) {
            return 1;
        }
    }
    if ((((obj->unkB8.vy - obj->unkA8.vy) * (x1 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y1 - obj->unkA8.vy)) ^ ((obj->unkB8.vy - obj->unkA8.vy) * (x2 - obj->unkA8.vx) - (obj->unkB8.vx - obj->unkA8.vx) * (y2 - obj->unkA8.vy))) >= 0) {
        if ((((y2 - y1) * (obj->unkA8.vx - x1) - (x2 - x1) * (obj->unkA8.vy - y1)) ^ ((y2 - y1) * (obj->unkB8.vx - x1) - (x2 - x1) * (obj->unkB8.vy - y1))) >= 0) {
            return 1;
        }
    }
    return 0;
}
