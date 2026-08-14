/* REJECTED (session 7) - h1-chassis-source-pointer-walker-without-the-reuse.c
 *
 * The session-7 candidate chassis MINUS session 4's reuse lever: the loop-1
 * p-walker is a source pointer but nothing extends its live range past loop
 * 1.  Score 12 (against 4 with the reuse).  cc1's allocno table shows why:
 * the walker is nrefs=9 livelen=31 pri=8709 versus the counter's nrefs=11
 * livelen=33 pri=10000, so the counter is allocated first and takes $a2.
 * The reuse is what turns 9 refs into 11 for only +2 of live length.
 * KILLED - kept as the isolation measurement for the lever.
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    u8 *s;
    s32 *q;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;
    s = (u8 *)p;

    do {
        s8 *b = &D_80102785;
        u8 *w = (u8 *)b - 9;
        s32 lv = (&D_8008D55C)[s[0]];
        w[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
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
