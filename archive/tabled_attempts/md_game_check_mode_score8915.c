
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern s32 D_800A3368;
extern s32 D_800A3230;
extern s16 D_800A3678;
extern s16 D_800A367A;
extern s16 D_800A367C;
extern u8 D_800A4750[];
extern u8 D_800A6690[];
extern s16 D_800A7FE0[];
extern s16 D_800A87E0[];
extern s32 D_800F66A0[];
extern void func_80052C10(void);

void md_game_check_mode(s16 *list, s16 val) {
    s32 i;
    s32 j;
    u16 first;
    u16 flag;
    u8 *ent;
    u8 *e2;
    u16 lo;
    u16 hi;
    s16 coord;
    s16 aux;
    s16 row;
    s16 col;
    s16 v10;
    s16 v12;
    s16 v14;
    s16 fn_idx;
    s16 c;
    s16 *tptr;
    u16 look;

    i = 0;
    while ((s16)*list != -1) {
        first = (u16)*list++;
        flag = (u16)*list++;
        ent = &D_800A4750[(s16)i * 0x10];
        i++;

        *(s16 *)(ent + 4) = flag;
        if ((flag & 0x8000) == 0) {
            ent[7] = 0;
        } else {
            *(s16 *)(ent + 4) = flag & 0x7FFF;
            ent[7] = 8;
        }

        *(s16 *)(ent + 0) = 0xC;
        ent[6] = 0;
        *(s16 *)(ent + 2) = val;

        coord = (s16)first;
        aux = (coord >= 0) ? coord : (s16)(coord + 0x1F);
        row = (s16)(aux >> 5);
        col = (s16)(coord - (row << 5));

        *(s32 *)(ent + 8) = col * 2000 - 0x7D00;
        *(s32 *)(ent + 12) = row * 2000 - 0x7D00;
    }
    list++;
    D_800A3368 = (s16)i;

    i = 0;
    if ((s16)*list != -1) {
        do {
            first = (u16)*list++;
            e2 = &D_800A6690[(s16)i * 0x68];
            i++;

            e2[1] = 0;
            *(s32 *)(e2 + 0xC) = 0;
            *(s16 *)(e2 + 8) = 0;
            *(s16 *)(e2 + 0xA) = 4;
            *(s16 *)(e2 + 2) = first;
            *(s16 *)(e2 + 4) = val;

            lo = (u16)*list++;
            *(u32 *)(e2 + 0x2C) = lo;
            hi = (u16)*list++;
            *(u32 *)(e2 + 0x2C) = lo | ((u32)hi << 16);

            lo = (u16)*list++;
            *(u32 *)(e2 + 0x30) = lo;
            hi = (u16)*list++;
            *(u32 *)(e2 + 0x30) = lo | ((u32)hi << 16);

            lo = (u16)*list++;
            *(u32 *)(e2 + 0x34) = lo;
            hi = (u16)*list++;
            *(u32 *)(e2 + 0x34) = lo | ((u32)hi << 16);

            *(s16 *)(e2 + 0x10) = *list++;
            *(s16 *)(e2 + 0x12) = *list++;
            *(s16 *)(e2 + 0x14) = *list++;

            v10 = *(s16 *)(e2 + 0x10);
            v12 = *(s16 *)(e2 + 0x12);
            v14 = *(s16 *)(e2 + 0x14);
            if (v10 == v12) {
                if (v14 == 1) {
                    e2[0] = 0;
                } else {
                    e2[0] = 1;
                }
            } else {
                if (v14 == 0) {
                    e2[0] = 0;
                } else {
                    e2[0] = 1;
                }
            }

            fn_idx = *(s16 *)(e2 + 8);
            ((void (*)(u8 *, u8 *))D_800F66A0[fn_idx])(e2 + 0x10, e2 + 0x18);

            e2[0x58] = 0;
        } while ((s16)*list != -1);
    }
    list++;

    j = 0;
    do {
        s16 *row_ptr = &D_800A7FE0[j * 32 + 0x1F];
        i = 0x1F;
        do {
            *row_ptr-- = -1;
            i--;
        } while (i >= 0);
        j++;
    } while (j < 32);

    j = 0;
    if ((s16)*list != -1) {
        do {
            c = (s16)*list++;
            aux = (c >= 0) ? c : (s16)(c + 0x1F);
            row = (s16)(aux >> 5);
            col = (s16)(c - (row << 5));

            D_800A7FE0[row * 32 + col] = j;

            tptr = &D_800A87E0[j];
            do {
                look = *list++;
                j++;
                *tptr++ = look;
            } while ((look & 0x8000) == 0);
        } while ((s16)*list != -1);
    }

    if (j > D_800A3230) D_800A3230 = j;
    if (D_800A3230 >= 0x3E8) func_80052C10();

    D_800A3678 = 0;
    D_800A367A = 0;
    D_800A367C = 0;
}
