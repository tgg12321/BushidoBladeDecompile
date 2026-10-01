s32 func_80030BA8(u8 *arg0) {
    s32 i;
    s32 old_val;

    for (i = 0; i < 12; i++) {
        u16 val = D_80106A78[i].unk_02;
        if ((unsigned)(val - 0x12) < 12u) {
            continue;
        }
        if ((s16)val == -1) {
            continue;
        }
        if (D_80106A78[i].unk_50 != 0) {
            continue;
        }
        {
            s32 bc = *(s32 *)(arg0 + 0xBC);
            s32 py = D_80106A78[i].unk_2C.y;
            if (bc - 0x64 >= py) {
                continue;
            }
            if (py >= bc + 0x64) {
                continue;
            }
        }
        {
            s32 dx = *(s32 *)(arg0 + 0xF4) - D_80106A78[i].unk_2C.x;
            s32 dz = *(s32 *)(arg0 + 0xFC) - D_80106A78[i].unk_2C.z;
            if (dx * dx + dz * dz > 0xF423F) {
                continue;
            }
        }
        if (func_80030B10(arg0, (s16)val) == 0) {
            return -1;
        }
        old_val = D_80106A78[i].unk_02;
        D_80106A78[i].unk_02 = -1;
        if (old_val == 0xE) {
            s32 a0val = D_800A36F2[0] ^ 0xE;
            func_80032854(a0val != 0, 0x2F, arg0 + 0xF4, 0);
        } else {
            func_80032854(*(s16 *)(arg0 + 4), 0x11, arg0 + 0xF4, 0);
        }
        return old_val;
    }
    return -1;
}
