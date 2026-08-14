/* vE — loop 1 written with pure i-indexing (no source-level walking pointers):
 * the two walking registers in target ($a2 over p, $a1 over D_8010277C) are
 * reconstructed as loop.c STRENGTH-REDUCTION givs, which is what explains the
 * asymmetry in target (the 3-use D_8010277C array and the 2-use p array walk,
 * while the 1-use D_80102780 / D_8010277E arrays stay lui+addu-indexed by i).
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    s8 *base;
    s8 val;

    p = func_80077D00();
    base = &D_80102785;

    for (i = 0; i < 2; i++) {
        s32 lv = (&D_8008D55C)[((u8 *)p)[i * 10]];
        (&D_8010277C)[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
            if (*base == 0) {
                (&D_8010277C)[i] = (&D_8010277C)[i] - 3;
            }
        }
        (&D_80102780)[i] = 0;
        (&D_8010277E)[i] = ((u8 *)p)[i * 10 + 1];
    }

    D_80102784 = ((u32)p[5] >> 4) & 0x3F;
    D_800A36F6 = 0;
    D_80102786 = ((u32)p[8] >> 3) & 1;

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
