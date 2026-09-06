/* REJECTED s41 (2026-09-06, rederive) - cell F1: sandbox --disable all = 22 at 127/119 on the HEAD src/ings.c:820 INCLUDE_ASM chassis, BASE = 3 at 127/127.
 * Both loops as `for (i = 0;; i++) { if (i >= count) break; ... }` (exit test at the loop top, no rotation, no duplicate_loop_exit_test). The guard add / base add pair collapses to one add and the bottom bound reload disappears.
 * No FAKE constructs. Do not re-propose; re-measure only if the chassis or a rule scope changes. */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
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

    p = *(u8 **)(ctx + 0xC);
    sh = slot_a << 6;
    lnk = *(u8 **)(ctx + 0x10);
    base = (u8 *)(sh + (s32)p);
    for (i = 0;; i++) {
        if (i >= *(s32 *)(base + 0x1C)) {
            break;
        }
        if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) + (s32)lnk + 0x4) == slot_b) {
            return 0;
        }
    }

    p = *(u8 **)(ctx + 0xC);
    sh2 = slot_a << 6;
    lnk = *(u8 **)(ctx + 0x10);
    base = (u8 *)(sh2 + (s32)p);
    for (i = 0;; i++) {
        if (i >= *(s32 *)(base + 0x20)) {
            break;
        }
        if (*(s16 *)((*(u8 *)(base + i + 0x2C) << 4) + (s32)lnk + 0x6) == slot_b) {
            return 0;
        }
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
