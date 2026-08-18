/* CANDIDATE - func_80017848, s9 (2026-08-18, rederive).  sandbox --disable all = 3
 * FLOOR 5 -> 3 (variant V1_sh2_both).  Residual is 3 differing instructions, all
 * of them in loop 1's EXIT TAIL and loop 2's PREHEADER.  Loop 1's preheader -
 * including target's long-unexplained dead reg-reg copy `addu a3,a0,zero` - is
 * now BYTE-EXACT for the first time in nine sessions.
 *
 * THE SESSION'S CENTRAL DISCOVERY: HOW TARGET'S DEAD PREHEADER COPY IS PRODUCED.
 * Sessions s6/s7/s8 concluded the copy had to be created by a pass AFTER
 * combine.c (local-alloc / global-alloc / reload), because "combine deletes any
 * copy it can see".  That conclusion is now REFUTED with a measurement.  combine
 * only deletes a copy it can SUBSTITUTE, and can_combine_p refuses when the
 * copy's destination is still used AFTER the insn being combined into.  So a
 * copy survives combine iff its destination pseudo has a SECOND use downstream
 * of the base add.  Concretely, loop 1's preheader is now:
 *
 *      q   = *(u8 **)(ctx + 0xC);   <- cse folds this redundant load to `q = p`,
 *                                      i.e. a plain reg-reg COPY insn
 *      lnk = *(u8 **)(ctx + 0x10);
 *      base = (u8 *)(sh + (s32)q);  <- base add reads q
 *      ... do/while ...
 *      p = q;                       <- THE SECOND USE.  Without it combine
 *                                      substitutes q:=p into the base add,
 *                                      the copy dies, and local-alloc's
 *                                      combine_regs ties base to the dying
 *                                      pointer, giving `addu a0,a1,a0`.
 *
 * Evidence that this is the mechanism and not a coincidence: the -da .lreg dump
 * for the s8 chassis (tmp/grind/func_80017848/s9/dump/F_lreg.txt, insn 85) shows
 * `(set (reg/v 79) (plus (reg/v 82) (reg/v 78)))` carrying REG_DEAD for 78 - the
 * pointer dies exactly at the base add, which is precisely the tie-to-a-dying-
 * input case.  Giving the addend a second use removes the REG_DEAD note, the
 * tie does not happen, and both `addu a3,a0,zero` and `addu a0,a1,a3` appear
 * with target's exact register numbers.
 *
 * THE THREE LEVERS STACKED THIS SESSION (each measured on its own):
 *  (1) 5 -> 6 -> ... `q` (a named local holding a redundant re-read of ctx+0xC)
 *      as loop 1's base addend, with `p = q` as the loop-1 exit tail instead of
 *      a fresh re-read (variant R2 = 6).  On its own this LOSES a point, because
 *      the tail's `p = q` materialises as `addu a0,a3,zero` where target has
 *      `lw a0,12(s2)`.  It is only worth taking together with (2).
 *  (2) 6 -> 4  an explicit `lnk = *(u8 **)(ctx + 0x10);` local for loop 1's links
 *      pointer, read BETWEEN the `q` read and the base assignment (variant S1).
 *      Order matters: reading lnk BEFORE q (S2) is 6, not 4.  This fixes the
 *      position of `lw a2,16(s2)` relative to the copy/base pair.
 *      (s7 had killed the links local outright as variants X1/X2/X3 - on the s7
 *      and s8 chassis.  Fourth confirmation that every spelling conclusion on
 *      this function is chassis-relative.)
 *  (3) 4 -> 3  a SECOND shift local `sh2 = slot_a << 6;` recomputed immediately
 *      before loop 2's guard and used by loop 2's guard AND base (variant V1).
 *      Guard-only (V2) is 5 and base-only (V3) is 7 - both sites must move
 *      together.  This restores target's `sll a1,s4,6` in loop 2's preheader,
 *      which the single shared `sh` of the s8 candidate had hoisted away.
 *      NOTE this directly contradicts s8's frontier item #2, which predicted the
 *      shift had to be shared by the two GUARDS and recomputed for loop 2's
 *      BASE; the 16-way sweep of that four-way assignment (s9 variants M0000..
 *      M1111) proved the opposite - see the rejected/ bank.
 *
 * THE REMAINING 3 (T.txt vs B.txt):
 *   loop-1 exit tail: target `lw a0,12(s2)`      ours `addu a0,a3,zero`
 *   loop-2 preheader: target `addu a3,a0,zero`   ours `lw v0,12(s2)`
 *                     target `addu a0,a1,a3`     ours `addu a0,a1,v0`
 * i.e. loop 2 still needs the copy that loop 1 now has, and loop 1's tail still
 * pays for supplying loop 1's second use.  The two are the SAME missing fact:
 * a second downstream use for loop 2's base addend.  Every use site available
 * after loop 2 was measured this session and all of them are fatal, because
 * target re-reads ctx+0xC freshly three more times in the tail (math_Distance3D,
 * rec_a, rec_b) - routing any of them through a live local costs 19 points
 * (T1/T3/T4 = 22).
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
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
    sh = slot_a << 6;
    i = 0;
    t = sh + (s32)p;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        q = *(u8 **)(ctx + 0xC);
        lnk = *(u8 **)(ctx + 0x10);
        base = (u8 *)(sh + (s32)q);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
        p = q;
    }

    i = 0;
    sh2 = slot_a << 6;
    if (i < *(s32 *)(sh2 + (s32)p + 0x20)) {
        base = (u8 *)(sh2 + (s32) * (u8 **)(ctx + 0xC));
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
