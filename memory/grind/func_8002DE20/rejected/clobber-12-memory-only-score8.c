extern s32 D_800A314C;
typedef struct {
    u8 unk0[0x60];
    s32 *unk60;
    u8 unk64[0xA8 - 0x64];
    s32 unkA8[4];
    s32 unkB8[4];
    u8 unkC8[0xF8 - 0xC8];
    s16 unkF8[4];
    u8 unk100[0x118 - 0x100];
    s32 unk118[3][3];
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
    s32 side_c;
    s32 side_p;

    obj->unkF8[0] = p0[0] - obj->unk60[0];
    obj->unkF8[1] = p0[1] - obj->unk60[1];
    obj->unkF8[2] = p0[2] - obj->unk60[2];
    vin = (s32 *)obj->unkF8;
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "lwc2 $0, 0($t4)\n"
        "lwc2 $1, 4($t4)\n"
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : "r"(vin) : "$12", "memory");
    obj->unkF8[0] = p1[0] - obj->unk60[0];
    obj->unkF8[1] = p1[1] - obj->unk60[1];
    obj->unkF8[2] = p1[2] - obj->unk60[2];
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "swc2 $25, 0($t4)\n"
        "swc2 $26, 4($t4)\n"
        "swc2 $27, 8($t4)"
        : : "r"(obj->unk118[0]) : "$12", "memory");
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "lwc2 $0, 0($t4)\n"
        "lwc2 $1, 4($t4)\n"
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : "r"(vin) : "$12", "memory");
    obj->unkF8[0] = p2[0] - obj->unk60[0];
    obj->unkF8[1] = p2[1] - obj->unk60[1];
    obj->unkF8[2] = p2[2] - obj->unk60[2];
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "swc2 $25, 0($t4)\n"
        "swc2 $26, 4($t4)\n"
        "swc2 $27, 8($t4)"
        : : "r"(obj->unk118[1]) : "$12", "memory");
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "lwc2 $0, 0($t4)\n"
        "lwc2 $1, 4($t4)\n"
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : "r"(vin) : "$12", "memory");
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "swc2 $25, 0($t4)\n"
        "swc2 $26, 4($t4)\n"
        "swc2 $27, 8($t4)"
        : : "r"(obj->unk118[2]) : "$12", "memory");

    max_i = 0;
    min_i = 0;
    max_z = obj->unk118[0][2];
    min_z = max_z;
    for (i = 1; i < 3; i++) {
        z = obj->unk118[i][2];
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
        obj->unk118[0][2]--;
        obj->unk118[1][2]++;
    }

    dz = obj->unk118[max_i][2] - obj->unk118[min_i][2];
    if (dz == 0) {
        D_800A314C++;
        dz = 1;
    }
    x1 = obj->unk118[min_i][0] + (-obj->unk118[min_i][2] * (obj->unk118[max_i][0] - obj->unk118[min_i][0])) / dz;
    y1 = obj->unk118[min_i][1] + (-obj->unk118[min_i][2] * (obj->unk118[max_i][1] - obj->unk118[min_i][1])) / dz;

    mid_i = 3 - min_i - max_i;
    if (obj->unk118[mid_i][2] >= 0) {
        dz = obj->unk118[mid_i][2] - obj->unk118[min_i][2];
        if (dz == 0) {
            D_800A314C++;
            dz = 1;
        }
        x2 = obj->unk118[min_i][0] + (-obj->unk118[min_i][2] * (obj->unk118[mid_i][0] - obj->unk118[min_i][0])) / dz;
        y2 = obj->unk118[min_i][1] + (-obj->unk118[min_i][2] * (obj->unk118[mid_i][1] - obj->unk118[min_i][1])) / dz;
    } else {
        dz = obj->unk118[max_i][2] - obj->unk118[mid_i][2];
        if (dz == 0) {
            D_800A314C++;
            dz = 1;
        }
        x2 = obj->unk118[mid_i][0] + (-obj->unk118[mid_i][2] * (obj->unk118[max_i][0] - obj->unk118[mid_i][0])) / dz;
        y2 = obj->unk118[mid_i][1] + (-obj->unk118[mid_i][2] * (obj->unk118[max_i][1] - obj->unk118[mid_i][1])) / dz;
    }

    cx = (obj->unkA8[0] + obj->unkB8[0]) / 3;
    cy = (obj->unkA8[1] + obj->unkB8[1]) / 3;

    side_c = obj->unkA8[1] * cx - obj->unkA8[0] * cy;
    side_p = obj->unkA8[1] * x1 - obj->unkA8[0] * y1;
    if ((side_c ^ side_p) >= 0) {
        side_c = obj->unkB8[1] * cx - obj->unkB8[0] * cy;
        side_p = obj->unkB8[1] * x1 - obj->unkB8[0] * y1;
        if ((side_c ^ side_p) >= 0) {
            side_c = (obj->unkB8[1] - obj->unkA8[1]) * (cx - obj->unkA8[0]) - (obj->unkB8[0] - obj->unkA8[0]) * (cy - obj->unkA8[1]);
            side_p = (obj->unkB8[1] - obj->unkA8[1]) * (x1 - obj->unkA8[0]) - (obj->unkB8[0] - obj->unkA8[0]) * (y1 - obj->unkA8[1]);
            if ((side_c ^ side_p) >= 0) {
                return 1;
            }
        }
    }
    side_c = obj->unkA8[1] * cx - obj->unkA8[0] * cy;
    side_p = obj->unkA8[1] * x2 - obj->unkA8[0] * y2;
    if ((side_c ^ side_p) >= 0) {
        side_c = obj->unkB8[1] * cx - obj->unkB8[0] * cy;
        side_p = obj->unkB8[1] * x2 - obj->unkB8[0] * y2;
        if ((side_c ^ side_p) >= 0) {
            side_c = (obj->unkB8[1] - obj->unkA8[1]) * (cx - obj->unkA8[0]) - (obj->unkB8[0] - obj->unkA8[0]) * (cy - obj->unkA8[1]);
            side_p = (obj->unkB8[1] - obj->unkA8[1]) * (x2 - obj->unkA8[0]) - (obj->unkB8[0] - obj->unkA8[0]) * (y2 - obj->unkA8[1]);
            if ((side_c ^ side_p) >= 0) {
                return 1;
            }
        }
    }
    side_c = obj->unkA8[1] * x1 - obj->unkA8[0] * y1;
    side_p = obj->unkA8[1] * x2 - obj->unkA8[0] * y2;
    if ((side_c ^ side_p) >= 0) {
        side_c = (y2 - y1) * (obj->unkA8[0] - x1) - (x2 - x1) * (obj->unkA8[1] - y1);
        side_p = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((side_c ^ side_p) >= 0) {
            return 1;
        }
    }
    side_c = obj->unkB8[1] * x1 - obj->unkB8[0] * y1;
    side_p = obj->unkB8[1] * x2 - obj->unkB8[0] * y2;
    if ((side_c ^ side_p) >= 0) {
        side_c = (y2 - y1) * (obj->unkB8[0] - x1) - (x2 - x1) * (obj->unkB8[1] - y1);
        side_p = (y2 - y1) * -x1 - (x2 - x1) * -y1;
        if ((side_c ^ side_p) >= 0) {
            return 1;
        }
    }
    side_c = (obj->unkB8[1] - obj->unkA8[1]) * (x1 - obj->unkA8[0]) - (obj->unkB8[0] - obj->unkA8[0]) * (y1 - obj->unkA8[1]);
    side_p = (obj->unkB8[1] - obj->unkA8[1]) * (x2 - obj->unkA8[0]) - (obj->unkB8[0] - obj->unkA8[0]) * (y2 - obj->unkA8[1]);
    if ((side_c ^ side_p) >= 0) {
        side_c = (y2 - y1) * (obj->unkA8[0] - x1) - (x2 - x1) * (obj->unkA8[1] - y1);
        side_p = (y2 - y1) * (obj->unkB8[0] - x1) - (x2 - x1) * (obj->unkB8[1] - y1);
        if ((side_c ^ side_p) >= 0) {
            return 1;
        }
    }
    return 0;
}
