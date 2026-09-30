typedef struct {
    s32 *header;
    s8 *table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 pad20;
    s32 pad24;
    s8 has_color;
} Env_8006CFBC;


extern s32 D_800A3524;
extern s32 D_800A34FC;
extern s32 g_gpu_ot_ptr;
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);

s32 func_8006CFBC(s32 *arg0) {
    Env_8006CFBC s;
    s16 counts[2];
    s32 *table;
    s16 outer;
    s16 column;
    s16 row;
    s16 result;
    s8 *temp;

    result = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.ot_idx = 8;
    s.has_color = 0;
    s.semi = 0;

    outer = 0;
    do {
        counts[0] = counts[1] = 0;
        s.y = outer * 16;
        column = 0;
        do {
            s.header = (s32 *)table[column + 8];
            temp = (s8 *)s.header + 0xC;
            s.table = temp;
            for (row = 0; row < 2; row++) {
                if (*(u8 *)(D_800A3524 + outer + 0x17) &
                    ((1 << (row * 4)) << column)) {
                    s.x = row * 280 + counts[row] * 23;
                    s.out = arg0[5];
                    arg0[5] = func_8007352C((s32)&s);
                    counts[row]++;
                }
            }
            column++;
        } while (column < 4);

        row = 0;
        do {
            if (counts[row] == 0) {
                s.x = row * 280;
                result |= 1 << row;
                s.header = (s32 *)table[12];
                temp = (s8 *)s.header + 0xC;
                s.table = temp;
                s.out = arg0[5];
                arg0[5] = func_8007352C((s32)&s);
            }
            row++;
        } while (row < 2);
        outer++;
    } while (outer < 3);

    row = 0;
    do {
        if (*(s32 *)(D_800A34FC + 0x28) == 0x50005) {
            s.header = (s32 *)table[17];
        } else if ((result >> row) & 1) {
            s.header = (s32 *)table[19];
        } else {
            s.header = (s32 *)table[18];
        }
        s.x = row * 280;
        s.y = 0;
        temp = (s8 *)s.header + 0xC;
        s.table = temp;
        s.out = arg0[5];
        arg0[5] = func_8007352C((s32)&s);
        row++;
    } while (row < 2);

    s.header = (s32 *)table[14];
    SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x20, arg0[7]);
    arg0[7] += 0xC;

    {
        u16 rect[4];
        rect[2] = 0xE1;
        rect[0] = 0xCF;
        rect[1] = 0x25;
        rect[3] = 1;
        func_80069898((GameObj *)arg0, rect, 0x11);
    }
    return (s16)result;
}
