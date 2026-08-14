/* candidate.c - func_8003504C - SESSION 10 (synthesis).  HONEST DISTANCE 0.
 *
 * `sandbox func_8003504C --disable all` = **0**, 141/141 instructions, ZERO
 * differing normalized positions, measured this session with this exact body
 * spliced into src/code6cac_b.c.  It replaces the session-7/9 candidate, which
 * scored 4 (the pre-loop rotation) and is otherwise IDENTICAL to this form.
 *
 * WHAT CHANGED FROM THE SESSION-9 CANDIDATE (the whole delta, 4 -> 0):
 *   the two loop-1 compare constants 5 and 20 are held in pre-loop locals that
 *   are assigned BEFORE the walker copy `s = (u8 *)p;`.
 *
 * WHY THAT IS THE WHOLE RESIDUAL (session 8's sched1 model, unchanged):
 * sched1 fills a block BACKWARDS, repeatedly taking the insn with the greatest
 * (INSN_PRIORITY, INSN_LUID) among those whose in-block successors are already
 * placed.  Target's preheader is
 *     move a3,zero / li t2,5 / li t1,20 / move t3,v0 / move a2,t3 /
 *     lui t0 / addiu t0 / addiu a1,t0,-9
 * so `s = p` (move a2,t3) has to be placed 5th.  There it is tied at priority 1
 * with the two `li`s, and priority ties go to the HIGHER LUID, so target's order
 * requires INSN_LUID(`s = p`) > INSN_LUID(`li 20`).  Written as literals the two
 * constants are loop.c MOVABLES, and move_movables inserts every movable
 * immediately before NOTE_INSN_LOOP_BEG - i.e. after EVERY pre-loop source
 * statement - so as literals they can only ever land BEHIND the two p-copies
 * (that is the session-4..9 chassis's 4-point residual).  Held in locals
 * assigned ahead of `s = p`, their `li`s are ordinary pre-loop SOURCE insns with
 * LUIDs below `s = p`'s, and the backward schedule then emits target's order
 * exactly: i=0, li 5, li 20, p=v0, s=p, b, w-giv-init.
 *
 * The insn ORDER of the initialisations is load-bearing and is what separates
 * this form from the banked rejection
 * rejected/preloop-constant-locals-are-cse-folded-and-loop-c-rehoists-them.c
 * (session 7's v_p2a, re-measured in session 9): that form declared the same two
 * constant locals but assigned them AFTER `s = (u8 *)p;`, giving them HIGHER
 * LUIDs, so the backward schedule still put them behind the p-copies and it
 * measured 4 with a diff byte-identical to the session-9 candidate.  Session 9's
 * recorded explanation for v_p2a ("cse folds them and loop.c re-hoists") is
 * therefore wrong; the real cause was the assignment order.  Measured this
 * session, on this chassis: constants after `s = p` -> 4; only 5 held, before
 * `s = p` -> 2; both held, before `s = p` -> 0.
 *
 * The two locals carry the mandatory FAKE annotation in src (see the
 * body below) under the constant-holder carve-out of
 * .claude/rules/named-local-fake-exception.md.  Everything else in this file is
 * the session-7 chassis unchanged:
 *   1. `b` / `w` assigned INSIDE loop 1's body: as in-loop invariants loop.c
 *      hoists them as MOVABLES, producing target's `lui t0 / addiu t0` with $t0
 *      live across the loop (the guard reads `lb v0,0(t0)`) and making the
 *      D_8010277C walker giv's initial value `(plus (reg b) (const_int -9))` =
 *      target's `addiu a1,t0,-9`.
 *   2. The loop-1 p-walker is a SOURCE pointer `s`, and `s` - dead after loop 1 -
 *      carries loop 2's D_801027D8 destination.  cc1's own allocno table gives
 *      s nrefs=11 livelen=31 pri=10645 against the counter's 11/33/10000, so s
 *      is allocated first and takes $a2 while i inherits $a3 = target's
 *      assignment.  (Without the reuse: 9/31/8709 and the assignment inverts.)
 *   3. `q = &p[8]` stages the second bitfield read through a pointer, and
 *      `D_800A36F6 = 0;` sits BETWEEN the two extractions - session 4's two
 *      cluster-2 levers.
 *
 * No pins, no inline asm, no dead stores, no volatile coercion, no unused
 * locals, no arrays: every local is assigned and then read.  See
 * memory/grind/func_8003504C/self_vet.md for the full six-test vet.
 */
void func_8003504C(void) {
    s32 *p;
    s32 i;
    u8 *s;
    /* FAKE: 5 and 20 held in locals so their `li`s are pre-loop SOURCE insns
       whose LUIDs are lower than the walker copy `s = p`; mechanism: sched.c
       rank_for_schedule's INSN_LUID tie-break inside sched1's backward list
       schedule (written as literals they are loop.c movables, and move_movables
       inserts every movable after ALL pre-loop statements, which emits them
       behind the two p-copies); lever-exhaustion: sessions 1-9 of
       memory/grind/func_8003504C/hypotheses.md. */
    s32 new_var;
    s32 new_var2;
    s32 *q;
    s8 val;
    u8 tmp;

    p = func_80077D00();
    i = 0;
    new_var = 5;
    new_var2 = 20;
    s = (u8 *)p;

    do {
        s8 *b = &D_80102785;
        u8 *w = (u8 *)b - 9;
        s32 lv = (&D_8008D55C)[s[0]];
        w[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == new_var || (u32)(lv - 18) < 2 || (s8)lv == new_var2) {
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
