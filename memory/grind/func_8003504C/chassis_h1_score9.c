/* chassis_h1_score9.c - func_8003504C - SESSION 6 (forensics) NEW CHASSIS.
 *
 * `sandbox func_8003504C --disable all` = **9**, 141/141 instructions,
 * **9 differing normalized positions - and all nine are ONE register swap**:
 * target has i in $a3 and the loop-1 p-walker in $a2; we have the inverse.
 * Every other instruction in the function, including the whole pre-loop block
 * (target's `move a3,zero / li t2,5 / li t1,20 / move t3,v0 / move a2,t3 /
 * lui t0 / addiu t0 / addiu a1,t0,-9`) and both bitfield-extraction clusters,
 * matches target byte for byte and register for register.
 *
 * The engine floor is still 4 (the session-4 pointer chassis, candidate.c)
 * because the sandbox score weights this 9-position register swap above that
 * chassis's 7-position block rotation.  This form is nonetheless the better
 * chassis: session 4 PROVED the i/walker allocation flip is reachable by a
 * live-range extension, whereas session 5 proved the floor-4 chassis's
 * pre-loop block can never be rotated (its `src`/`base`/`ptr` pre-loop
 * statements always emit ahead of loop.c's movables).
 *
 * How it is built, from the session-5 giv chassis (v_idx_idx_plain, score 18):
 *   1. BOTH loop-1 walkers are i-indexed, so loop.c strength-reduces them into
 *      givs whose initial values are emitted at loop_start AFTER the movables
 *      - which is what puts `li 5` / `li 20` ahead of the p-copies (session 5).
 *   2. `s8 *b = &D_80102785;` is assigned INSIDE loop 1's body.  As an in-loop
 *      loop-invariant it becomes a loop.c MOVABLE, so its `lui t0 / addiu t0`
 *      is emitted in the preheader (not as a pre-loop source statement, which
 *      is what used to push it ahead of the constants), and $t0 stays live so
 *      the guard reads `lb v0,0(t0)` exactly like target.  Score 18 -> 13.
 *   3. `u8 *w = (u8 *)b - 9;` - also in-loop invariant - gives the D_8010277C
 *      walker giv the initial value `(plus (reg b) (const_int -9))`, so loop.c
 *      emits target's third insn `addiu a1,t0,-9` and every in-loop access is
 *      at offset 0 instead of carrying -9 in the memory offsets.  13 -> 9.
 *
 * What remains: on this chassis `.greg`'s allocno order is
 * `79 92 162 161 160 159 180 73 156 176 124 158 157 72 139 77 99 94`, i.e. the
 * source pseudo 73 (`i`) is ranked ahead of every loop.c-created giv pseudo,
 * so `i` takes $a2 as the first free register in REG_ALLOC_ORDER and the
 * p-walker giv inherits $a3.  Dispositions: 72 (p) = $t3, 73 (i) = $a2,
 * 77 (b) = $t0.  To match, a giv must outrank 73.
 *
 * NO cheat construct: no pins, no inline asm, no dead stores, no volatile
 * coercion, no unused locals.  `b` and `w` are ordinary in-loop pointer
 * locals, each assigned once and read on the following lines.
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
        s8 *b = &D_80102785;
        u8 *w = (u8 *)b - 9;
        s32 lv = (&D_8008D55C)[((u8 *)p)[i * 10]];
        w[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == 5 || (u32)(lv - 18) < 2 || (s8)lv == 20) {
            if (*b == 0) {
                w[i] = w[i] - 3;
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
