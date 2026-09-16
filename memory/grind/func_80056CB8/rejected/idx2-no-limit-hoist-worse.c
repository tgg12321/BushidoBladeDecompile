/* REJECTED s30 (solver modality) — score 52/204, build_insns 201 (baseline: 38/204, 198).
 *
 * Hand-carried idx2 accumulator (idx2 = start * 2; idx2 += 2 in the
 * for-statement increment clause), loop guard rewritten as `i < start + 2`
 * with NO separate `limit` local at all, both `(&D_8009A821)[i * 2]` /
 * `(&D_8009A820)[i * 2]` reads replaced with `[idx2]`.
 *
 * Motivation: a fresh read of asm/funcs/func_80056CB8.s shows target's $fp
 * register really is a loop-carried strength-reduced i*2 accumulator
 * (sll $fp,$v1,2 at entry; addiu $fp,$fp,2 at the loop tail), and target
 * never hoists a stable `limit` -- it re-spills and re-adds `start+2` fresh
 * every iteration. This combination (idx2 present AND no limit hoist) had
 * never been tried in isolation before (s22/s27 only tried idx2 ON TOP OF
 * the limit-hoisted chassis).
 *
 * Result: worse in the same direction and roughly the same magnitude as
 * every prior idx2 attempt (s22: 55/204; s27: 55/204). Confirms GCC treats
 * a hand-written idx2 as an ordinary competing user pseudo, not as the
 * compiler's own strength-reduced giv -- loop.c's own strength_reduce would
 * have to promote a literal `[i * 2]` expression automatically for target's
 * shape to emerge, and the s26-banked loop.c:3823 mechanism record already
 * shows that predicate rejecting i*2 promotion on every chassis measured so
 * far in this ledger.
 *
 * DO NOT re-propose either idx2 combination (with or without a hoisted
 * `limit`) without a genuinely new predicate-changing structural lever
 * elsewhere in the function first.
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
    s32 i;
    s32 idx2;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    idx2 = start * 2;
    for (i = start; i < start + 2; i++, idx2 += 2) {
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;

        obj = arg0;
        flags = (&D_8009A821)[idx2] << 8;
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
        scale = (&D_8009A820)[idx2] << 8;
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
