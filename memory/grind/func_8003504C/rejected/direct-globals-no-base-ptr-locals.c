/* REJECTED (session 3) — drop the `base` / `ptr` locals, spell the
 * D_8010277C-area accesses as direct i-indexed globals inside loop 1.
 *
 * Score 30 (vs 24), 141 insns, 44 differing positions. The pre-loop block loses
 * target's `addiu $a1,$t0,-9` derivation (it becomes an independent lui/addiu
 * pair), which is the same evidence session 2 got from the giv reconstruction:
 * the original DID have the `base` / `ptr = base - 9` source relationship.
 *
 * KEPT FOR ITS MECHANISM VALUE: this form DID move the two LICM-hoisted
 * constants (`li 5` / `li 20`) ahead of the address setup in the pre-loop
 * block — positions 7,8 with the lui/addiu at 9,10 — confirming that the
 * constants-vs-address order in that block is decided by loop.c's hoist order
 * (an address computed inside the loop is hoisted AFTER the constants), not by
 * source statement order. Target additionally has both `move $t3,$v0` and
 * `move $a2,$t3` AFTER the constants, which this form does not reproduce.
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    u8 *src;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;
    src = (u8 *)p;

    do {
        s32 lv = (&D_8008D55C)[*src];
        (&D_8010277C)[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
            if (D_80102785 == 0) {
                (&D_8010277C)[i] = (&D_8010277C)[i] - 3;
            }
        }
        tmp = src[1];
        src += 10;
        (&D_80102780)[i] = 0;
        (&D_8010277E)[i] = tmp;
        i++;
    } while (i < 2);

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
