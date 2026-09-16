/* REJECTED s55 (rederive modality) — pt0/pt1 merged into one shared 8-word
 * buffer instead of two separate 4-word arrays.
 *
 * s53/s54 flagged this as a "genuinely untried, correctness-safe" axis: since
 * pt0/pt1's first-call values are fully dead (overwritten) before the second
 * func_80053614 call writes new values, sharing storage cannot corrupt
 * anything (unlike the already-KILLED hit0/hit1 idea, which s54 proved
 * unsafe on correctness grounds — hit0 is read AFTER hit1 is written).
 *
 * MEASURED WORSE: sandbox --disable all: 38/204 -> 103/204 (build_insns
 * 198 -> 207). Converting the two fixed-address stack arrays into
 * pointer-indirected slices of one buffer forces every store/load through
 * computed-pointer addressing instead of frame-relative fixed-offset
 * addressing — a real, substantial codegen cost, not a neutral relayout.
 *
 * Only the declaration differs from the s22-s54-banked 38/204 candidate.c
 * body (memory/grind/func_80056CB8/candidate.c); every use-site read/write
 * is byte-for-byte identical to that body.
 * --------------------------------------------------------------------- */
extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820[];
extern u8 D_8009A821[];
extern s32 D_800F6610;

void func_80056CB8(s32 arg0) {
    s32 pts[8];
    s32 *pt0 = pts;
    s32 *pt1 = pts + 4;
    s32 hit0[4];
    s32 hit1[4];
    s32 work[4];
    s32 start;
    s32 limit;
    s32 i;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    limit = start + 2;
    for (i = start; i < limit; i++) {
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;

        obj = arg0;
        flags = D_8009A821[i * 2] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            flags += *(s16 *)(obj + 0x1CA);
        } else {
            flags += ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                             D_800F6610 - *(s32 *)(obj + 0xFC));
        }

        sin_p = &Judge + (flags & 0xFFF);
        scale = D_8009A820[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        cos_p = &Judge + ((flags + 0x400) & 0xFFF);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1;
        if (flags == 3) {
            if (hit1[1] - *(s32 *)(obj + 0xBC) < 5) {
                flags = 0;
            }
        } else if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = *(s32 *)(obj + 0xBC);
                if (y - hit1[1] >= 0) {
                    if (y - hit1[1] >= 0x3E9) {
                        flags = 5;
                    }
                } else {
                    if (hit1[1] - y >= 0x3E9) {
                        flags = 5;
                    }
                }
            }
        }
        *(s8 *)(arg0 + 0x444 + i) = (s8)flags;
    }
}
