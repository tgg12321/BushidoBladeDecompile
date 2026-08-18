/* CANDIDATE - func_80017848, s7 (2026-08-18, forensics).  sandbox --disable all = 11
 * 125/127 build insns, frame 0x40 both sides.  FLOOR 14 -> 11 (variant O).
 *
 * WHAT CHANGED FROM THE S6 CANDIDATE (B, 14).  Two independent structural facts
 * were combined; each one alone is worth nothing, together they are worth 3:
 *
 *  (1) THE LOOP-1 EXIT-TAIL RELOAD (new this session).  `p` is read fresh ONCE
 *      before loop 1's entry guard and re-read ONLY in loop 1's EXIT TAIL (after
 *      the do/while, inside the if).  Loop 2's entry guard then reads its count
 *      through the SAME `p`, so on loop 1's skip path it consumes the pre-guard
 *      value and on the loop-ran path it consumes the reload.  GCC turns that
 *      into exactly target's control flow: the `blez` at loop 1's guard branches
 *      PAST the reload straight into the middle of loop 2's guard block
 *      (`blez v0,1444` where `1444` is the `sll`, one insn after the
 *      `lw a0,12(s2)` at `1440`).  This is target's `blez v0,.L8001791C` shape.
 *  (2) THE BASE'S ADDEND IS THE PRE-JOIN `slots` READ, NOT `p` (the s6 E-s6-3
 *      fold-defeat lever, which s6 could only measure on the OLD chassis where it
 *      LOST an instruction).  If the base is built from `p` as well, both the
 *      guard address and the base are `(plus shift p)` inside the SAME cse
 *      extended basic block, cse.c rewrites the base to a copy of the guard
 *      address, and the preheader is 1 insn short in loop 1 and 2 short in loop 2
 *      (124 insns, score 14 - variants J/M/N below).  Sourcing the addend from
 *      `slots`, which was live before the join, makes the two addends
 *      un-equatable and both `addu`s survive (s7/O/ings_pp.c.combine: insn 146
 *      `reg111 = reg110 + reg/v78` for the guard, insn 158
 *      `reg/v79 = reg110 + reg/v77` for the base - two independent addsi3s).
 *
 * S7 MEASUREMENTS (all from a clean tree via s7/score.sh; s7/scores.txt):
 *   O   = THIS FORM: guards through fresh `p` + exit-tail reload, base through
 *         `slots`                                                        -> 11
 *   R   = O but the base is written inside the loop body (LICM hoists it)  -> 11
 *   Y2  = O + an explicit per-loop `links` local read after the base       -> 11
 *   B   = s6 candidate (guards through `slots`, base from a fresh read)    -> 14
 *   J   = guards AND base both through `p` (+ exit-tail reload)            -> 14
 *   M   = J with the base written inside the loop body                     -> 14
 *   N   = J with no base variable at all (fully inlined)                   -> 14
 *   S   = J but only loop 1's base through `slots`                         -> 14
 *   W   = O but only loop 2's base through `p`                             -> 14
 *   Y1  = O + the `links` local read BEFORE the base                       -> 15
 *   V   = guards through `slots`, bases through `p`                        -> 16
 *   K   = J + an explicit `shift` local recomputed in the exit tail        -> 16
 *   X   = O + that same explicit `shift` local                             -> 17
 *
 * THE ENTIRE REMAINING RESIDUAL IS 2 INSTRUCTIONS, AND THEY ARE THE SAME
 * INSTRUCTION TWICE (s7/T.txt vs s7/B.txt - the ONLY non-register hunks left):
 *      target preheader:  addu a3,a0,zero / lw a2,16(s2) / addu a0,a1,a3
 *      ours:              addu a0,a1,a2   / lw a1,16(s2)
 * i.e. target routes the base's addend through a DEAD register copy of the
 * pre-guard pointer, once per scan loop.  Everything else in the function is
 * instruction-identical; the only other diff is the register naming that falls
 * out of keeping `slots` live (ours: slots=a2, links=a1; target: slots=v1 and
 * dead after the two >=0 guards, links=a2).
 *
 * DO NOT re-propose a C-level copy statement for that `addu a3,a0,zero` - s6
 * killed it at pass level (cse.c folds it when the expressions are equal,
 * combine.c's try_combine propagates and deletes it when they are not; s6/G's
 * dump shows the copy already `NOTE_INSN_DELETED` before local-alloc).  In
 * target the copy is dead after its single use in BOTH preheaders, so it is a
 * pass artifact of a source shape not yet found, not a written copy.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    p = *(u8 **)(ctx + 0xC);
    i = 0;
    if (i < *(s32 *)((slot_a << 6) + (s32)p + 0x1C)) {
        base = (u8 *)((slot_a << 6) + (s32)slots);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
        p = *(u8 **)(ctx + 0xC);
    }

    i = 0;
    if (i < *(s32 *)((slot_a << 6) + (s32)p + 0x20)) {
        base = (u8 *)((slot_a << 6) + (s32)slots);
        do {
            if (*(s16 *)((*(u8 *)(base + i + 0x2C) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x20));
    }

    dist = math_Distance3D((s32 *)(*(u8 **)(ctx + 0xC) + (slot_a << 6)),
                           (s32 *)(*(u8 **)(ctx + 0xC) + (slot_b << 6)));
    link = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 0x6) << 4);
    *(s32 *)(link + 0x0) = dist;
    *(s32 *)(link + 0x8) = dist * 3;
    *(s32 *)(link + 0xC) = arg1;
    *(s32 *)(link + 0x4) = (slot_a << 16) | slot_b;

    rec_a = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_a + 0x1C);
    *(s32 *)(rec_a + 0x1C) = i + 1;
    *(u8 *)(rec_a + i + 0x24) = *(u16 *)(ctx + 0x6);

    rec_b = (u8 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_b + 0x20);
    *(s32 *)(rec_b + 0x20) = i + 1;
    *(u8 *)(rec_b + i + 0x2C) = *(u16 *)(ctx + 0x6);

    *(s16 *)(ctx + 0x6) = *(u16 *)(ctx + 0x6) + 1;
    return 1;
}
