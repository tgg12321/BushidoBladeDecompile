s32 func_80030BA8(PracticeMenuRec *arg0) {
    Obj80106A78 *p;
    s32 i;
    s32 old_val;

    for (i = 0, p = D_80106A78; i < 12; i++, p++) {
        s16 kind = p->unk_02;
        if (kind >= 0x12 && kind < 0x1E) {
            continue;
        }
        if (kind == -1) {
            continue;
        }
        if (p->unk_50 != 0) {
            continue;
        }
        {
            s32 bc = arg0->unk_B8.vy;
            s32 py = p->unk_2C.y;
            if (bc - 0x64 >= py) {
                continue;
            }
            if (py >= bc + 0x64) {
                continue;
            }
        }
        {
            s32 dx = arg0->unk_F4.x - p->unk_2C.x;
            s32 dz = arg0->unk_F4.z - p->unk_2C.z;
            if (dx * dx + dz * dz > 0xF423F) {
                continue;
            }
        }
        if (func_80030B10((u8 *)arg0, kind) == 0) {
            return -1;
        }
        old_val = p->unk_02;
        p->unk_02 = -1;
        if (old_val == 0xE) {
            func_80032854((D_800A36F2[0] ^ 0xE) != 0, 0x2F, (u8 *)&arg0->unk_F4, 0);
        } else {
            func_80032854(arg0->unk_04, 0x11, (u8 *)&arg0->unk_F4, 0);
        }
        return old_val;
    }
    return -1;
}
