extern s32 obj_CalcOffset(s32, s32);
extern s32 func_80017D84(u8 *);
extern void func_80045230(s32);
extern u16 **D_80103608[];
extern s16 D_80094AEC[];
s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2);

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
    u8 *obj;
    u16 *src;
    u16 *block;
    s16 count;
    u16 flags;
    s32 packet_type;
    s32 mode;
    s32 point_count;
    s32 value;
    s32 n;
    s16 group_count;
    s16 group_id;
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
    if (count != -1) {
        do {
            dst[0] = *src++;
            dst[1] = *src++;
            dst[2] = *src++;
            dst += 4;
            count--;
        } while (count != -1);
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
    cur = ((u32)cur & 3) ? cur + 2 : cur;

    src = block;
    packet = (s16 *)0x1F800000;
    init.groups = packet;
    for (n = *(s16 *)src++, count = n; n != 0; n = *(s16 *)src++, count = n) {
        flags = *src++;
        packet_type = (s16)flags >> 3;
        mode = packet_type & 3;
        if (packet_type & 1) {
            while (--count != -1) {
                if (packet_type & 2) {
                    value = ((u32)src[9] << 16) | src[8];
                } else {
                    value = ((u32)src[8] << 16) | src[7];
                }
                group_count = 4;
                *packet++ = group_count;
                group_id = 2;
                *packet++ = group_id;
                *packet++ = value & 0xFF;
                *packet++ = (value >> 8) & 0xFF;
                *packet++ = (value >> 16) & 0xFF;
                *packet++ = (u32)value >> 24;
                src += D_80094AEC[mode];
            }
        } else {
            while (--count != -1) {
                /* FAKE: identical arms (gouraud and flat triangle records keep
                 * the colour at the same offset); jump2 cross-jumping merges
                 * them, leaving loop.c's hoisted `packet_type & 2` test. */
                if (packet_type & 2) {
                    value = ((u32)src[7] << 16) | src[6];
                } else {
                    value = ((u32)src[7] << 16) | src[6];
                }
                group_count = 3;
                *packet++ = group_count;
                group_id = 2;
                *packet++ = group_id;
                *packet++ = value & 0xFF;
                *packet++ = (value >> 8) & 0xFF;
                *packet++ = (value >> 16) & 0xFF;
                src += D_80094AEC[packet_type & 3];
            }
        }
        *packet++ = 0;
    }

    cur = ((u32)cur & 3) ? cur + 2 : cur;
    func_80045230((s32)cur);
    if (*src != 0) {
        func_80052C10(D_80010D8C);
    }
    func_8003FE40((s16 *)init.points, init.count, cmds);

    init.matrix = obj + 0x18;
    init.flags = 0xE00;
    *(s16 *)rec = func_80017D84((u8 *)&init);
    *(u8 **)(obj + 0x60) = init.point_end;
    rec->inner.count = 0;
    rec->inner.objs[4] = 0;
    cur = ((u32)cur & 3) ? cur + 2 : cur;
    *(u16 *)((u8 *)rec + 4) = *(u16 *)rec;
    *(u8 **)((u8 *)rec + 8) = obj + 0x18;
    *(u8 **)((u8 *)rec + 0xC) = *(u8 **)(obj + 0x60);
    *(void **)((u8 *)rec + 0x10) = (u8 *)rec + 0x2C;
    *((u8 *)rec + 6) = 0;
    *((u8 *)rec + 7) = 0;
    return cur;
}
