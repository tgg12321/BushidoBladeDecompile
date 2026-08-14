/* REJECTED (session 3) — cluster-2 statement order, ANY permutation.
 *
 * This file is the C,B,A,D representative of an EXHAUSTIVE sweep of all 24
 * orderings of the four mutually independent statements that make up cluster 2:
 *   A  D_80102784 = ((u32)p[5] >> 4) & 0x3F;
 *   B  D_800A36F6 = 0;
 *   C  D_80102786 = ((u32)p[8] >> 3) & 1;
 *   D  val = D_80102785;
 * (they are independent: D_80102785 is written by nothing in the function, and
 * the three destinations are distinct objects, so all 24 orders are the same
 * program).
 *
 * MEASURED RESULT — the final asm takes exactly TWO shapes and neither is
 * closer than HEAD:
 *   * the 6 orders with D last (ABCD ACBD BACD BCAD CABD CBAD): 141 insns,
 *     score 24, and the normalized position diff is BYTE-IDENTICAL to HEAD's,
 *     cluster 2 included. Permuting A/B/C is completely codegen-neutral.
 *   * every order with D not last: 143 insns, score 24 or 27 — moving the
 *     `val` read away from its first use costs 2 insns (cc1 keeps val as a
 *     QImode pseudo and emits lbu + sll 24 + sra 24 instead of combine folding
 *     the read into a single `lb`/extendqisi2_insn).
 *
 * So cluster 2 does NOT close on statement order: sched1's schedule of that
 * block is invariant under source order, and the ordering that survives into
 * the asm is set downstream (lreg's $v0-vs-$v1 choice, then sched2), not by the
 * order the four statements are written in. This retires session 2's top
 * frontier item.
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    u8 *src;
    u8 *ptr;
    s8 *base;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;
    src = (u8 *)p;
    base = &D_80102785;
    ptr = (u8 *)(base - 9);

    do {
        s32 lv = (&D_8008D55C)[*src];
        *ptr = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
            if (*base == 0) {
                *ptr = *ptr - 3;
            }
        }
        tmp = src[1];
        src += 10;
        (&D_80102780)[i] = 0;
        (&D_8010277E)[i] = tmp;
        i++;
        ptr++;
    } while (i < 2);

    D_80102786 = ((u32)p[8] >> 3) & 1;
    D_800A36F6 = 0;
    D_80102784 = ((u32)p[5] >> 4) & 0x3F;
    val = D_80102785;
    if (val == 2) {
        D_800A389A = ((u32)p[5] >> 17) & 1;
        D_800A3788 = ((u32)p[5] >> 18) & 7;
    } else if (val == 5) {
        u32 idx;
        s32 sel;

        D_800A389B = (((u32)p[5] >> 10) & 3) + 3;
        idx = ((u32)p[5] >> 12) & 3;
        D_800A36CC = (&D_8008EC30)[idx];
        sel = 1;
        if ((u32)p[5] & 0x4000) {
            sel = 2;
        }
        D_800A37F8 = sel;
        D_800A38E1 = ((u32)p[5] >> 15) & 3;
        {
            s32 j = 0;
            u8 *dst_d = &D_801027D8;
            u8 *dst_a = &D_801027A0;
            do {
                s32 k = 0;
                u8 *da = dst_d;
                u8 *db = dst_a;
                s32 off = j << 1;
            loop_inner:
                {
                    u8 *pp = (u8 *)p + off;
                    *db = (&D_8008D55C)[pp[0]];
                    off += 10;
                    k++;
                    *da = pp[1];
                    db++;
                    da++;
                }
                if (k < 2) goto loop_inner;
                dst_d += 2;
                j++;
                dst_a += 2;
            } while (j < 5);
        }
    }

    func_800344B4();
}
