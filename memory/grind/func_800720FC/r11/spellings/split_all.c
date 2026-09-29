/* Page origins for the three scroll pages 4..6 (the page's (x, y) pair; mode m
 * shows page m + 4). */
extern s16 D_8009BCC4[][2];
extern s16 D_8009BCD0[2];
extern s32 D_800A354C;
typedef struct {
    s32 header;     /* sprite sheet header */
    s32 cells;      /* its cell table */
    s32 sprt_out;   /* func_8007352C output cursor */
    s32 ft4_out;    /* func_80073728 output cursor */
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Desc720FC;
typedef struct {
    s32 hdr0;
    s32 hdr4;
    s32 hdr8;
    s32 hdrC[4][2];
    s32 hdr2C;
    s32 hdr30;
} Sheets720FC;
typedef struct {
    u8 unk0[0x14];
    u32 unk14_0 : 4;
    u32 unk14_4 : 6;
    u32 unk14_10 : 22;
} Cfg720FC;
void func_800720FC(s32 arg0, s32 arg1, s32 mode) {
    Desc720FC s;
    u16 rect2[4];
    u16 rect[4];
    u8 *menu;
    s32 *sheets;
    s32 cells6;
    s32 cells4;
    s32 cells3;
    s32 cells2;
    s32 i;
    s32 j;
    s32 d;
    u8 code;
    u8 action;
    s32 c;
    s32 other;
    s16 *timer;

    menu = *(u8 **)(D_800A35A8 + 0x80);
    s.semi = 0;
    s.has_color = 0;
    s.ot_idx = 0x11;
    rect[0] = ((u16 *)D_800A35C0)[0];
    rect[1] = ((u16 *)D_800A35C0)[1];
    rect[2] = ((u16 *)D_800A35C0)[2];
    rect[3] = ((u16 *)D_800A35C0)[3];
    SetDrawArea(((s32 *)arg0)[7], rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[7]);
    ((s32 *)arg0)[7] += 0xC;
    ((u16 *)D_800A35C4)[8] = ((u16 *)D_800A35C0)[4];
    ((u16 *)D_800A35C4)[9] = ((u16 *)D_800A35C0)[5];
    SetDrawOffset(((s32 *)arg0)[8], (u16 *)D_800A35C4 + 8);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[8]);
    ((s32 *)arg0)[8] += 0xC;

    if ((D_800A3578 & 0xFF) == 0) {
        s32 cells1;

        s.x = -10;
        s.y = -5;
        sheets = *(s32 **)(D_800A35A8 + 0x74);
        s.header = sheets[4];
        cells1 = s.header + 0x18;
        if (!(D_800A359C * 2 + D_800A3598 == 3 && D_800A35BC == 6 && mode == 2)) {
            s.cells = cells1;
            s.cells += ((D_800A359C + mode * 3) * 2 + D_800A3598) * 8;
            s.sprt_out = ((s32 *)arg0)[4];
            ((s32 *)arg0)[4] = func_8007352C(&s);
        }
        s.header += 0xC;
        for (i = 0; i < 6; i++) {
            if (!(i == 3 && D_800A35BC == 6 && mode == 2) && D_800A359C * 2 + D_800A3598 != i) {
                s.cells = cells1 + (mode * 6 + i) * 8;
                s.sprt_out = ((s32 *)arg0)[4];
                ((s32 *)arg0)[4] = func_8007352C(&s);
            }
        }
        s.header = sheets[4];
        SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[6]);
        ((s32 *)arg0)[6] += 0xC;
    }

    s.x = 0x90;
    s.y = 0x28;
    s.header = ((Sheets720FC *)arg1)->hdr0;
    cells2 = s.header + 0xC;
    s.cells = cells2;
    s.sprt_out = ((s32 *)arg0)[4];
    ((s32 *)arg0)[4] = func_8007352C(&s);
    ((u16 *)D_800A35C4)[8] = ((u16 *)D_800A35C0)[4] - D_8009BCC4[mode][0];
    ((u16 *)D_800A35C4)[9] = ((u16 *)D_800A35C0)[5] - D_8009BCC4[mode][1];
    if (((D_800A3578 & 0xFF) == 1 || (D_800A3578 & 0xFF) == 3) &&
        D_800A3584 >= 4 && D_800A3584 < 7 && D_800A3580 >= 4 && D_800A3580 < 7) {
        for (i = 0; i < 2; i++) {
            d = D_8009BCC4[D_800A3584 - 4][i] - D_8009BCC4[D_800A3580 - 4][i];
            d *= 30;
            if ((d >= 0 ? d : -d) > (D_8009BCD0[i] >= 0 ? D_8009BCD0[i] : -D_8009BCD0[i])) {
                D_8009BCD0[i] += d * 32 / 488;
            } else {
                D_8009BCD0[i] = d;
            }
            ((s16 *)D_800A35C4)[8 + i] -= D_8009BCD0[i] / 30;
        }
    } else {
        D_8009BCD0[1] = 0;
        D_8009BCD0[0] = 0;
    }
    SetDrawOffset(((s32 *)arg0)[8], (u16 *)D_800A35C4 + 8);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[8]);
    ((s32 *)arg0)[8] += 0xC;
    rect[0] = s.x;
    rect[1] = ((u16 *)D_800A35C0)[1] + s.y;
    rect[2] = 0x160;
    rect[3] = 0x4A;
    SetDrawArea(((s32 *)arg0)[7], rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[7]);
    ((s32 *)arg0)[7] += 0xC;
    SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[6]);
    ((s32 *)arg0)[6] += 0xC;

    s.scale_x = 0x100; /* FAKE: dead store (overwritten by 0x180 below before any
                        * read) - the shipped code performs it: asm lines 356-358
                        * store 0x100 to both scales and lines 364-367 then store
                        * 0x180/0x120; GCC 2.7.2 has no dead-store elimination for
                        * the stack descriptor, so only a source that stores both
                        * emits both. dead-store-fake-exception.md; receipts in
                        * memory/grind/func_800720FC/evidence.md. */
    s.scale_y = 0x100; /* FAKE: same dead store as above (asm line 358). */
    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xB;
    s.header = ((Sheets720FC *)arg1)->hdr4;
    s.scale_x = 0x180;
    s.scale_y = 0x120;
    cells3 = s.header + 0xC;
    s.cells = cells3;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 0);
    s.cells += *(u8 *)(s.header + 2) * 8;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 1);

    if (D_800A354C & 0xA000A000) {
        if ((D_800A3578 & 0xFF) == 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((D_800A354C & 0x20002000) && D_800A3598 != 0) ||
                ((D_800A354C & 0x80008000) && D_800A3598 == 0)) {
                code = menu[(D_800A3580 - 4) * 8 + 3 * 2 + D_800A3598];
                D_800A3584 = code & 0xF;
                if (D_800A3584 != 0xF) {
                    D_800A3578 = code >> 4;
                    func_8005C650(6, 0x7F, 0x7F);
                }
            }
            D_800A3598 = D_800A3598 == 0;
        }
    }
    if (D_800A354C & 0x40004000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A359C++;
    } else if (D_800A354C & 0x10001000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A359C--;
    }
    if (D_800A359C >= 3) {
        D_800A359C = 0;
    } else if (D_800A359C < 0) {
        D_800A359C = 2;
    }

    c = ((rsin((((s32 *)D_800A35C4)[2] & 0x1F) * 128 + 0x1FF) * 63) >> 12) - 0x40;
    s.col_b = c;
    s.col_g = c;
    s.col_r = c;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    s.ot_idx = 1;
    s.header = ((Sheets720FC *)arg1)->hdr8;
    cells4 = s.header + 0xC;
    s.cells = cells4;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 0);
    rect2[2] = 0x111;
    rect2[0] = 0xB7;
    rect2[1] = 0x25;
    rect2[3] = 1;
    func_80069898(arg0, rect2, 1);
    s.ot_idx = 0xA;
    for (i = 0; i < 4; i++) {
        s32 row;

        for (j = 0, row = i * 2; j < 2; j++) {
            s32 cells5;

            if (i == D_800A359C && j == D_800A3598 && (D_800A3578 & 0xFF) == 0) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            if (row + j != 3 || D_800A35BC != 6 || mode != 2) {
                s.header = ((s32 *)arg1 + i * 2)[j + 3];
            } else {
                s.header = ((Sheets720FC *)arg1)->hdr30;
            }
            cells5 = s.header + 0xC;
            s.cells = cells5;
            s.ft4_out = ((s32 *)arg0)[1];
            ((s32 *)arg0)[1] = func_80073728(&s, 0);
        }
    }
    s.has_color = 0;
    s.header = ((Sheets720FC *)arg1)->hdr2C;
    cells6 = s.header + 0xC;
    s.cells = cells6;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 0);

    if (D_800A3578 == 0) {
        for (i = 0; i < D_800A35B0 + 1; i++) {
            if (D_800A354C & (0x10 << (i * 16))) {
                s32 ctx = i * 3;
                s32 slot;

                func_8005C650(2, 0x7F, 0x7F);
                slot = D_800A3560[ctx];
                D_800A3560[ctx + 2] = 0xFF;
                if (slot == 5 || slot == 16) {
                    D_800A3580 = 0;
                    other = i == 0 ? 3 : 0;
                    D_800A3560[other + 2] = 0xFF;
                    D_800A3560[ctx] = 0xFF;
                } else {
                    D_800A3580 = 1;
                }
                timer = D_800A35C8; /* FAKE: pointer alias (pointer-alias-fake-exception.md):
                                     * `D_800A35C8[0] = 0xF; D_800A35C8[1] = 0x14;` gives
                                     * each element address its own pseudo; cse.c
                                     * use_related_value rewrites the second as
                                     * (plus base 2), so the base is used twice and loop.c
                                     * hoists it out of the player loop (asm then stores
                                     * via a callee-saved base). Through the local, the
                                     * [1] address folds to a constant in the MEM and the
                                     * single remaining use of the local is substituted
                                     * by loop.c's large-loop single-usage rule, giving
                                     * the target's two direct gp_rel stores. Receipts:
                                     * memory/grind/func_800720FC/q21/ + evidence.md. */
                timer[0] = 0xF;
                timer[1] = 0x14;
                ((s16 *)D_800A35C4)[3] = 0;
                ((s16 *)D_800A35C4)[2] = 0;
                goto end;
            }
        }
        if (D_800A354C & 0x400040) {
            action = menu[(D_800A3580 - 4) * 8 + D_800A359C * 2 + D_800A3598];
            func_8005C650(1, 0x7F, 0x7F);
            if (action != 0xD || D_800A35BC != 6) {
                ((Cfg720FC *)D_800A3568)->unk14_4 = action;
            } else {
                ((Cfg720FC *)D_800A3568)->unk14_4 = 0x25;
            }
            D_800A35A0 = 1;
        }
    }
end:
    func_80072E10(arg0);
    func_80073200(arg0);
    func_8005C6D0();
}
