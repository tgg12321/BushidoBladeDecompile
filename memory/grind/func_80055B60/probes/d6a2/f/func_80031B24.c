void func_80031B24(void) {
    u8 *scr = (u8 *)0x1F8002B8;
    s32 i;
    s32 deep;
    Obj80106A78 *obj;
    Vec3i32 *seg = (Vec3i32 *)scr;
    PracticeMenuRec *ch;
    s32 other;
    u16 st;
    u8 *rec;
    s32 j;
    s32 hit;
    s32 diff;
    s32 r;
    s32 kind;
    s32 flag;

    obj = D_80106A78;
    for (i = 0; i < 12; i++, obj++) {
        if (obj->unk_02 == -1) continue;
        if (obj->unk_04 == 0) continue;
        if (obj->unk_50 == 0) continue;
        other = obj->unk_06 == 0;
        ch = &g_practice_menu_table[other];
        st = ch->unk_6A;
        if (st == 4 || st == 0x14 || st == 0xF || st == 0x1C || st == 0x1D || st == 0x1E ||
            st == 0x1F || st == 0x20 || st == 0x21 || st == 0x11) {
            continue;
        }

        seg[0] = obj->unk_38;
        seg[1] = obj->unk_2C;
        *(Vec3i32 **)(scr + 0x60) = &seg[0];
        *(Vec3i32 **)(scr + 0x64) = &seg[1];
        func_8002E838(scr);

        hit = 0;
        rec = &D_800F5F68[other * 0x1B8];
        for (j = 0; j < 22; j++, rec += 0x14) {
            s32 *pos;
            if (ch->unk_26C == 0 && j >= 6 && j <= 9) continue;
            pos = (s32 *)&SPAD->unkA8[other][j];
            hit = func_8002EA24(scr, pos, *(u16 *)(rec + 0xC), *(u16 *)(rec + 0xE));
            if (hit != 0) {
                deep = 0;
                if (*(s16 *)rec != 0 && D_8008E194[obj->unk_02].unkD == 0) {
                    deep = func_8002EA24(scr, pos, *(u16 *)(rec + 0x10), *(u16 *)(rec + 0x12)) != 0;
                }
                break;
            }
        }
        if (hit == 0) continue;

        diff = (ch->unk_1C8.vy - ratan2(obj->unk_44.x, obj->unk_44.z)) & 0xFFF;
        if (diff >= 0x800) diff = 0x1000 - diff;
        func_800274BC((s32 *)&obj->unk_44, &D_800A37E8);
        obj->unk_2C.x -= obj->unk_44.x / 2;
        obj->unk_2C.y -= obj->unk_44.y / 2;
        obj->unk_2C.z -= obj->unk_44.z / 2;
        r = func_80027AD8(1, (u8 *)ch, j, diff, deep, &D_8008E194[obj->unk_02], 0, &flag);
        if (r == 2) continue;
        if (r != 0) {
            func_80032854((obj->unk_02 ^ D_800A36F2[0]) != 0, 0x2B, (u8 *)&SPAD->unkA8[other][j], 0);
            func_8002FF20(obj, *(s16 *)(rec + 2));
            obj->unk_04 = 0;
            st = ch->unk_6A;
            if (st == 8 || st == 0x23) {
                g_disp_fade = 1;
            }
            continue;
        }
        kind = obj->unk_02;
        if (kind == 0xF) {
            func_80032854(other ^ 1, 0xE, (u8 *)&SPAD->unkA8[other][j], &D_800A37E8);
            obj->unk_02 = -1;
            continue;
        }
        if (kind == 0xE) {
            func_80032854((D_800A36F2[0] ^ 0xE) != 0, 0x2F, (u8 *)&obj->unk_2C, 0);
        } else if (flag == 0) {
            func_80032854((kind ^ D_800A36F2[0]) != 0, 0x2B, (u8 *)&SPAD->unkA8[other][j], 0);
        }
        func_80031890(scr, obj, j);
        obj->unk_04 = 0;
    }
}
