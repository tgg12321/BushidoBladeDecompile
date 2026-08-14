/* candidate.c - func_8003504C - SESSION 7 (forensics) replacement candidate.
 *
 * `sandbox func_8003504C --disable all` = **4**, 141/141 instructions, and
 * only **FOUR** differing normalized positions (the session-4 candidate this
 * replaces scored the same 4 but had SEVEN).  The entire residual is one
 * rotation inside the pre-loop block:
 *
 *     target  ... li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 ...
 *     ours    ... move t3,v0 / move a2,t3 / li t2,5 / li t1,20 / lui t0 ...
 *
 * Every other instruction in the function - including the loop-1 register
 * assignment (p-walker $a2, counter i $a3, D_8010277C walker $a1, guard base
 * $t0), the `addiu a1,t0,-9` derivation and both bitfield-extraction clusters
 * - matches target byte for byte and register for register.
 *
 * How it is built (session 6's h1 chassis crossed with session 4's lever):
 *   1. `b` / `w` are assigned INSIDE loop 1's body.  As in-loop loop
 *      invariants loop.c hoists them as MOVABLES, which is what puts target's
 *      `lui t0 / addiu t0` in the preheader with $t0 live across the loop
 *      (guard reads `lb v0,0(t0)`), and makes the D_8010277C walker giv's
 *      initial value `(plus (reg b) (const_int -9))` = target's third
 *      preheader insn `addiu a1,t0,-9`.   [session 6, gaps (a) and (b)]
 *   2. The loop-1 p-walker is a SOURCE pointer `s` (not an i-indexed
 *      expression strength-reduced into a giv), and `s` - dead after loop 1 -
 *      carries loop 2's D_801027D8 destination.  Session 7 measured why that
 *      is load-bearing: with the reuse, cc1's own allocno table (BB2_ALLOC_DEBUG)
 *      gives s nrefs=11 livelen=31 pri=10645 against the counter's nrefs=11
 *      livelen=33 pri=10000, so s is allocated first and takes $a2 while i
 *      inherits $a3 - target's assignment.  Without the reuse (v_s1) s is
 *      nrefs=9 livelen=31 pri=8709 and the assignment inverts (score 12).
 *
 * Why the remaining rotation does not fall to a source spelling ON THIS
 * chassis: the two constants are loop.c MOVABLES, and move_movables inserts
 * them immediately before NOTE_INSN_LOOP_BEG, i.e. after EVERY pre-loop source
 * statement.  `p = call` and `s = (u8 *)p` are pre-loop source statements, so
 * their LUIDs are always lower, and sched1 (all these insns are latency-1, so
 * rank_for_schedule falls through to INSN_LUID among the ready insns) keeps
 * them ahead.  Only a walker emitted by loop.c AT loop_start (a giv init) sits
 * after the movables - which is the h1 chassis, where the allocation inverts
 * because a giv cannot pick up the two post-loop refs this reuse gives `s`.
 * See memory/grind/func_8003504C/chassis_h1_score9.c and the session-7
 * hypotheses for the exact arithmetic.
 *
 * NO cheat construct: no pins, no inline asm, no dead stores, no volatile
 * coercion, no unused locals.  Every local is assigned and then read; `s` is
 * ordinary variable reuse of a local that is dead after loop 1.
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
