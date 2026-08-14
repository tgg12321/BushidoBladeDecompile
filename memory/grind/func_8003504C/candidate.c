/* candidate.c — func_8003504C — BEST FORM as of session 4 (permuter).
 *
 * Honest pure-C floor: **4** (`sandbox func_8003504C --disable all`),
 * 141/141 instructions, **7 differing normalized positions**, down from the 24
 * / 26 positions that stood unchanged through sessions 1-3. This is the form
 * currently spliced into src/code6cac_b.c.
 *
 * THREE edits vs the session-3 form, all found by decomp-permuter random
 * search and then re-measured with the engine sandbox:
 *
 *   1. The `p[8]` read is staged through a named POINTER intermediate
 *      (`q = &p[8]; D_80102786 = ((u32)*q >> 3) & 1;`) rather than spelled
 *      inline as `((u32)p[8] >> 3) & 1`.
 *   2. The post-loop statements are ordered `D_80102784` store -> the p[8]
 *      extraction -> `D_800A36F6 = 0` -> `val = D_80102785;`, i.e. the
 *      D_800A36F6 zero-store now sits BETWEEN the two bitfield extractions.
 *   3. `src` — dead after loop 1 — is REUSED to carry the D_801027D8
 *      destination pointer for loop 2 (`src = &D_801027D8;` then
 *      `u8 *dst_d = src;`).
 *
 * Edits 1+2 close cluster 2 completely (24 -> 13): taking the ADDRESS of the
 * eighth word keeps a live pointer pseudo across the first extraction chain,
 * so sched1 can no longer hoist the p[8] load above the D_80102784 store; the
 * chains stop interleaving, their live ranges stop overlapping, and lreg
 * reuses $v0 for the second chain instead of allocating $v1. Only the POINTER
 * spelling does this — `s32 v8 = p[8];` and `u32 v8 = p[8];` are folded
 * straight back into the shift (measured, score 24), and reusing `i` as the
 * value temp scores 20. Neither edit works alone: with the inline p[8]
 * spelling all 24 statement orders were measured dead in session 3, and with
 * the pointer temp at session 3's order the score is 17.
 *
 * Edit 3 closes cluster 1's REGISTER INVERSION (13 -> 4): extending `src`'s
 * live range into the `val == 5` branch flips the global.c allocno order so
 * that `src` is allocated before `i`, landing p=$t3, ptr=$a1, src=$a2,
 * i=$a3, base=$t0 — target's exact assignment. This DISPROVES session 2's
 * arithmetic closure of the axis (see hypotheses.md [s4]).
 *
 * WHAT REMAINS — 7 positions, and they are a pure ROTATION of one 7-insn
 * pre-loop block; every instruction is present with the right register:
 *   target: li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 /
 *           addiu t0 / addiu a1,t0,-9
 *   ours:   move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9 /
 *           li t2,5 / li t1,20
 * i.e. the two LICM-hoisted loop-1 comparison constants must move from LAST
 * to FIRST in the pre-loop block.
 *
 * NO cheat construct of any kind is present: no pins, no inline asm, no dead
 * stores, no volatile coercion, no unused locals. Every local is live and
 * read. `q` is a plain pointer intermediate in a function that already walks
 * five other pointer locals (src, ptr, base, pp, da/db), and the `src` reuse
 * assigns a value that is then read on the next line.
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    u8 *src;
    u8 *ptr;
    s8 *base;
    s32 *q;
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
        src = &D_801027D8;
        D_800A38E1 = ((u32)p[5] >> 15) & 3;
        {
            s32 j = 0;
            u8 *dst_d = src;
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
