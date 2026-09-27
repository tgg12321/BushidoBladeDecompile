extern u8 D_800A30F0[];
extern s32 D_800A30F4[];
typedef struct { s32 vx, vy, vz, pad; } CamVec;
typedef struct {
    u16 unk0;       /* 0x00 */
    u16 yaw;        /* 0x02 */
    u16 roll;       /* 0x04 */
    u16 unk6;       /* 0x06 */
    CamVec focus;   /* 0x08 */
    CamVec eye;     /* 0x18 */
    CamVec head;    /* 0x28 */
    s32 unk38[8];   /* 0x38 */
    u16 unk58;      /* 0x58 */
    u16 unk5A;      /* 0x5A */
    s32 unk5C;      /* 0x5C */
    s32 unk60;      /* 0x60 */
} CamScratch;
void func_8001A820(s32 arg0, GameObj *arg1, s32 arg2, s32 arg3) {
    s32 lzc_out;
    CamScratch *scr;
    Rec44 *cam;
    s32 dx, dy, dz;
    s32 x, y, z;
    s32 shift;
    u32 dist_sq;
    u32 dist;
    u32 q;
    s32 zoom;
    s32 yaw0, yaw1;
    s32 base_yaw;
    s32 p;
    s32 yaw, other;
    s32 i;
    s32 d;

    scr = (CamScratch *)0x1F800000;
    cam = &D_800F6608;
    D_800F6608.h30 = 0x64;
    D_800F6608.h32 = 0;
    D_800F6608.h34 = 0x64;
    D_800F6608.h38 = 0x64;
    D_800F6608.h3A = 0;
    D_800F6608.h3C = 0x64;
    dx = ((s32 *)arg1)[0] - ((s32 *)arg0)[0];
    dy = ((s32 *)arg1)[1] - ((s32 *)arg0)[1];
    dz = ((s32 *)arg1)[2] - ((s32 *)arg0)[2];
    if (D_800A3690 == 0) {
        scr->focus.vx = (((s32 *)arg0)[0] + ((s32 *)arg1)[0]) / 2;
        scr->focus.vy = (((s32 *)arg0)[1] + ((s32 *)arg1)[1]) / 2;
        scr->focus.vz = (((s32 *)arg0)[2] + ((s32 *)arg1)[2]) / 2;
    } else {
        scr->focus = *(CamVec *)arg0;
    }
    cam->w0 += (scr->focus.vx - cam->w0) / 4;
    cam->w4 += (scr->focus.vy - cam->w4) / 4;
    cam->w8 += (scr->focus.vz - cam->w8) / 4;

    x = dx;
    y = dy;
    z = dz;
    shift = 0;
    while ((u32)(x + 0x4000) > 0x8000U || (u32)(z + 0x4000) > 0x8000U) {
        x /= 2;
        y /= 2;
        z /= 2;
        shift++;
    }
    dist_sq = x * x + z * z + y * y;
    if (dist_sq < 0x400) {
        dist = (u32)*(&D_8008D118 + dist_sq) >> 3;
    } else {
        s32 lzcr = 0;
        if ((s32)dist_sq >= 0) {
            __asm__ volatile ("move  $12,%0": :"r"(dist_sq):"$12","$13","$14","$15","memory");
            __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
            __asm__ volatile ("move  $12,%0": :"r"(&lzc_out):"$12","$13","$14","$15","memory");
            __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
            lzcr = lzc_out;
        }
        {
            s32 sh = 0x16 - (lzcr & ~1);
            s32 tbl = *(&D_8008D118 + (dist_sq >> sh));
            dist = (u32)(tbl << 16) >> (0x13 - ((u32)sh >> 1));
        }
    }
    dist <<= shift;
    q = 0x2000000U / (dist + 0x4000);
    q += 0x400;
    if (*(u16 *)(arg2 + 0x6A) == 0x13 || *(u16 *)(arg2 + 0x6A) == 0x1B || *(u16 *)(arg2 + 0x6A) == 0x30 ||
        *(u16 *)(arg3 + 0x6A) == 0x13 || *(u16 *)(arg3 + 0x6A) == 0x1B || *(u16 *)(arg3 + 0x6A) == 0x30) {
        q += 0x1000;
    }
    zoom = ((dist + q) << 7) / 100;
    if (dy < 0) {
        dy = -dy;
    }
    zoom += dy;
    if (*(u16 *)(arg2 + 0x6A) == 0xF || *(u16 *)(arg2 + 0x6A) == 0x1C || *(u16 *)(arg2 + 0x6A) == 0x1D ||
        *(u16 *)(arg2 + 0x6A) == 0x1E || *(u16 *)(arg2 + 0x6A) == 0x1F || *(u16 *)(arg2 + 0x6A) == 0x20 ||
        *(u16 *)(arg2 + 0x6A) == 0x21) {
        zoom = 0xBB8;
    }
    cam->w18 += (zoom - cam->w18) / 12;
    if (!(*(u16 *)(arg2 + 0x6A) == 0xF || *(u16 *)(arg2 + 0x6A) == 0x1C || *(u16 *)(arg2 + 0x6A) == 0x1D ||
          *(u16 *)(arg2 + 0x6A) == 0x1E || *(u16 *)(arg2 + 0x6A) == 0x1F || *(u16 *)(arg2 + 0x6A) == 0x20 ||
          *(u16 *)(arg2 + 0x6A) == 0x21) && cam->w18 < 0x1770) {
        cam->w18 = 0x1770;
    }
    if (cam->w18 > 100000) {
        cam->w18 = 100000;
    }
    if (cam->b1E) {
        cam->b1E = cam->w18 >= 0x558D;
    } else {
        cam->b1E = cam->w18 >= 0x55F1;
    }
    func_8003F1E4(cam->b1E);

    if (*(u16 *)(arg2 + 0x6A) == 0x11) {
        scr->yaw = cam->h12;
    } else {
        scr->yaw = (0x400 - ratan2(dx, dz)) & 0xFFF;
    }
    { u16 t = scr->yaw; scr->roll = 0; cam->h12 += math_SignExt12Div((s16)t - cam->h12, 8); }
    cam->h14 += math_SignExt12Div((s16)scr->roll - cam->h14, 8);
    base_yaw = cam->h10;

    for (p = 0; p < 2; p++) {
        yaw = base_yaw;
        cam->h10 = yaw;
        func_8001A538((s32 *)cam, (s32 *)&scr->eye);
        if (p != 0) {
            scr->head = *(CamVec *)(arg3 + 0xB8);
        } else {
            scr->head = *(CamVec *)(arg2 + 0xB8);
        }
        scr->head.vy -= 0xC8;
        if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->unk38, (s32 *)&scr->unk58, (s32)&scr->unk60) &&
            (s16)scr->unk5A < -0x320) {
            func_8001A67C((s16 *)((u8 *)cam + 0x30 + p * 8), (s32 *)&scr->eye, scr->unk38);
            if (D_800A30F0[p]) {
                D_800A30F4[p] += 0x20;
            } else {
                D_800A30F4[p] = 0x10;
                yaw--;
            }
            if (D_800A30F4[p] < 0x10) {
                D_800A30F4[p] = 0x10;
            } else if (D_800A30F4[p] > 0x200) {
                D_800A30F4[p] = 0x200;
            }
            other = yaw + D_800A30F4[p] / 8;
            for (i = 0; i < 2; i++) {
                d = (yaw - other) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                cam->h10 = other + d / 2;
                func_8001A538((s32 *)cam, (s32 *)&scr->eye);
                if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->unk38, (s32 *)&scr->unk58,
                                  (s32)&scr->unk60) &&
                    (s16)scr->unk5A < -0x320) {
                    yaw = cam->h10;
                } else {
                    other = cam->h10;
                }
            }
            yaw = other;
            D_800A30F0[p] = 1;
        } else {
            if (D_800A30F0[p]) {
                D_800A30F4[p] = 0x10;
                yaw++;
            } else {
                D_800A30F4[p] += 0x10;
            }
            if (D_800A30F4[p] < 0x10) {
                D_800A30F4[p] = 0x10;
            } else if (D_800A30F4[p] > 0x100) {
                D_800A30F4[p] = 0x100;
            }
            other = yaw - D_800A30F4[p] / 8;
            for (i = 0; i < 2; i++) {
                d = (yaw - other) & 0xFFF;
                if (d >= 0x800) {
                    d -= 0x1000;
                }
                cam->h10 = other + d / 2;
                func_8001A538((s32 *)cam, (s32 *)&scr->eye);
                if (func_80053614((s32 *)&scr->head, (s32 *)&scr->eye, scr->unk38, (s32 *)&scr->unk58,
                                  (s32)&scr->unk60) &&
                    (s16)scr->unk5A < -0x320) {
                    func_8001A67C((s16 *)((u8 *)cam + 0x30 + p * 8), (s32 *)&scr->eye, scr->unk38);
                    other = cam->h10;
                } else {
                    yaw = cam->h10;
                }
            }
            D_800A30F0[p] = 0;
        }
        if (p != 0) {
            yaw1 = yaw;
        } else {
            yaw0 = yaw;
        }
    }
    yaw = yaw1;
    if (yaw0 >= yaw1) {
        yaw = yaw0;
    }
    if (yaw < 0x80) {
        yaw = 0x80;
    }
    if (yaw > 0x1C0) {
        yaw = 0x1C0;
    }
    cam->h10 = base_yaw;
    cam->h10 += math_SignExt12Div(yaw - base_yaw, 8);
}
