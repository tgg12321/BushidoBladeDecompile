/* REJECTED s40 (synthesis modality) -- score 38/204 -> 95/204, build_insns
 * 198 -> 177 (21 FEWER real instructions than baseline, yet substantially
 * WORSE score: the instruction mix diverges further from target even though
 * the count shrinks).
 *
 * Construct: merge ALL SIX occurrences of `*(s32 *)(obj + 0xBC)` (two in
 * block1 at -0x320, two in block2 at -0x834/+0x1004, one in the flags==3
 * comparison, one as `y` in the flags==4 branch) into a single local `by`
 * read ONCE before the first func_80053614 call and reused across BOTH
 * calls. This extends s39's within-block-only y0 merge (which combined
 * only the 2 block1 occurrences and was also worse, 38->73/204) to a
 * cross-call-surviving carrier.
 *
 * Result confirms s39's mechanism at a larger scope: GCC's own combine/CSE
 * decides independently, per occurrence, whether to keep the source's
 * literal re-read or fold it -- forcing an explicit shared carrier (which
 * must now be call-clobber-safe, i.e. spilled/reloaded or kept in a
 * callee-saved register across TWO calls) picks a DIFFERENT allocation
 * than target's actual per-site re-reads. Target's own 204-instruction
 * body evidently re-materializes `obj + 0xBC` at (most of) these sites
 * rather than caching it across the calls -- consistent with s39's
 * "explicit named carrier changes which register/instruction sequence
 * holds the shared value, and that different allocation is worse."
 *
 * Measured on: s40 chassis (candidate.c's s22-s39-banked 38/204 body +
 * func_80053614 s32-return prerequisite + header externs, `by` local
 * introduced immediately after `obj` is resolved, no FAKE constructs
 * present). kill_scope: instance.
 */
extern s16 Judge;
extern s32 ratan2(s32, s32);
extern u8 D_8009A820;
extern u8 D_8009A821;
extern s32 D_800F6610;

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
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
        flags = (&D_8009A821)[i * 2] << 8;
        if ((flags & 0x1000) != 0) {
            obj = *(s32 *)arg0;
        }

        if (*(u16 *)(arg0 + 0x6A) == 0x13 || *(u16 *)(arg0 + 0x6A) == 6) {
            flags += *(s16 *)(obj + 0x1CA);
        } else {
            flags += ratan2(D_800F6608.w0 - *(s32 *)(obj + 0xF4),
                             D_800F6610 - *(s32 *)(obj + 0xFC));
        }

        s32 by = *(s32 *)(obj + 0xBC);

        sin_p = &Judge + (flags & 0xFFF);
        scale = (&D_8009A820)[i * 2] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        cos_p = &Judge + ((flags + 0x400) & 0xFFF);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = by - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = by - 0x320;
        pt1[2] = z;

        flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (flags != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = by - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = by + 0x1004;
        pt1[2] = z;

        flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8) << 1)) + 1;
        if (flags == 3) {
            if (hit1[1] - by < 5) {
                flags = 0;
            }
        } else if (flags == 4) {
            s32 dx = hit0[0] - *(s32 *)(obj + 0xB8);
            s32 dz = hit0[2] - *(s32 *)(obj + 0xC0);
            if (0x3D0900 < dx * dx + dz * dz) {
                s32 y = by;
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
