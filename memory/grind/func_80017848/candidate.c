/* CANDIDATE - func_80017848, s6 (2026-08-18, forensics).  sandbox --disable all = 14
 * 125/127 build insns, frame 0x40 both sides.
 *
 * THIS REPLACES THE S4/S5 12-FORM AND THE FLOOR MOVES 12 -> 14 ON PURPOSE.
 * The inherited 12-form is SEMANTICALLY DIVERGENT from the target and therefore
 * provably cannot reach distance 0.  s6 proved it from the RTL and from the
 * disassembly, not by argument:
 *
 *   The 12-form declares `base = slots;` and then RE-POINTS `base` inside loop 1's
 *   preheader (`base = (u8 *)((slot_a << 6) + (s32)p);`) while loop 2's ENTRY GUARD
 *   still reads its count through `base`:
 *       if (i < *(s32 *)((s32)base + (slot_a << 6) + 0x20))
 *   On the path where loop 1's guard passed, `base` already holds `ptr + slot_a*64`,
 *   so that guard addresses `ptr + 2*slot_a*64 + 0x20`.  The target addresses
 *   `ptr + slot_a*64 + 0x20` on the same path - it RELOADS the pointer in loop 1's
 *   exit tail (`lw a0,0xC(s2)` at 0x80017914) precisely so the second guard sees a
 *   fresh pointer.  Confirmed in the cse dump (insn 146 `reg111 = reg/v79 + reg110`
 *   consumes the loop-1 preheader's reg/v79 set at insn 86) and in the build
 *   disassembly (`sll a1,s4,6 ; addu v0,a0,a1 ; lw v0,32(v0)` with a0 = loop 1's
 *   base).  Byte-identical code implies identical semantics, so no compilation of
 *   the 12-form can produce the target's bytes.  The whole s3/s4/s5 chassis - and
 *   the ~230 swept cells and ~80k permuter iterations spent around it - was
 *   searching a form that could never close.  Banked as
 *   rejected/base_reuse_loop2_guard_is_semantically_divergent.c.
 *
 * THIS FORM (variant B) is the best SEMANTICALLY CORRECT one measured in s6:
 *   B  both scan-loop entry guards read their count through `slots` (the single
 *      pre-join read, never re-pointed); each loop's base comes from its own fresh
 *      read after the branch                                             -> 14
 *   A  12-form + a fresh `base = *(u8 **)(ctx + 0xC);` before loop 2's guard -> 15
 *   D  fresh pre-guard read `p` per loop feeding the guard, base from `slots` -> 15
 *   G  D + an explicit copy variable feeding the base                      -> 15
 *   C  re-read into `slots` before loop 2's guard (closest to the HEAD form) -> 16
 *   I  D but the base's addend is a copy of `p` (cse re-folds)             -> 17
 *   F  D but loop 2's base through `p`                                     -> 18
 * (HEAD's untouched while-loop form is 16, so 14 is still the best correct floor
 * this function has ever had.)
 *
 * WHAT S6 ESTABLISHED ABOUT THE REMAINING RESIDUAL (all pass-level, from -da dumps
 * under tmp/grind/func_80017848/s6/):
 *  1. The cse fold IS defeatable and the lever is named.  In the E-s5-3 "hoisted
 *     read" family one C pointer feeds both the guard address and the loop base,
 *     both expressions land in the SAME extended basic block, and cse.c replaces
 *     the second `(plus shift p)` with a copy of the first (hoist dump, insn 83
 *     `(set reg/v79 reg93)`), which is why that family loses 5 instructions.
 *     Sourcing the base's addend from a pseudo that was live BEFORE the join label
 *     (variant D/G: guard through the fresh post-join read, base through `slots`)
 *     defeats the fold - G's dump shows insn 71 `reg94 = reg78 + reg93` and insn 86
 *     `reg/v80 = reg93 + reg/v77` as two independent addsi3s - and the build then
 *     contains BOTH target's pre-guard `lw v0,12(s2)` and two separate addus.  Its
 *     cost is that `slots` must stay live, which is why D/G sit at 15.
 *  2. Target's `addu a3,a0,zero` CANNOT be written in C.  s5 attributed the
 *     disappearance of an explicit copy statement to allocator coalescing; the
 *     dumps say otherwise.  In variant G's own dump the copy's insn is already
 *     `(note 82 ... NOTE_INSN_DELETED)` in ings_pp.c.combine, i.e. combine.c's
 *     try_combine substituted the copy into its single use (insn 86) and the dead
 *     copy was deleted BEFORE local-alloc ever ran.  No spelling of a C-level copy
 *     survives that, and G confirms it end to end (125 insns, unchanged from D).
 *     The `move a3,a0` is therefore an allocator/reload artifact of the pointer
 *     being live-out on the guard branch's TAKEN edge (in target a0 flows to loop
 *     2's guard at .L8001791C on the skip path), not a source construct.
 *
 * NEXT PROBE: the two remaining instructions are the two `addu a3,a0,zero` copies.
 * Build a form where ONE pointer variable is (a) read fresh after each join, (b)
 * used by that loop's entry guard, and (c) still LIVE-OUT on the guard branch's
 * TAKEN edge into the NEXT guard - that is the live-range shape that forces the
 * allocator to copy rather than overwrite.  In target that is exactly how it
 * happens: loop 2's guard consumes loop 1's pre-guard pointer on the skip path.
 * Read tmp/grind/func_80017848/s6/G/ings_pp.c.greg for reg/v77's conflict record
 * first.  Do NOT re-open the preheader spelling sweep and do NOT re-propose a
 * C-level copy statement - combine.c deletes it.
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

    i = 0;
    if (i < *(s32 *)((s32)slots + (slot_a << 6) + 0x1C)) {
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
    if (i < *(s32 *)((s32)slots + (slot_a << 6) + 0x20)) {
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
