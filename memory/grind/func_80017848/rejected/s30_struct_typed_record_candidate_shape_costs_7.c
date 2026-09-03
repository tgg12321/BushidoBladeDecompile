typedef struct {
    s32 unk0[6];
    s32 f18;
    s32 na;
    s32 nb;
    u8 la[8];
    u8 lb[8];
    s32 unk34[3];
} IngRec;

typedef struct {
    s32 dist;
    u16 b;
    s16 a;
    s32 dist3;
    s32 owner;
} IngLink;

s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    IngRec *slots;
    IngRec *p;
    IngRec *q;
    IngRec *rec;
    IngLink *lnk;
    u8 *link;
    u8 *rec_a;
    u8 *rec_b;
    s32 dist;
    s32 t;
    s32 i;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(IngRec **)(ctx + 0xC);
    if (slots[slot_a].f18 >= 0) {
        if (slots[slot_b].f18 >= 0) {
            return 0;
        }
    }

    p = *(IngRec **)(ctx + 0xC);
    i = 0;
    t = p[slot_a].na;
    if (i < t) {
        q = *(IngRec **)(ctx + 0xC);
        lnk = *(IngLink **)(ctx + 0x10);
        rec = &q[slot_a];
        do {
            if (lnk[rec->la[i]].b == slot_b) {
                return 0;
            }
            i++;
        } while (i < rec->na);
        p = q;
    }

    i = 0;
    if (i < p[slot_a].nb) {
        rec = &(*(IngRec **)(ctx + 0xC))[slot_a];
        do {
            if ((*(IngLink **)(ctx + 0x10))[rec->lb[i]].a == slot_b) {
                return 0;
            }
            i++;
        } while (i < rec->nb);
    }

    dist = math_Distance3D((s32 *)(*(u8 **)(ctx + 0xC) + (slot_a << 6)),
                           (s32 *)(*(u8 **)(ctx + 0xC) + (slot_b << 6)));
    link = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 0x6) << 4);
    *(s32 *)(link + 0x0) = dist;
    *(s32 *)(link + 0x8) = dist * 3;
    *(s32 *)(link + 0xC) = arg1;
    *(s32 *)(link + 0x4) = (slot_a << 16) | slot_b;

    rec_a = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_a + 0x1C);
    *(s32 *)(rec_a + 0x1C) = i + 1;
    *(u8 *)(rec_a + i + 0x24) = *(u16 *)(ctx + 0x6);

    rec_b = (u8 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_b + 0x20);
    *(s32 *)(rec_b + 0x20) = i + 1;
    *(u8 *)(rec_b + i + 0x2C) = *(u16 *)(ctx + 0x6);

    *(s16 *)(ctx + 0x6) = *(u16 *)(ctx + 0x6) + 1;
    return 1;
}
