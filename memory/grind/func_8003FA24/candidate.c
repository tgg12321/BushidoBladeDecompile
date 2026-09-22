/* func_8003FA24 — honest work-in-progress candidate.
 * Measured 2026-09-22: sandbox --disable all = 129/263, build 262 insns.
 * This is not completion-grade and must not be copied to src/ until score 0,
 * full oracle verification, ablation, and fresh adversarial review all pass.
 */
u8 *func_8003FA24(SceneRec *rec, s16 *cmds, u8 *cur) {
    struct SceneObjInit {
        s16 count;
        s16 flags;
        u8 *points;
        s16 *groups;
        void *matrix;
        u8 *point_end;
        s32 unk14;
        s32 unk18;
        s32 unk1C;
    } init;
    extern s32 obj_CalcOffset(s32, s32);
    extern s32 func_80017D84(u8 *);
    extern void func_80045230(s32);
    extern void func_80052C10();
    extern u16 **D_80103608[];
    extern s16 D_80094AEC[];
    u8 *obj;
    u16 *src;
    u16 *block;
    s16 count;
    u16 flags;
    s32 packet_type;
    s32 block_count;
    s32 loaded_count;
    s32 mode;
    s32 point_count;
    s32 value;
    s16 *packet;
    u16 *dst;

    obj = rec->obj;
    dst = (u16 *)cur;
    src = D_80103608[*(s16 *)(obj + 4)][*(s16 *)(obj + 2)];
    count = *src;
    init.count = count;
    init.points = cur;
    src += 2;
    count--;
    if ((s16)count != -1) {
        u16 *coord = dst + 2;
        do {
            *dst = *src++;
            coord[-1] = *src++;
            *coord = *src++;
            dst += 4;
            coord += 4;
            count--;
        } while ((s16)count != -1);
    }
    cur = (u8 *)dst;
    if ((u32)src & 3) {
        src++;
    }

    block = src;
    point_count = 0;
    count = *src++;
    while (count != 0) {
        flags = *src++;
        if (flags & 8) {
            point_count += count * 6;
        } else {
            point_count += count * 3;
        }
        src += D_80094AEC[(flags >> 3) & 3] * count;
        count = *src++;
    }

    init.point_end = cur;
    cur = cur + obj_CalcOffset(init.count, point_count);
    {
        u8 *aligned = cur;
        if ((u32)cur & 3) {
            aligned = cur + 2;
        }
        cur = aligned;
    }

    src = block;
    packet = (s16 *)0x1F800000;
    init.groups = packet;
    block_count = *(s16 *)src++;
    loaded_count = block_count;
    while (block_count != 0) {
        flags = *src++;
        packet_type = (s16)flags >> 3;
        mode = packet_type & 3;
        switch ((packet_type & 1) != 0) {
        case 1:
            loaded_count--;
            if ((s16)loaded_count != -1) {
              flags = packet_type & 2;
              do {
                if (flags) {
                    value = ((u32)src[9] << 16) | src[8];
                } else {
                    value = ((u32)src[8] << 16) | src[7];
                }
                *packet = 4;
                packet++;
                *packet = 2;
                packet++;
                *packet = value & 0xFF;
                packet++;
                *packet = (value >> 8) & 0xFF;
                packet++;
                *packet = (value >> 16) & 0xFF;
                packet++;
                *packet = (u32)value >> 24;
                packet++;
                src += D_80094AEC[mode];
                loaded_count--;
              } while ((s16)loaded_count != -1);
            }
            break;
        default:
            loaded_count--;
            if ((s16)loaded_count != -1) {
              flags = packet_type & 2;
              do {
                value = ((u32)src[7] << 16) | src[6];
                *packet = 3;
                packet++;
                *packet = 2;
                packet++;
                *packet = value & 0xFF;
                packet++;
                *packet = (value >> 8) & 0xFF;
                packet++;
                *packet = (value >> 16) & 0xFF;
                packet++;
                src += D_80094AEC[mode];
                loaded_count--;
              } while ((s16)loaded_count != -1);
            }
            break;
        }
        *packet++ = 0;
        block_count = *(s16 *)src++;
        loaded_count = block_count;
    }

    {
        u8 *aligned = cur;
        if ((u32)cur & 3) {
            aligned = cur + 2;
        }
        cur = aligned;
    }
    func_80045230((s32)cur);
    if (*src != 0) {
        ((void (*)(const char *))func_80052C10)(D_80010D8C);
    }
    func_8003FE40((s16 *)init.points, init.count, cmds);

    init.matrix = obj + 0x18;
    init.flags = 0xE00;
    *(s16 *)rec = func_80017D84((u8 *)&init);
    {
        u8 *result;
        *(u8 **)(obj + 0x60) = init.point_end;
        rec->inner.count = 0;
        rec->inner.objs[4] = 0;
        result = cur;
        if ((u32)cur & 3) {
            result = cur + 2;
        }
    *(u16 *)((u8 *)rec + 4) = *(u16 *)rec;
    *(u8 **)((u8 *)rec + 8) = obj + 0x18;
    *(u8 **)((u8 *)rec + 0xC) = *(u8 **)(obj + 0x60);
    *(void **)((u8 *)rec + 0x10) = (u8 *)rec + 0x2C;
    *((u8 *)rec + 6) = 0;
    *((u8 *)rec + 7) = 0;
        return result;
    }
}
