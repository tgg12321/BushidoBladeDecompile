/* REJECTED s11 (forensics modality): making the doubled byte-table index
 * `j` the loop's PRIMARY induction variable (for (j = start*2; j < start*2+4;
 * j += 2) { s32 i = j >> 1; ...index sites use [j]...; store uses i; }),
 * with `i` recomputed each iteration as a fresh local (j >> 1), instead of
 * keeping `i` as the loop control variable and indexing with `i * 2`.
 *
 * Measured: score REGRESSED 58 -> 86 (build_insns 198 -> 202) on the
 * s7-banked chassis (immediately before the s11 r1/r2 merge was applied).
 * Reverted; re-confirmed floor 58 exactly reproduces after revert.
 *
 * Root cause (see candidate.c s11 header + tmp/grind/func_80056CB8/dumps/
 * text1b.loop lines 13251-14311): loop.c's giv detector only tracks
 * affine mult/add relations to a biv; `i = j >> 1` is a right-shift, so it
 * is NOT recognized as a giv of the new biv `j` -- `i` has to be
 * recomputed via a real `sra` every iteration instead of being a cheap
 * strength-reduced accumulator, which is strictly worse than the original
 * `i`-as-biv shape (which lets both `i*2` byte-table indices combine into
 * one giv, per "giv at 42 combined with giv at 140" in the dump -- they
 * just aren't judged worth strength-reducing into an accumulator either,
 * for the SAME insn_count/threshold-vs-benefit reason regardless of which
 * variable is nominally the biv).
 *
 * Distinct from s10's killed idx2 forms (loop-carried-idx2-worse.c,
 * shared-idx-local-worse.c): those kept `i` as the loop CONTROL variable
 * and added a second loop-carried value of the same biv class; this form
 * changes which variable IS the loop's primary biv. Both directions are
 * now killed.
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
    s32 j;

    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    for (j = start * 2; j < start * 2 + 4; j += 2) {
        s32 i = j >> 1;
        s32 obj;
        s32 flags;
        s32 scale;
        s16 *sin_p;
        s16 *cos_p;
        s32 x;
        s32 z;
        s32 r1;
        s32 r2;

        obj = arg0;
        flags = (&D_8009A821)[j] << 8;
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
        cos_p = &Judge + ((flags + 0x400) & 0xFFF);
        scale = (&D_8009A820)[j] << 8;
        x = *(s32 *)(obj + 0xB8) + ((scale * *sin_p) >> 12);
        z = *(s32 *)(obj + 0xC0) + ((scale * *cos_p) >> 12);
        pt0[0] = *(s32 *)(obj + 0xB8);
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt0[2] = *(s32 *)(obj + 0xC0);
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) - 0x320;
        pt1[2] = z;

        r1 = func_80053614(pt0, pt1, (s32)hit0, (s32)work, 0x1F8002B8);
        if (r1 != 0) {
            x += (*sin_p * 0x7D) >> 8;
            z += (*cos_p * 0x7D) >> 8;
        }

        pt0[0] = x;
        pt0[1] = *(s32 *)(obj + 0xBC) - 0x834;
        pt0[2] = z;
        pt1[0] = x;
        pt1[1] = *(s32 *)(obj + 0xBC) + 0x1004;
        pt1[2] = z;

        r2 = func_80053614(pt0, pt1, (s32)hit1, (s32)work, 0x1F8002B8);

        flags = (r1 | (r2 << 1)) + 1;
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
