void func_80017A44(u8 *a0, u8 *a1) {
    s32 pos[3];
    s32 center[3];
    s32 flag;
    u8 *record_base;
    u8 *record;
    s16 *groups;
    s16 *outer;
    s16 *inner;
    s32 valid_count;
    s32 i;
    s32 i2;
    s32 j;
    s32 group_count;
    s32 group_id;
    s32 dist;
    s32 value;

    center[0] = 0;
    center[1] = 0;
    center[2] = 0;
    valid_count = 0;
    SetRotMatrix(*(u8 **)(a0 + 0xC));
    SetTransMatrix(*(u8 **)(a0 + 0xC));

    record_base = *(u8 **)(a1 + 0xC);
    i = 0;
    if (*(s16 *)a0 > 0) {
        record = record_base;
        do {
            *(s32 *)(record + 0x18) = *(s16 *)(*(u8 **)(a0 + 4) + i * 8 + 6);
            RotTrans((s16 *)(*(u8 **)(a0 + 4) + i * 8), pos, &flag);
            pos[0] <<= 7;
            pos[1] <<= 7;
            pos[2] <<= 7;
            if (*(s32 *)(record + 0x18) >= 0) {
                valid_count++;
                center[0] += pos[0];
                center[1] += pos[1];
                center[2] += pos[2];
            }
            *(s32 *)(record + 0x00) = pos[0];
            *(s32 *)(record + 0x04) = pos[1];
            i++;
            *(s32 *)(record + 0x0C) = 0;
            *(s32 *)(record + 0x10) = 0;
            *(s32 *)(record + 0x14) = 0;
            *(s32 *)(record + 0x1C) = 0;
            *(s32 *)(record + 0x20) = 0;
            *(s32 *)(record + 0x08) = pos[2];
            record += 0x40;
        } while (i < *(s16 *)a0);
    }

    center[0] /= valid_count;
    center[1] /= valid_count;
    center[2] /= valid_count;

    i2 = 0;
    if (*(s16 *)a0 > 0) {
        record = record_base;
        do {
            value = math_Distance3D_16(center, (s32 *)record) >> 8;
            if (value > 0x100) {
                value = 0x100;
            }
            *(s32 *)(record + 0x34) = value;
            i2++;
            record += 0x40;
        } while (i2 < *(s16 *)a0);
    }

    groups = *(s16 **)(a0 + 8);
    while ((group_count = *groups++) != 0) {
        group_id = *groups++;
        outer = groups;
        for (i = 0; i < group_count - 1; i++, outer++) {
            dist = math_Distance3D_16(center, (s32 *)(record_base + *outer * 0x40));
            inner = groups + i + 1;
            for (j = i + 1; j < group_count; j++, inner++) {
                if (dist < math_Distance3D_16(center, (s32 *)(record_base + *inner * 0x40))) {
                    func_80017848(a1, group_id, *outer, *inner);
                } else {
                    func_80017848(a1, group_id, *inner, *outer);
                }
            }
        }
        groups += group_count;
    }
}
