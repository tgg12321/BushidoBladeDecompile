typedef struct { s32 x, y, z; } HVec3;
typedef struct {
    HVec3 misc[10];
    HVec3 pt[2][2];
    HVec3 joint[2][22];
} HitScr;
#define HSCR ((HitScr *)0x1F800000)
extern s32 D_800A3144[];
extern u16 D_80102322;
extern s32 D_80102448;
extern s32 D_80102450;
void func_800288C8(void) {
    s32 sp_tmp;
    s32 hits;
    s32 i;
    s32 j;
    s32 c;
    s32 push1_x;
    s32 push1_z;
    s32 push2_x;
    s32 push2_z;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 reach;
    s32 *rad;
    s32 dist_sq;
    s32 dist;
    s32 tbl;
    s32 pen;
    s32 bias;
    s32 lock1;
    s32 lock2;
    s32 ang;

    if (D_80101F32 == 0x28 || D_8010237E == 0x28) {
        return;
    }
    for (c = 0; c < 2; c++) {
        HSCR->pt[c][0].x = HSCR->joint[c][1].x;
        HSCR->pt[c][0].y = HSCR->joint[c][1].y;
        HSCR->pt[c][0].z = HSCR->joint[c][1].z;
        HSCR->pt[c][1].x = (HSCR->joint[c][15].x + HSCR->joint[c][19].x) / 2;
        HSCR->pt[c][1].y = (HSCR->joint[c][15].y + HSCR->joint[c][19].y) / 2;
        HSCR->pt[c][1].z = (HSCR->joint[c][15].z + HSCR->joint[c][19].z) / 2;
    }
    rad = D_800A3144;
    hits = 0;
    push2_z = 0;
    push2_x = 0;
    push1_z = 0;
    push1_x = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            dx = HSCR->pt[0][i].x - HSCR->pt[1][j].x;
            dy = HSCR->pt[0][i].y - HSCR->pt[1][j].y;
            dz = HSCR->pt[0][i].z - HSCR->pt[1][j].z;
            reach = rad[i] + rad[j];
            if (dx > reach || dx < -reach || dy > reach || dy < -reach
                || dz > reach || dz < -reach) {
                continue;
            }
            dist = dx * dx + dy * dy + dz * dz;
            if (reach * reach < dist) {
                continue;
            }
            hits++;
            tbl = dist;
            if ((u32)dist < 0x400) {
                dist = (u32)*(((u8 *)&D_8008D118) + dist) >> 3;
            } else {
                __asm__ volatile(
                    "move   $12, %0\n"
                    "mtc2   $12, $30\n"
                    "nop\n"
                    "nop\n"
                    :: "r"(tbl) : "$12", "$13", "$14", "$15", "memory");
                __asm__ volatile(
                    "move   $12, %0\n"
                    "swc2   $31, 0($12)\n"
                    :: "r"(&sp_tmp) : "$12", "$13", "$14", "$15", "memory");
                {
                    s32 lz;
                    s32 shift;
                    lz = ~1;
                    lz &= sp_tmp;
                    shift = 0x16 - lz;
                    tbl = *(((u8 *)&D_8008D118) + ((u32)dist >> shift));
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            pen = reach - dist;
            if (pen > 0x80) {
                pen = 0x80;
            }
            if (dist == 0) {
                dist = 1;
            }
            bias = (D_80102334 - D_80101EE8) / 4;
            push1_x += (dx * (pen * (0x400 + bias)) / dist) >> 10;
            push1_z += (dz * (pen * (0x400 + bias)) / dist) >> 10;
            push2_x -= (dx * (pen * (0x400 - bias)) / dist) >> 10;
            push2_z -= (dz * (pen * (0x400 - bias)) / dist) >> 10;
        }
    }
    if (hits == 0) {
        return;
    }
    lock1 = 0;
    if (D_80101F32 == 0x13 || D_80101F32 == 0x1B || D_80101F32 == 0x30) {
        if (D_80102016 > 0x9C4) {
            lock1 = 1;
        }
    }
    lock2 = 0;
    if (D_8010237E == 0x13 || D_8010237E == 0x1B || D_8010237E == 0x30) {
        if (D_80102462 > 0x9C4) {
            lock2 = 1;
        }
    }
    ang = (*(s16 *)(&D_80101EC8 + 0x1CA) - *(s16 *)(&D_80101EC8 + 0x44C + 0x1CA)) & 0xFFF;
    if (ang >= 0x800) {
        ang = 0x1000 - ang;
    }
    if (lock1) {
        if (lock2) {
            if (ang > 0x600) {
                func_80032854(0, 0x21, &D_80101EC8 + 0xF4, 0);
                *(s16 *)(&D_80101EC8 + 0x44C + 0x286) = 0xF;
                *(s16 *)(&D_80101EC8 + 0x286) = 0xF;
                func_80027A58((s32 *)&D_80101EC8);
                func_80027A58((s32 *)(&D_80101EC8 + 0x44C));
                hits = 0;
            }
        } else {
            if (ang < 0x400 && D_8010237E == 0x15) {
                *(s16 *)(&D_80101EC8 + 0x44C + 0x286) = 0x10;
                func_80027A58((s32 *)(&D_80101EC8 + 0x44C));
            }
            if (ang > 0x600 && *(u16 *)(&D_80101EC8 + 0x44C + 0x6A) == 0x15) {
                if ((u32)((u16)D_80101ED6 - 6) < 2 || (u32)(D_80102322 - 6) < 2) {
                    func_80032854(1, 0x21, &D_80101EC8 + 0x44C + 0xF4, 0);
                    func_80032854(1, 0x2D, &D_80101EC8 + 0x44C + 0xF4, 0);
                    *(s16 *)(&D_80101EC8 + 0x44C + 0x286) = 5;
                } else {
                    D_800A38A8 = 1;
                    D_800A3876 = 0;
                }
            }
        }
    } else if (lock2) {
        if (ang < 0x400 && D_80101F32 == 0x15) {
            *(s16 *)(&D_80101EC8 + 0x286) = 0x10;
            func_80027A58((s32 *)&D_80101EC8);
        }
        if (ang > 0x600 && *(u16 *)(&D_80101EC8 + 0x6A) == 0x15) {
            if ((u32)((u16)D_80101ED6 - 6) < 2 || (u32)(D_80102322 - 6) < 2) {
                func_80032854(0, 0x21, &D_80101EC8 + 0xF4, 0);
                func_80032854(0, 0x2D, &D_80101EC8 + 0xF4, 0);
                *(s16 *)(&D_80101EC8 + 0x286) = 5;
            } else {
                D_800A38A8 = 1;
                D_800A3876 = 1;
            }
        }
    }
    if (hits != 0) {
        *(s32 *)(&D_80101EC8 + 0x134) += push1_x;
        *(s32 *)(&D_80101EC8 + 0x13C) += push1_z;
        *(s32 *)(&D_80101EC8 + 0x44C + 0x134) += push2_x;
        *(s32 *)(&D_80101EC8 + 0x44C + 0x13C) += push2_z;
    }
}
