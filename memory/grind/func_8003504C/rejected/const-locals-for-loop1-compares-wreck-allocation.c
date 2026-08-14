/* REJECTED (session 7) - const-locals-for-loop1-compares-wreck-allocation.c
 *
 * Holding loop 1's two compare constants (5, 20) in PRE-LOOP LOCALS so their
 * `li`s would get low LUIDs and emit ahead of the two p-copies.  Measured on
 * the session-7 candidate chassis (score 4): score 9 in BOTH placements
 * (constants initialised after the call, v_p2a, and before it, v_p2b).  The
 * locals become their own allocnos live across loop 1, which re-shuffles the
 * whole loop-1 allocation and loses the p-walker/counter assignment the
 * chassis had.  The rotation is untouched.  KILLED - and note this shape is
 * a constant-holder spelling, so it should not be re-proposed on codegen
 * grounds either.
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    s32 c5;
    s32 c20;
    u8 *s;
    s32 *q;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;
    s = (u8 *)p;
    c5 = 5;
    c20 = 20;

    do {
        s8 *b = &D_80102785;
        u8 *w = (u8 *)b - 9;
        s32 lv = (&D_8008D55C)[s[0]];
        w[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == c5 || (u32)(lv - 18) < 2 || (s8)lv == c20) {
            if (*b == 0) {
                w[i] = w[i] - 3;
            }
        }
        tmp = s[1];
        s += 10;
        (&D_80102780)[i] = 0;
        (&D_8010277E)[i] = tmp;
        i++;
    } while (i < 2);

    D_80102784 = ((u32)p[5] >> 4) & 0x3F;
    q = &p[8];
    D_80102786 = ((u32)*q >> 3) & 1;
    D_800A36F6 = 0;
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
        s = &D_801027D8;
        D_800A38E1 = ((u32)p[5] >> 15) & 3;
        {
            s32 j = 0;
            u8 *dst_d = s;
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
