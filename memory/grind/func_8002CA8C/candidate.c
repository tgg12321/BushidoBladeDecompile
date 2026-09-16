
extern u8 D_800F5F68;
extern s32 func_8002D320(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq);
extern s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq);

void func_8002CA8C(u8 *a0, s32 a1, s32 a2) {
    u8 *scr = (u8 *)0x1F8002B8;
    s32 id = *(s16 *)(a0 + 4);
    u8 *recbase = (u8 *)&D_800F5F68 + id * 0x1B8;
    u8 *rec;
    s32 base = id * 0x108;
    s32 hitMask = 0;
    s32 seenMask = 0;
    s32 i;

    for (i = 0, rec = recbase; i < 0x16; i++, rec += 0x14) {
        s32 off = base + i * 0xC;
        s32 rejected;
        s32 c;
        s32 r;
        s32 hit;

        if (*(s16 *)(a0 + 0x26C) == 0 && i >= 6 && i <= 9) {
            continue;
        }

        r = *(u16 *)(rec + 0xC);
        rejected = 0;
        c = *(s32 *)((u8 *)0x1F8000A8 + off);
        if (*(s32 *)(scr + 0x84) < c - r || c + r < *(s32 *)(scr + 0x78)) {
            rejected = 1;
        } else {
            c = *(s32 *)((u8 *)0x1F8000AC + off);
            if (*(s32 *)(scr + 0x88) < c - r || c + r < *(s32 *)(scr + 0x7C)) {
                rejected = 1;
            } else {
                c = *(s32 *)((u8 *)0x1F8000B0 + off);
                if (*(s32 *)(scr + 0x8C) < c - r || c + r < *(s32 *)(scr + 0x80)) {
                    rejected = 1;
                }
            }
        }
        if (rejected != 0) {
            continue;
        }

        if (a1 != 0) {
            hit = func_8002D780(0, scr, (s32 *)&SCR[id].j[i + 4],
                                r, *(u16 *)(rec + 0xE));
            if (hit != 0) {
                if (*(s16 *)rec != 0 && a2 != 0) {
                    if (func_8002D780(1, scr, (s32 *)0,
                                      *(u16 *)(rec + 0x10),
                                      *(u16 *)(rec + 0x12)) != 0) {
                        hitMask |= 1 << i;
                    }
                }
            }
        } else {
            hit = func_8002D320(0, scr, (s32 *)&SCR[id].j[i + 4],
                                r, *(u16 *)(rec + 0xE));
            if (hit != 0) {
                if (*(s16 *)rec != 0 && a2 != 0) {
                    if (func_8002D320(1, scr, (s32 *)0,
                                      *(u16 *)(rec + 0x10),
                                      *(u16 *)(rec + 0x12)) != 0) {
                        hitMask |= 1 << i;
                    }
                }
            }
        }
        if (hit != 0) {
            seenMask |= 1 << i;
        }
    }

    *(s32 *)(scr + 0xB4) = seenMask;
    *(s32 *)(scr + 0xC4) = hitMask;
}
