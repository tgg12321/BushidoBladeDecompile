/* cpu_check_run_attack — C body removed by Campaign 4 (INCLUDE_ASM conversion).
 * The build DISCARDED this body (asmfix replace_with_asmfile substituted
 * asm/funcs/cpu_check_run_attack.s verbatim), so it never reached the binary.
 * This body is a real prior decomp attempt — archaeology for the C SHAPE when this function is decompiled.
 * NOTE: its signature was never checked by anything — do not trust it.
 *
 * !! WARNING — FORBIDDEN CONSTRUCTS BELOW: 4 register-asm pin(s), 6 hardcoded-$N inline-asm block(s), 62 __asm__ occurrence(s) incl. 34 raw .word encoding(s).
 * These are NOT sanctioned and must NOT be carried back into src/.
 * See .claude/rules/register-asm-pins.md (diagnostic-only, never
 * committable), inline-asm-injection.md (hardcoded-$N) and
 * inline-move-aliasing.md (archived tombstone). This file is outside
 * every detector's scope, so nothing will flag them for you.
 * Resume from the C SHAPE only — control flow, field offsets, the
 * algorithm — and re-derive in PURE C.
 * Campaign plan R2.
 */
void cpu_check_run_attack(s32 *arg) {
    register s32 t4 asm("t4");
    register s32 count asm("s0");
    register s32 *out asm("s1");
    register s32 sp_addr asm("s2");
    s32 sp_buf[6];

    s32 *data;
    s32 *base;
    s32 pair, radius;
    s32 thresh, neg_thresh;
    s32 dx, dy, dz;
    s32 *nodeA, *nodeB;

    out = (s32 *)0x1F8000BC;
    data = *(s32 **)((u8 *)arg + 0x10);
    base = *(s32 **)((u8 *)arg + 0xC);
    count = *(s16 *)((u8 *)arg + 6) - 1;

    pair = data[1];
    radius = data[0];

    nodeA = (s32 *)((u8 *)base + ((pair >> 16) << 6));
    nodeB = (s32 *)((u8 *)base + ((pair & 0xFFFF) << 6));

    thresh = radius * 2 + radius;
    neg_thresh = -thresh;

    dx = nodeB[0] - nodeA[0];
    dy = nodeB[1] - nodeA[1];
    dz = nodeB[2] - nodeA[2];

    if (count <= 0) goto post_loop;
    goto range_check;

bisect:
    {
        s32 mid_x, mid_y, mid_z;

        mid_x = (nodeB[0] + nodeA[0]) / 2;
        mid_y = (nodeB[1] + nodeA[1]) / 2;
        mid_z = (nodeB[2] + nodeA[2]) / 2;

        if (nodeB[6] < 0) {
            s32 ext;
            nodeB[0] = mid_x;
            nodeB[1] = mid_y;
            nodeB[2] = mid_z;
            ext = nodeB[3];
            nodeB[3] = ext / 4;
            ext = nodeB[4];
            nodeB[4] = ext / 4;
            ext = nodeB[5];
            nodeB[5] = ext / 4;
        } else {
            s32 ext;
            nodeA[0] = mid_x;
            nodeA[1] = mid_y;
            nodeA[2] = mid_z;
            ext = nodeA[3];
            nodeA[3] = ext / 4;
            ext = nodeA[4];
            nodeA[4] = ext / 4;
            ext = nodeA[5];
            nodeA[5] = ext / 4;
        }

        dx = nodeB[0] - nodeA[0];
        dy = nodeB[1] - nodeA[1];
        dz = nodeB[2] - nodeA[2];
    }

range_check:
    if (dx < neg_thresh) goto bisect;
    if (thresh < dx) goto bisect;
    if (dy < neg_thresh) goto bisect;
    if (thresh < dy) goto bisect;
    if (dz < neg_thresh) goto bisect;
    if (thresh < dz) goto bisect;

    {
        s32 sum_sq, sqrt_val;
        s32 tmp;

        tmp = dx >> 3;
        __asm__ volatile("sw\t%0, 0x1F800000" :: "r"(tmp));
        tmp = dy >> 3;
        __asm__ volatile("sw\t%0, 0x1F800004" :: "r"(tmp));
        sp_addr = 0x1F800000;
        tmp = dz >> 3;
        __asm__ volatile("sw\t%0, 0x1F800008" :: "r"(tmp));

        __asm__ volatile("addu\t$12, %0, $0" : "=r"(sp_addr) : "0"(sp_addr));
        __asm__ volatile(".word 0xC9890000");
        __asm__ volatile(".word 0xC98A0004");
        __asm__ volatile(".word 0xC98B0008");
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile(".word 0x4AA00428");

        data = (s32 *)((u8 *)data + 0x10);
        count--;

        sp_addr = 0x1F80000C;
        pair = data[1];
        __asm__ volatile("addu\t$12, %0, $0" : "=r"(sp_addr) : "0"(sp_addr));
        __asm__ volatile(".word 0xE9990000");
        __asm__ volatile(".word 0xE99A0004");
        __asm__ volatile(".word 0xE99B0008");

        {
            s32 rd0, rd1, rd2;

            __asm__ volatile("lw\t%0, 0x1F80000C" : "=r"(rd0));
            __asm__ volatile("lw\t%0, 0x1F800010" : "=r"(rd1));
            __asm__ volatile("lw\t%0, 0x1F800014" : "=r"(rd2));
            sum_sq = rd0 + rd1 + rd2;
        }

        if ((u32)sum_sq < 0x400) {
            sqrt_val = *(&D_8008D118 + sum_sq);
            nodeA = (s32 *)((u8 *)base + ((pair >> 16) << 6));
            nodeB = (s32 *)((u8 *)base + ((pair & 0xFFFF) << 6));
        } else {
            s32 shift_a, shift_b;

            t4 = sum_sq;
            __asm__ volatile(".word 0x488CF000" :: "r"(t4));

            nodeA = (s32 *)((u8 *)base + ((pair >> 16) << 6));
            nodeB = (s32 *)((u8 *)base + ((pair & 0xFFFF) << 6));

            t4 = (s32)&sp_buf[0];
            __asm__ volatile(".word 0xE99F0000" :: "r"(t4));

            {
                s32 lzcr = sp_buf[0];
                s32 mask = -2;

                mask = lzcr & mask;
                shift_a = 0x16 - mask;
            }
            shift_b = shift_a >> 1;
            sqrt_val = ((u32)(*(&D_8008D118 + ((u32)sum_sq >> shift_a))) << 16) >> (0x10 - shift_b);
        }

        sp_addr = 0x1F800000;
        __asm__ volatile("addu\t$12, %0, $0" : "=r"(sp_addr) : "0"(sp_addr));
        __asm__ volatile(".word 0xC9890000");
        __asm__ volatile(".word 0xC98A0004");
        __asm__ volatile(".word 0xC98B0008");

        {
            t4 = (sqrt_val - radius) << 14;
            radius = data[0];
            {
                t4 = t4 / sqrt_val;
                thresh = radius * 2 + radius;
                neg_thresh = -thresh;

                __asm__ volatile(".word 0x488C4000" :: "r"(t4));
                __asm__ volatile("nop");
                __asm__ volatile("nop");
                __asm__ volatile(".word 0x4B98003D");
            }
        }

        t4 = (s32)out;
        dx = nodeB[0] - nodeA[0];
        dy = nodeB[1] - nodeA[1];
        dz = nodeB[2] - nodeA[2];

        __asm__ volatile(".word 0xE9890000" :: "r"(t4));
        __asm__ volatile(".word 0xE98A0004");
        __asm__ volatile(".word 0xE98B0008");
        out = (s32 *)((u8 *)out + 0xC);
    }

    if (count > 0) goto range_check;

post_loop:
    {
        s32 sum_sq, sqrt_val;
        s32 tmp;

        tmp = dx >> 3;
        __asm__ volatile("sw\t%0, 0x1F800000" :: "r"(tmp));
        tmp = dy >> 3;
        __asm__ volatile("sw\t%0, 0x1F800004" :: "r"(tmp));
        sp_addr = 0x1F800000;
        tmp = dz >> 3;
        __asm__ volatile("sw\t%0, 0x1F800008" :: "r"(tmp));

        __asm__ volatile("addu\t$12, %0, $0" : "=r"(sp_addr) : "0"(sp_addr));
        __asm__ volatile(".word 0xC9890000");
        __asm__ volatile(".word 0xC98A0004");
        __asm__ volatile(".word 0xC98B0008");
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        __asm__ volatile(".word 0x4AA00428");

        sp_addr = 0x1F80000C;
        __asm__ volatile("addu\t$12, %0, $0" : "=r"(sp_addr) : "0"(sp_addr));
        __asm__ volatile(".word 0xE9990000");
        __asm__ volatile(".word 0xE99A0004");
        __asm__ volatile(".word 0xE99B0008");

        {
            s32 rd0, rd1, rd2;

            __asm__ volatile("lw\t%0, 0x1F80000C" : "=r"(rd0));
            __asm__ volatile("lw\t%0, 0x1F800010" : "=r"(rd1));
            __asm__ volatile("lw\t%0, 0x1F800014" : "=r"(rd2));
            sum_sq = rd0 + rd1 + rd2;
        }

        if ((u32)sum_sq < 0x400) {
            sqrt_val = *(&D_8008D118 + sum_sq);
        } else {
            s32 shift_a, shift_b;

            t4 = sum_sq;
            __asm__ volatile(".word 0x488CF000" :: "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");

            t4 = (s32)&sp_buf[0];
            __asm__ volatile(".word 0xE99F0000" :: "r"(t4));

            {
                s32 lzcr = sp_buf[0];
                s32 mask = -2;

                mask = lzcr & mask;
                shift_a = 0x16 - mask;
            }
            shift_b = shift_a >> 1;
            sqrt_val = ((u32)(*(&D_8008D118 + ((u32)sum_sq >> shift_a))) << 16) >> (0x10 - shift_b);
        }

        sp_addr = 0x1F800000;
        __asm__ volatile("addu\t$12, %0, $0" : "=r"(sp_addr) : "0"(sp_addr));
        __asm__ volatile(".word 0xC9890000");
        __asm__ volatile(".word 0xC98A0004");
        __asm__ volatile(".word 0xC98B0008");

        {
            t4 = (sqrt_val - radius) << 14;
            t4 = t4 / sqrt_val;

            __asm__ volatile(".word 0x488C4000" :: "r"(t4));
            __asm__ volatile("nop");
            __asm__ volatile("nop");
            __asm__ volatile(".word 0x4B98003D");
        }

        t4 = (s32)out;
        __asm__ volatile(".word 0xE9890000" :: "r"(t4));
        __asm__ volatile(".word 0xE98A0004");
        __asm__ volatile(".word 0xE98B0008");
    }
}
