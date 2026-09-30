/* RETRO-AUDIT 2026-09-29 FAIL -- func_8006CFBC reopened (Q37 class C, owner rulings 803d0fea1).
 * Landed on main in 5c79ba8f1 (src/text1b.c); this is that landed text, verbatim from main
 * as of the reopen, banked before the body went back to INCLUDE_ASM.
 * FAIL: union word view `Counts_8006CFBC { s32 word; s16 half[2]; }` landed 2026-09-20, before Q33 (2026-09-29); also the 3-write carrier `value` with no FAKE annotation or admitting ruling (score 8 without it).
 * Detail: tmp/audit-2026-09-29/review/batch_00.md (gitignored), tmp/audit-2026-09-29/SUMMARY.md;
 * rulings: docs/grind/owner-rulings-2026-09-26.md Q37/Q38 (803d0fea1).
 * Reopen notes: Removed with the body: the private Env_8006CFBC / Counts_8006CFBC typedefs (no other user). Kept: the extern block between them and the body (D_800A3524, D_800A34FC, g_gpu_ot_ptr, func_8007352C, func_8006E480, SetDrawMode).
 * DO NOT resubmit this body as-is. */

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

typedef union {
    s32 word;
    s16 half[2];
} Counts_8006CFBC;

extern s32 D_800A3524;
extern s32 D_800A34FC;
extern s32 g_gpu_ot_ptr;
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);

s32 func_8006CFBC(s32 *arg0) {
    Env_8006CFBC s;
    Counts_8006CFBC counts;
    s32 *table;
    s16 outer;
    s16 column;
    s16 row;
    s16 result;
    s32 value;

    result = 0;
    table = *(s32 **)(arg0[1] + 0x30);
    s.ot_idx = 8;
    s.has_color = 0;
    s.semi = 0;

    outer = 0;
    do {
        counts.word = 0;
        s.y = outer * 16;
        column = 0;
        do {
            s.header = (s32 *)table[column + 8];
            value = (s32)s.header + 0xC;
            s.table = (s8 *)value;
            for (row = 0; row < 2; row++) {
                if (*(u8 *)(D_800A3524 + outer + 0x17) &
                    ((1 << (row * 4)) << column)) {
                    s.x = row * 280 + counts.half[row] * 23;
                    s.out = arg0[5];
                    arg0[5] = func_8007352C((s32)&s);
                    counts.half[row]++;
                }
            }
            column++;
        } while (column < 4);

        row = 0;
        do {
            if (counts.half[row] == 0) {
                s.x = row * 280;
                result |= 1 << row;
                s.header = (s32 *)table[12];
                value = (s32)s.header + 0xC;
                s.table = (s8 *)value;
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
        value = (s32)s.header + 0xC;
        s.table = (s8 *)value;
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
