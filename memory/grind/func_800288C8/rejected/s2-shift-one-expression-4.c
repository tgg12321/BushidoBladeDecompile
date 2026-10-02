typedef struct {
    LeafPos unk00[2][3];
    LeafPos unk48[2][2];
    LeafPos unk78[2][2];
    LeafPos unkA8[2][22];
} ScrPadT;
#define SPADT ((ScrPadT *)0x1F800000)

extern s32 D_800A3144[];

void func_800288C8(void) {
    s32 lzc_out;
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
    s32 dist;
    s32 tbl;
    s32 pen;
    s32 bias;
    s32 flag1;
    s32 flag2;
    s32 facing;

    if (g_practice_menu_table[0].unk_6A == 0x28 || g_practice_menu_table[1].unk_6A == 0x28) {
        return;
    }
    for (c = 0; c < 2; c++) {
        SPADT->unk78[c][0].x = SPADT->unkA8[c][1].x;
        SPADT->unk78[c][0].y = SPADT->unkA8[c][1].y;
        SPADT->unk78[c][0].z = SPADT->unkA8[c][1].z;
        SPADT->unk78[c][1].x = (SPADT->unkA8[c][15].x + SPADT->unkA8[c][19].x) / 2;
        SPADT->unk78[c][1].y = (SPADT->unkA8[c][15].y + SPADT->unkA8[c][19].y) / 2;
        SPADT->unk78[c][1].z = (SPADT->unkA8[c][15].z + SPADT->unkA8[c][19].z) / 2;
    }
    hits = 0;
    push1_x = push1_z = push2_x = push2_z = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            dx = SPADT->unk78[0][i].x - SPADT->unk78[1][j].x;
            dy = SPADT->unk78[0][i].y - SPADT->unk78[1][j].y;
            dz = SPADT->unk78[0][i].z - SPADT->unk78[1][j].z;
            reach = D_800A3144[i] + D_800A3144[j];
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
                dist = (u32)g_sqrt_table_u8[dist] >> 3;
            } else {
                __asm__ volatile ("move  $12,%0": :"r"(tbl):"$12","$13","$14","$15","memory");
                __asm__ volatile ("mtc2  $12,$30": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("nop   ": : :"$12","$13","$14","$15","memory");
                __asm__ volatile ("move  $12,%0": :"r"(&lzc_out):"$12","$13","$14","$15","memory");
                __asm__ volatile ("swc2  $31,($12)": : :"$12","$13","$14","$15","memory");
                {
                    s32 lz;
                    s32 shift;
                    shift = 0x16 - (lzc_out & ~1);
                    tbl = g_sqrt_table_u8[(u32)dist >> shift];
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
            bias = (g_practice_menu_table[1].unk_20 - g_practice_menu_table[0].unk_20) / 4;
            push1_x += (dx * (pen * (0x400 + bias)) / dist) >> 10;
            push1_z += (dz * (pen * (0x400 + bias)) / dist) >> 10;
            push2_x -= (dx * (pen * (0x400 - bias)) / dist) >> 10;
            push2_z -= (dz * (pen * (0x400 - bias)) / dist) >> 10;
        }
    }
    if (hits == 0) {
        return;
    }
    flag1 = 0;
    if (g_practice_menu_table[0].unk_6A == 0x13 || g_practice_menu_table[0].unk_6A == 0x1B
        || g_practice_menu_table[0].unk_6A == 0x30) {
        if (g_practice_menu_table[0].unk_14E > 0x9C4) {
            flag1 = 1;
        }
    }
    flag2 = 0;
    if (g_practice_menu_table[1].unk_6A == 0x13 || g_practice_menu_table[1].unk_6A == 0x1B
        || g_practice_menu_table[1].unk_6A == 0x30) {
        if (g_practice_menu_table[1].unk_14E > 0x9C4) {
            flag2 = 1;
        }
    }
    facing = (g_practice_menu_table[0].unk_1C8.vy - g_practice_menu_table[1].unk_1C8.vy) & 0xFFF;
    if (facing >= 0x800) {
        facing = 0x1000 - facing;
    }
    if (flag1) {
        if (flag2) {
            if (facing > 0x600) {
                func_80032854(0, 0x21, (u8 *)&g_practice_menu_table[0].unk_F4, 0);
                g_practice_menu_table[1].unk_286 = 0xF;
                g_practice_menu_table[0].unk_286 = 0xF;
                func_80027A58((s32 *)&g_practice_menu_table[0]);
                func_80027A58((s32 *)&g_practice_menu_table[1]);
                hits = 0;
            }
        } else {
            if (facing < 0x400 && g_practice_menu_table[1].unk_6A == 0x15) {
                g_practice_menu_table[1].unk_286 = 0x10;
                func_80027A58((s32 *)&g_practice_menu_table[1]);
            }
            if (facing > 0x600 && g_practice_menu_table[1].unk_6A == 0x15) {
                if (g_practice_menu_table[0].unk_0E == 6 || g_practice_menu_table[0].unk_0E == 7
                    || g_practice_menu_table[1].unk_0E == 6 || g_practice_menu_table[1].unk_0E == 7) {
                    func_80032854(1, 0x21, (u8 *)&g_practice_menu_table[1].unk_F4, 0);
                    func_80032854(1, 0x2D, (u8 *)&g_practice_menu_table[1].unk_F4, 0);
                    g_practice_menu_table[1].unk_286 = 5;
                } else {
                    D_800A38A8 = 1;
                    D_800A3876 = 0;
                }
            }
        }
    } else if (flag2) {
        if (facing < 0x400 && g_practice_menu_table[0].unk_6A == 0x15) {
            g_practice_menu_table[0].unk_286 = 0x10;
            func_80027A58((s32 *)&g_practice_menu_table[0]);
        }
        if (facing > 0x600 && g_practice_menu_table[0].unk_6A == 0x15) {
            if (g_practice_menu_table[0].unk_0E == 6 || g_practice_menu_table[0].unk_0E == 7
                || g_practice_menu_table[1].unk_0E == 6 || g_practice_menu_table[1].unk_0E == 7) {
                func_80032854(0, 0x21, (u8 *)&g_practice_menu_table[0].unk_F4, 0);
                func_80032854(0, 0x2D, (u8 *)&g_practice_menu_table[0].unk_F4, 0);
                g_practice_menu_table[0].unk_286 = 5;
            } else {
                D_800A38A8 = 1;
                D_800A3876 = 1;
            }
        }
    }
    if (hits != 0) {
        g_practice_menu_table[0].unk_134.vx += push1_x;
        g_practice_menu_table[0].unk_134.vz += push1_z;
        g_practice_menu_table[1].unk_134.vx += push2_x;
        g_practice_menu_table[1].unk_134.vz += push2_z;
    }
}
