/* REJECTED (session 5) - giv-walker-based-on-D80102785-cse-folds-the-minus9
 *
 * Same giv chassis, but the D_8010277C walker written as (&D_80102785)[i - 9]
 * so cse would materialize %hi/%lo(D_80102785) once and derive the walker as
 * -9 (target's `addiu a1,t0,-9`). cse instead FOLDS the -9 into the %lo of the
 * giv's own initial address, emitting `lui a1 / addiu a1,a1,-9` = 2 insns with
 * no separate base register, and the in-loop `D_80102785 == 0` guard gets its
 * own lui. Score 18, identical shape to the plain giv chassis.
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
        (&D_80102785)[i - 9] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
            if (D_80102785 == 0) {
                (&D_80102785)[i - 9] = (&D_80102785)[i - 9] - 3;
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
