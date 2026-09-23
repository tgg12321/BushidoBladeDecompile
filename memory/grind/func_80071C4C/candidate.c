void func_80071C4C(s32 arg0) {
    Spr_8006F100 s;
    s32 i;
    s32 base;
    Obj_8006F100 *obj;
    s32 sel;
    s16 dx;
    s16 dy;

    base = *(s32 *)(D_800A35A8 + 0x58);
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        s32 ctx = i * 3;

        if (D_800A3560[ctx] != 5 && D_800A3560[ctx] != 16) {
            s.unk14 = 1;
            s.unk10 = 0;
            s.scale_x = 0x100;
            s.scale_y = 0x100;
            s.unk28 = 0;
            obj = *(Obj_8006F100 **)(base + D_800A3560[ctx + 2] * 4);
            sel = 1;
            if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                if (D_8009BC7C[D_800A3561] & 2) {
                    sel = 1;
                } else {
                    sel = 0;
                }
            } else if (D_8009BC7C[D_800A3560[ctx + 1]] & 2) {
                sel = 0;
            }
            s.hdr = &obj->hdr[sel];
            if (i != 0) {
                s.hdr->unk8 = 0x40;
            } else {
                s.hdr->unk8 = 0;
            }
            s.ent = obj->ent;
            {
                s32 idx = s.hdr->count - 1;
                dx = s.ent[idx].x + s.ent[idx].w - obj->ent[0].x;
                dy = s.ent[idx].y + s.ent[idx].h - obj->ent[0].y;
            }
            s.x = D_8009BC94[i][D_800A3590[i]].x + 0x140 - ((dx * s.scale_x >> 8) / 2);
            s.y = D_8009BC94[i][D_800A3590[i]].y + 0x9D - ((dy * s.scale_y >> 8) / 2);
            s.ret = *(s32 *)(arg0 + 4);
            if (i != 0) {
                *(s32 *)(arg0 + 4) = func_80073C78(&s, 0x1C0, 1);
            } else {
                *(s32 *)(arg0 + 4) = func_80073C78(&s, 0xE40, 0);
            }
        }
    }

    D_800A3550 += 8;
    if (D_800A3550 >= 0xFF) {
        D_800A3550 = 0xFF;
        D_800A3578 = 1;
        func_8005C650(6, 0x7F, 0x7F);
        if ((u32)D_800A35BC < 2) {
            s32 mode = func_80071C20();

            D_800A35A0 = 1;
            *(s32 *)(D_800A3568 + 0x14) =
                (*(s32 *)(D_800A3568 + 0x14) & ~0x3F0) | ((mode & 0x3F) << 4);
        } else if (D_800A35BC == 4) {
            D_800A3584 = 5;
        } else if (D_800A35BC == 6) {
            D_800A3584 = 6;
            D_800A359C = 1;
            D_800A3598 = 1;
        } else {
            D_800A35A0 = 1;
        }
        for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
            s32 dst = i * 10;
            s32 ctx = i * 3;

            *(u8 *)(D_800A3568 + dst) = D_800A3560[ctx];
        }
        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
            s32 dst = i * 10;
            s32 ctx = i * 3;

            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560[ctx + 2];
        }
    }
    func_8006F038(arg0);
}
