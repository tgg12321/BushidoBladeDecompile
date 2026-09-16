/* REJECTED - s63 W1: K2 + sanctioned do{}while(0) wrap around the INNER do-while of both loops (preheader outside). 12 at 127/127. The extra loop_depth lifts lnk (80/82) above sh (85/86) in global.c's order: lnk takes a1, sh a2 (sll a2,s4,6 / lw a1,16(s2) / addu a0,a2,v0); the copy dest (78) STILL seats v0 - the wrap changes priorities only, never the v0/a0 hard conflicts the a3 seat needs (E-s61-3/E-s62-1). Measured s63 on HEAD src/ings.c:820, FAKE-annotated wrap present. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *lnk;
    u8 *q2;
    u8 *lnk2;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
    s32 t2;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    sh = slot_a << 6;
    i = 0;
    q = *(u8 **)(ctx + 0xC);
    t = sh + (s32)q;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        p = q;
        q = *(u8 **)(ctx + 0x10);
        lnk = q;
        q = (u8 *)(sh + (s32)p);
        /* FAKE: do-while(0) wrap, mechanism: flow.c loop_depth weighting of reg_n_refs feeding global.c allocno priority; lever-exhaustion: memory/grind/func_80017848/hypotheses.md s43/s62 */
        do {
        do {
            if (*(u16 *)((*(u8 *)(q + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q + 0x1C));
        } while (0);
    }

    sh2 = slot_a << 6;
    i = 0;
    q2 = *(u8 **)(ctx + 0xC);
    t2 = sh2 + (s32)q2;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        p = q2;
        q2 = *(u8 **)(ctx + 0x10);
        lnk2 = q2;
        q2 = (u8 *)(sh2 + (s32)p);
        /* FAKE: do-while(0) wrap, mechanism: flow.c loop_depth weighting of reg_n_refs feeding global.c allocno priority; lever-exhaustion: memory/grind/func_80017848/hypotheses.md s43/s62 */
        do {
        do {
            if (*(s16 *)((*(u8 *)(q2 + i + 0x2C) << 4) +
                         (s32)lnk2 + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q2 + 0x20));
        } while (0);
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
