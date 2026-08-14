/* REJECTED (session 5) - giv-both-walkers-preloop-order-matches-score18
 *
 * Both loop-1 walkers spelled i-indexed so loop.c strength-reduction creates
 * them as givs. THE PRE-LOOP BLOCK ORDER BECOMES TARGET'S EXACTLY
 * (i=0 / li 5 / li 20 / move t3,v0 / move <src>,t3 / lui / addiu) - the first
 * form in five sessions to do so - but the whole form scores 18, not 4,
 * because (a) cluster 1's i/src register inversion returns (there is no `src`
 * local left to carry session 4's live-range-extension lever) and (b) the
 * D_8010277C walker's giv init is 2 insns (lui a1 / addiu a1) instead of
 * target's 3 (lui t0 / addiu t0 / addiu a1,t0,-9), so everything from
 * position 11 on is shifted by one. Dead AS A CLOSING FORM; kept because it
 * is the positive proof of the hoist/giv-init ordering mechanism.
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    s32 *q;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;

    do {
        s32 lv = (&D_8008D55C)[((u8 *)p)[i * 10]];
        (&D_8010277C)[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
            if (D_80102785 == 0) {
                (&D_8010277C)[i] = (&D_8010277C)[i] - 3;
            }
        }
        tmp = ((u8 *)p)[i * 10 + 1];
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
