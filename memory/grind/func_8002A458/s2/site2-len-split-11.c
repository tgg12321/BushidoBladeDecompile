struct vec3w { s32 x, y, z; };
extern char D_80010478[];
extern void printf();
extern u8 D_8008D118;
extern u8 D_800F5F68[];
extern s16 D_800A37EA;
extern s16 D_800A37EC;
extern void func_8002E838(u8 *obj);
extern s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq);
extern s32 func_80053614(s32 *, s32 *, s32 *, s32 *, s32 *);
extern s32 func_80054434(void);
extern void func_80032854(s32, s32, u8 *, s16 *);
void func_8002A458(u8 *obj, s32 *hit, s32 *deep, s32 quiet) {
    u8 *scr = (u8 *)0x1F8002B8;
    s32 id = *(s16 *)(obj + 4);
    u8 *partner = *(u8 **)obj;
    s32 work[64];
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 dz;
    s32 dy;
    s32 dx;
    s32 diff;
    s32 h_sq;
    s32 lzc_in;
    s32 len_sq;
    s32 hit_sq;
    s32 len;
    s32 temp;
    s32 qx;
    s32 qy;
    s32 qz;
    s32 *p;
    u8 *rec;
    s32 i;

    *(u8 **)(scr + 0x60) = scr;
    *(u8 **)(scr + 0x64) = scr + 0xC;
    *(struct vec3w *)(scr + 0xC8) = *(struct vec3w *)(scr + 0xC);
    dx = (*(s32 **)(scr + 0x64))[0] - (*(s32 **)(scr + 0x60))[0];
    dy = (*(s32 **)(scr + 0x64))[1] - (*(s32 **)(scr + 0x60))[1];
    dz = (*(s32 **)(scr + 0x64))[2] - (*(s32 **)(scr + 0x60))[2];
    diff = (*(s16 *)(partner + 0x1D8) - ratan2(dx, dz)) & 0xFFF;
    if (diff >= 0x800) {
        diff = 0x1000 - diff;
    }
    temp = dx * dx + dz * dz - dy * dy;
    if (temp < 0) {
        printf(D_80010478, temp);
        return;
    }
    lzc_in = temp;
    if ((u32)temp < 0x400) {
        temp = (u32)*(((u8 *)&D_8008D118) + temp) >> 3;
    } else {
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            "nop\n"
            "nop\n"
            :: "r"(lzc_in) : "$12");
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            :: "r"(&sp_tmp) : "$12", "memory");
        {
            s32 lz = ~1;
            s32 shift;
            lz &= sp_tmp;
            shift = 0x16 - lz;
            lzc_in = *(((u8 *)&D_8008D118) + ((u32)temp >> shift));
            temp = (u32)(lzc_in << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    *(s16 *)(scr + 0xF8) = -ratan2(dy, temp);
    *(s16 *)(scr + 0xFA) = ratan2(dx, dz);
    *(s16 *)(scr + 0xFC) = 0;
    if (quiet == 0) {
        func_80032854(id == 0, 0xB, scr + 0xC8, (s16 *)(scr + 0xF8));
    }
    len_sq = dx * dx + dy * dy + dz * dz;
    if ((u32)len_sq < 0x400) {
        len = (u32)*(((u8 *)&D_8008D118) + len_sq) >> 3;
    } else {
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            "nop\n"
            "nop\n"
            :: "r"(len_sq) : "$12");
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            :: "r"(&sp_tmp2) : "$12", "memory");
        {
            s32 lz = ~1;
            s32 shift;
            s32 tbl;
            lz &= sp_tmp2;
            shift = 0x16 - lz;
            tbl = *(((u8 *)&D_8008D118) + ((u32)len_sq >> shift));
            len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    qx = (dx << 12) / len;
    qy = (dy << 12) / len;
    qz = (dz << 12) / len;
    **(struct vec3w **)(scr + 0x60) = **(struct vec3w **)(scr + 0x64);
    (*(s32 **)(scr + 0x64))[0] += qx * 4;
    (*(s32 **)(scr + 0x64))[1] += qy * 4;
    (*(s32 **)(scr + 0x64))[2] += qz * 4;
    if (diff < 0x400) {
        func_8002E838(scr);
        do {
            rec = &D_800F5F68[id * 0x1B8];
        } while (0);
        for (i = 0; i < 22; i++, rec += 0x14) {
            s32 *pos;
            if (*(s16 *)(obj + 0x26C) == 0 && i >= 6 && i <= 9) {
                continue;
            }
            pos = (s32 *)((u8 *)0x1F8000A8 + id * 0x108 + i * 0xC);
            if (func_8002EA24(scr, pos, *(u16 *)(rec + 0xC), *(u16 *)(rec + 0xE)) != 0) {
                s32 bit = 1 << i;
                *hit |= bit;
                if (*(s16 *)rec != 0
                    && func_8002EA24(scr, pos, *(u16 *)(rec + 0x10), *(u16 *)(rec + 0x12)) != 0) {
                    *deep |= bit;
                }
            }
        }
    }
    *(s32 *)(scr + 0xA8) = (*(s32 **)(scr + 0x60))[0] - qx / 4;
    *(s32 *)(scr + 0xAC) = (*(s32 **)(scr + 0x60))[1] - qy / 4;
    *(s32 *)(scr + 0xB0) = (*(s32 **)(scr + 0x60))[2] - qz / 4;
    if (func_80053614((s32 *)(scr + 0xA8), *(s32 **)(scr + 0x64), (s32 *)(scr + 0x100),
                      (s32 *)(scr + 0xF8), work) != 0
        && func_80054434() != 7) {
        if (*hit != 0) {
            p = *(s32 **)(scr + 0x60);
            dx = *(s32 *)(scr + 0x100) - p[0];
            dy = *(s32 *)(scr + 0x104) - p[1];
            dz = *(s32 *)(scr + 0x108) - p[2];
            hit_sq = dx * dx + dy * dy + dz * dz;
            dx = *(s32 *)(obj + 0xF4) - p[0];
            dy = *(s32 *)(obj + 0xF8) - p[1];
            dz = *(s32 *)(obj + 0xFC) - p[2];
            if (hit_sq >= dx * dx + dy * dy + dz * dz) {
                goto done;
            }
            *deep = 0;
            *hit = 0;
        }
        if (quiet == 0) {
            func_80032854(id == 0, 0xA, scr + 0x100, 0);
        }
    }
done:
    D_800A37E8 = -qx;
    D_800A37EA = -qy;
    D_800A37EC = -qz;
}
