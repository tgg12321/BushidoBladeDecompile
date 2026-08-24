void func_80044800(void) {
    s16 local[3];
    s16 sp18[10];
    s32 pad[4];
    register s32 i asm("s4");
    register s8 *obj asm("s2");
    s32 frame;
    s32 angle;
    s16 *scan;
    s16 *base_ptr;
    s32 type;
    s16 cos_val, sin_val;
    s32 fade;
    s32 anim_offset;
    s16 *judge_base;

    obj = *(s8 **)(&D_800A9D04);
    if (D_800A9CFE <= 0) return;

    base_ptr = (s16 *)((s8 *)&D_800A9D04 - 0xA);
    judge_base = Judge;
    anim_offset = 0;
    i = 0;
    do {
        frame = *(s32 *)(obj + 0x58);
        if (frame >= 0) {
            if (frame < *base_ptr) {
                local[0] = 0;
                angle = *(s32 *)(obj + 0x5C);
                local[2] = 0;
                scan = (s16 *)(*(s32 *)((s8 *)base_ptr + 6) + frame * 12);
                local[1] = (s16)angle;
                func_8004A348(local, (s32 *)sp18);
                local[0] = *(u16 *)scan; scan++;
                local[1] = *(u16 *)scan; scan++;
                local[2] = *(u16 *)scan; scan++;
                *(s16 *)(obj + 0x10) = *(u16 *)scan; scan++;
                *(s16 *)(obj + 0x12) = *(u16 *)scan;
                *(s16 *)(obj + 0x14) = *(u16 *)(scan + 1);
                *(s16 *)(obj + 0x6) = 0;
                func_800417D0((s32 *)obj);
                MulMatrix2((s32 *)sp18, (s32 *)(obj + 0x18));

                cos_val = judge_base[(angle + 0x400) & 0xFFF];

                {
                    s32 cz, sz, cx, sx;
                    cz = cos_val * local[2];
                    angle &= 0xFFF;
                    sin_val = judge_base[angle];
                    sz = sin_val * local[2];
                    cx = cos_val * local[0];
                    sx = sin_val * local[0];

                    *(s32 *)(obj + 0x2C) += (sz + cx) >> 12;
                    *(s32 *)(obj + 0x30) += local[1];
                    *(s32 *)(obj + 0x34) += (cz - sx) >> 12;
                }

                frame++;
                if (frame >= *base_ptr) {
                    *(s16 *)(obj + 0x60) = 0x1000;
                }
                *(s32 *)(obj + 0x58) = frame;

                type = *(s16 *)((s8 *)base_ptr + 2);
                if (type == 0x12) {
                    s8 *saved_obj = obj;
                    s32 idx;
                    obj = (s8 *)(*(s32 *)((s8 *)base_ptr + 0xE) + anim_offset);
                    idx = (frame - 1) + *base_ptr;
                    scan = (s16 *)(D_800A9D00 + idx * 12);
                    local[0] = *(u16 *)scan; scan++;
                    local[1] = *(u16 *)scan; scan++;
                    local[2] = *(u16 *)scan; scan++;
                    *(s16 *)(obj + 0x10) = *(u16 *)scan; scan++;
                    *(s16 *)(obj + 0x12) = *(u16 *)scan;
                    *(s16 *)(obj + 0x14) = *(u16 *)(scan + 1);
                    *(s16 *)(obj + 0x6) = 0;
                    func_800417D0((s32 *)obj);

                    *(s32 *)(obj + 0x2C) += local[0];
                    *(s32 *)(obj + 0x30) += local[1];
                    *(s32 *)(obj + 0x34) += local[2];
                    obj = saved_obj;
                    goto append_list;
                }
            } else {
                fade = *(s16 *)(obj + 0x60) - 0x40;
                if (fade < 0) {
                    fade = 0;
                }
                if (fade == 0 && *(s16 *)((s8 *)base_ptr + 2) != 4) {
                    *(s32 *)(obj + 0x58) = -2;
                }
                *(s16 *)(obj + 0x60) = (s16)fade;
            }
append_list:
            {
                s32 *list = (s32 *)D_800A3820;
                D_800A3820 = (s32)(list + 1);
                *list = (s32)obj;
            }
        }
        obj += 0x68;
        i++;
        anim_offset += 0x68;
    } while (i < D_800A9CFE);
}
