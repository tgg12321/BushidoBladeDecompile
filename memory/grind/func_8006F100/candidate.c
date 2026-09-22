extern s16 D_800A355C;
extern s16 D_800A35C8[];
extern s16 D_800A3590[];
extern void func_80072E10(s32);
extern void func_80073200(s32);
extern s32 func_80073C78();

typedef struct {
    u8 unk0[2];
    u8 count;
    u8 unk3[5];
    s16 unk8;
    u8 unkA[2];
} Hdr_8006F100;

typedef struct {
    u16 x;
    u16 y;
    u8 unk4[2];
    u8 w;
    u8 h;
} Ent_8006F100;

typedef struct {
    Hdr_8006F100 hdr[2];
    Ent_8006F100 ent[1];
} Obj_8006F100;

typedef struct {
    Hdr_8006F100 *hdr;
    Ent_8006F100 *ent;
    s32 unk08;
    s32 ret;
    s32 unk10;
    s32 unk14;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 unk28;
} Spr_8006F100;

void func_8006F100(s32 arg0) {
    Spr_8006F100 s;
    s32 i;
    s32 base;
    Obj_8006F100 *obj;
    s32 sel;
    s32 t0;
    s16 t1;
    s16 t2;
    s16 dx;
    s16 dy;

    if (D_800A3550 != 0) {
        D_800A3550 += 0x10;
        if (D_800A3550 >= 0x100) {
            D_800A3550 = 0xFF;
            func_8006F038(arg0);
            D_800A3550 = 0;
        } else {
            func_8006F038(arg0);
        }
        func_8006ECF4(arg0);
        func_80072E10(arg0);
        func_80073200(arg0);
        return;
    }

    if (D_800A355C >= 0x79) {
        D_800A3584 = 3;
    }
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.unk14 = 0x13;
    s.unk10 = 0;
    s.unk28 = 0;
    base = *(s32 *)(D_800A35A8 + 0x58);
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        obj = *(Obj_8006F100 **)(base + D_800A3560[i * 3 + 2] * 4);
        sel = 1;
        if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
            if (D_8009BC7C[D_800A3561] & 2) {
                sel = 1;
            } else {
                sel = 0;
            }
        } else if (D_8009BC7C[D_800A3560[i * 3 + 1]] & 2) {
            sel = 0;
        }
        s.hdr = &obj->hdr[sel];
        if (i != 0) {
            s.hdr->unk8 = 0x40;
        } else {
            s.hdr->unk8 = 0;
        }
        s.ent = obj->ent;
        t0 = -(D_800A35C8[i] * 800) / 20;
        t1 = t0;
        {
            s32 idx = s.hdr->count - 1;
            dx = s.ent[idx].x + s.ent[idx].w - obj->ent[0].x;
            dy = s.ent[idx].y + s.ent[idx].h - obj->ent[0].y;
        }
        t2 = -(D_800A35C8[i] * 664) / 20;
        if (i != 0) {
            t1 = -t0;
        }
        {
            /* FAKE: constant-holder local (named-local-fake-exception). The
             * target adds the screen-centre offset to the (sign-extended)
             * shake offset BEFORE adding the table origin -- `addiu 0x140`
             * then `addu tbl,v0`. Spelled with the literal, fold-const.c's
             * `associate:` (split_tree) reassociates `tbl + (t1 + 0x140)` to
             * `(tbl + t1) + 0x140` / `tbl + 0x140 + t1` and the order is lost;
             * a local operand is not TREE_CONSTANT, so the tree keeps the
             * grouping and cse propagates 0x140 back into the addiu
             * (byte-neutral: 266 insns either way). Block scope keeps loop.c
             * from hoisting it into a callee-save. */
            s32 cx = 0x140;

            s.x = D_8009BC94[i][D_800A3590[i]].x + (t1 + cx) - ((dx * s.scale_x >> 8) / 2);
        }
        {
            /* FAKE: same constant-holder mechanism as `cx` above, for the
             * vertical centre 0x9D. */
            s32 cy = 0x9D;

            s.y = D_8009BC94[i][D_800A3590[i]].y + (t2 + cy) - ((dy * s.scale_y >> 8) / 2);
        }
        s.ret = *(s32 *)(arg0 + 4);
        if (i != 0) {
            *(s32 *)(arg0 + 4) = func_80073C78(&s, 0x1C0, 1);
        } else {
            *(s32 *)(arg0 + 4) = func_80073C78(&s, 0xE40, 0);
        }
        if (D_800A35C8[i] > 0) {
            D_800A35C8[i]--;
        }
        if (D_800A35C8[i] == 10) {
            func_8005C650(7, 0x7F, 0x7F);
        }
        if (D_800A35C8[i] < 0) {
            D_800A35C8[i] = 0;
            func_8005C650(8, 0x7F, 0x7F);
        }
    }
    D_800A355C++;
}
