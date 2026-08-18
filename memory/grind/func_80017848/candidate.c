/* CANDIDATE - func_80017848, s5 (2026-08-18).  sandbox --disable all = 12
 * 125/127 build insns, frame 0x40 both sides.
 *
 * THE BODY IS UNCHANGED FROM S4 AND THAT IS THE HEADLINE FINDING OF S5.
 * s5 appeared to reach 10 and it does not.  The 12 -> 10 drop was produced by
 * an UNUSED LOCAL DECLARATION (`u8 *q;`, never written and never read) that the
 * s5 sweep templates emitted in a fixed declaration block for every cell.  Once
 * the declaration block is pruned to exactly the variables a cell uses, the
 * apparent winner scores 13 - one WORSE than this form.  Measured three ways:
 *     s4 body, guard address POINTER-FIRST, no unused local   -> 12
 *     same body, guard address SHIFT-FIRST, no unused local   -> 13
 *     same body, SHIFT-FIRST, plus the unused `u8 *q;`        -> 10
 *     same body, POINTER-FIRST, plus the unused `u8 *q;`      -> 12
 * So the unused declaration is worth 3 instructions on the shift-first chassis
 * and 0 on the pointer-first one, and the guard operand order is worth nothing
 * on its own.  s3's banked "the loop entry guard's count address must be
 * written POINTER-FIRST" therefore STANDS.
 *
 * The unused declaration is NOT shipped here.  It has no semantic purpose
 * (checklist T1), no programmer would write it (T2), and its only appearance in
 * the function is the declaration itself (T6) - it is the dead-local family, and
 * first reach of an unsanctioned family is a cheat regardless of spelling.  It is
 * banked in rejected/unused_local_decl_q_contaminates_sweeps.c as a MEASUREMENT
 * ARTIFACT to be avoided, not as a lever to be spent.
 *
 * INHERITED LEVERS (do not re-derive - s2/s3/s4):
 *   (1) source-level do-while -> target's per-iteration bound reload + one-addu index;
 *   (2) entry guard spelled `i = 0; if (i < count)` -> the phantom-16 frame;
 *   (3) the loop entry guard's count address written POINTER-FIRST, and the
 *       `slots` read hoisted ABOVE the two >=0 top guards so those guards
 *       consume it too;
 *   (4) `base = slots;` right after the slots read, so the two SCAN-LOOP ENTRY
 *       guards read their count through `base` while the two >=0 top guards read
 *       through `slots`.  `base` is re-pointed at the per-slot record inside each
 *       preheader.  Ordinary variable reuse - `base` is live and read at every
 *       point.
 *
 * REMAINING RESIDUAL (2 insns short of 127, 12 differing).  s5 decomposed it
 * exactly, per scan loop:
 *     target: lw a0,0xC(s2) BEFORE the entry guard ; sll a1,s4,6 ;
 *             addu v0,a1,a0 ; lw v0,0x1C(v0) ; blez ; move a3,a0 ;
 *             lw a2,0x10(s2) ; addu a0,a1,a3
 *     ours:   sll a1,s4,6 ; addu v0,a0,a1 ; lw v0,0x1C(v0) ; blez ;
 *             lw v0,0xC(s2) AFTER the branch ; lw a2,0x10(s2) ; addu a0,a1,v0
 * i.e. target computes `ptr + (slot_a<<6)` TWICE per loop, with the pointer live
 * across the guard branch and copied in between; we compute it once.  The
 * `lw ...,0xC(s2)` COUNT is already equal (6 both sides, measured) - this is a
 * PLACEMENT difference, not a missing load.  The remaining differing insns in
 * the top-guard block are downstream of the same property.
 *
 * NEXT PROBE (banked and unspent since s1): the cc1 -da dumps.
 * tmp/grind/func_80017848/s4/dump.sh regenerates them in seconds.  Read
 * ings_pp.c.cse first - does cse fold the guard-address addu into the loop-base
 * addu, and at which insn?  Then .lreg/.greg for the pseudo pair's conflict and
 * preference records.  Do NOT open another preheader spelling sweep.
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
    base = slots;
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    i = 0;
    if (i < *(s32 *)((s32)base + (slot_a << 6) + 0x1C)) {
        p = *(u8 **)(ctx + 0xC);
        base = (u8 *)((slot_a << 6) + (s32)p);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
    }

    i = 0;
    if (i < *(s32 *)((s32)base + (slot_a << 6) + 0x20)) {
        p = *(u8 **)(ctx + 0xC);
        base = (u8 *)((slot_a << 6) + (s32)p);
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
