s32 func_80030BA8(u8 *arg0) {
    s32 i = 0;
    s32 empty_slot = -1;
    Obj80106A78 *p = D_80106A78;
    s32 old_val;

    loop:;
    {
        u16 val = p->unk_02;
        s32 sval;
        if ((unsigned)(val - 0x12) < 12u) {
            goto next;
        }
        sval = (s16)val;
        if (sval == empty_slot) {
            goto next;
        }
        if (p->unk_50 != 0) {
            goto next;
        }
        {
            s32 bc = *(s32 *)(arg0 + 0xBC);
            s32 pos2e = p->unk_2C.y;
            if (bc - 0x64 >= pos2e) {
                goto next;
            }
            if (pos2e >= bc + 0x64) {
                goto next;
            }
        }
        {
            s32 dx = *(s32 *)(arg0 + 0xF4) - p->unk_2C.x;
            s32 dz = *(s32 *)(arg0 + 0xFC) - p->unk_2C.z;
            s32 range_sq = 0xF423F;
            i++;
            if (dx * dx + dz * dz > range_sq) {
                goto loop_test;
            }
        }
        if (func_80030B10(arg0, sval) == 0) {
            return -1;
        }
        old_val = (s32)p->unk_02;
        p->unk_02 = (s16)empty_slot;
        if (old_val == 0xE) {
            s32 a0val = D_800A36F2 ^ 0xE;
            func_80032854(a0val != 0, 0x2F, arg0 + 0xF4, 0);
        } else {
            func_80032854(*(s16 *)(arg0 + 4), 0x11, arg0 + 0xF4, 0);
        }
        return old_val;
    }
    next:
    i++;
    loop_test:
    if (i < 12) {
        p++;
        goto loop;
    }
    return -1;
}
