/* [s60 SOLVER - body below UNCHANGED (BASE, 3 at 127/127 on the HEAD chassis;
 * fake_ablate: no FAKE construct).  K2 (candidate_alt_s56_k2_..., 4 at 127/127)
 * remains the faithful chassis for the seat residual.  What s60 established:
 *  1. classify (object path) on K2 = RA, seat-only; the global.c model is
 *     exact (15/15).  The shared copy dest (pseudo 78) has hard conflicts
 *     {v1, sp} only, does NOT conflict with q/base (79/81) and carries a v0
 *     full preference, so pass 0 hands it v0.
 *  2. inverse global with the FULL 13-seat goal, depth 2 and 3, 403 atoms:
 *     exactly one vector - reroute 78's preference {v0} -> {a3}.  That vector
 *     is CLASS-KILLED: a3 appears in the pre-RA RTL only as slot_b's incoming
 *     copy (pseudo 75, call-crossing so pruned, dying at the `ior` into a
 *     local temp), so set_preference/expand_preferences (global.c:829-870,
 *     1645-1700) have no path to 78.
 *  3. Model variants pin the ONLY in-model route to a3 with every other seat
 *     intact: 78 must conflict with base (79/81) AND a v0 local AND have
 *     priority < lnk's 2500 (livelen >= 9 for a per-loop p, >= 33 for the
 *     shared p) = a reader >= 8 insns after the copy, past the loop bottom.
 *     BASE does this for loop 1 (loop 2's guard reads p -> a3) and pays the
 *     join copy; loop 2 has no reader slot before the jal at all.
 *  4. No unallocated pseudo can exist (find_reg fails only on a full `used`;
 *     REG_EQUIV only from stack parms / block-local pseudos), so the s59
 *     frontier item 1 is void and reload's find_equiv_reg copy stays closed.
 * Frontier: hypotheses.md s60 (jump2-deleted reader; loop 2's copy as a
 * reload register rather than an allocno seat).  Evidence: E-s60-1..7.
 */
/* [s59 SYNTHESIS - body below UNCHANGED (BASE, 3 at 127/127 on the HEAD
 * chassis; fake_ablate: no FAKE construct).  Two facts change how to read it:
 *  1. The target's loop-1 exit-path `lw a0,0xC(s2); sll a1,s4,6` are NOT a
 *     tail copy: reorg (reorg.c:3442-3460, redundant_insn on an un-owned
 *     thread) retargets loop 1's blez past loop 2's ordinary join-block
 *     loads.  K2's dumps keep those loads inside the join block through
 *     sched2 and still print the target's bytes.  So the `p = q;` tail below
 *     is a device the target does not have; loop 1 matches by coincidence.
 *     K2 (candidate_alt_s56_k2_..., 4 at 127/127, both loops symmetric) is
 *     the faithful chassis; the residual is the copy-dest seat, twice.
 *  2. The copy-dest seat is conflict/order-decided and CAN leave v0: with the
 *     copy in the guard block and q live past the branch (W2), loop 1 prints
 *     `addu a2,a0,zero; lw a3,16(s2); addu a0,a1,a2` (a3 missed only because
 *     p is allocated before lnk).  But a guard-block copy lands in the blez
 *     delay slot; the target's copy sits after it, in the preheader.  s57 cell
 *     B died to cse's copy-swap (cse.c:7454, shared p canonical), and V1-V4
 *     (per-loop p) die to optimize_reg_copy_1 (local-alloc.c:700, q dead
 *     before the JUMP_INSN).  Frontier + details: hypotheses.md s59,
 *     evidence.md E-s59-1..7.
 */
/* [s58 ENUMERATE - body below UNCHANGED (BASE, 3 at 127/127, re-measured on the
 * HEAD chassis this session; tools/fake_ablate.py: no FAKE construct).  The
 * systematic spelling sweep (owner ruling 2026-09-08, tools/spelling_enum.py +
 * tools/sweep_variants.py) ran over SEVEN regions/chassis, 1,420 spellings in
 * total (1,298 after discarding the tool's `u8 * x` swap-bug outputs):
 *   loop-2 joint region (guard + preheader + body), guard via p ... 403: 8 at 3, none below
 *   same region, guard via a fresh ctx+0xC local ................ 302: flat 31-32
 *   K2 chassis loop-2 region (sh2/t2a/t2, q2 kept) ...............  12: flat 4
 *   commutative swaps on the 28 best loop-2 forms ................  56 valid: 16 at 3
 *   loop-1 region, decls above the guard .......................... 403: best 15
 *   loop-1 in-preheader region (q1/lnk1/b1 + tail copy, swaps) ...  60 valid: 6 at 3
 *   loop-2 in-preheader region (q2/lnk2/b2, swaps) ...............  60 valid: 24 at 3
 * ZERO hits below 3.  Every form at 3 is byte-identical to this body (naming
 * a guard sub-expression sh2n/ga/gc is inert).  The gradient inside the region
 * is monotone: naming lnk2 above the guard = 5; naming q2 above the guard
 * (the target's own lw-a0-in-guard-block shape) = 6 at 126; naming the base
 * add as well = 33.  CLASS KILL for the naming/declaration-order/commutative-
 * swap spelling space of both loop regions on this chassis: the residual is
 * not a spelling of these regions, it lives in variable identity / scope /
 * the object model (s57's RANGE frontier stands).  Histograms:
 * tmp/grind/func_80017848/s58/sweep_*.json; evidence.md E-s58-1..5.
 */
/* [s57 STRUCTURAL - READ THIS FIRST.  Body below UNCHANGED (BASE, 3 at 127/127;
 * K2 re-measured 4 at 127/127 on the HEAD chassis this session).  s57 read
 * global.c end to end for the copy destination's seat and CLOSED every
 * allocator route to a3 for a copy dest whose live range is [copy, base add]:
 *   - pass-0 scan (global.c:985-1078): a3 needs v0,v1,a0,a1,a2 all in `used`.
 *     a0 can enter `used` only as a conflict (impossible: q dies at the copy,
 *     base is born at the add, global.c:755-790) or as regs_someone_prefers
 *     (impossible: the only a0-preference holder is the ctx param pseudo, which
 *     must cross the call to seat in s2 and prune_preferences global.c:900
 *     strips call-used prefs from call-crossing allocnos).
 *   - preference override (global.c:1097-1160): needs an a3 event.  The only
 *     hard-a3 insn is the incoming-argument copy; measured (cell A, param
 *     reused as the copy dest) combine folds it into its single use, so the
 *     preference lands on a call-crossing pseudo and is pruned.
 *   - local-alloc (K1 shape): `used` = hard regs live in the block only ->
 *     v0.  reload's find_equiv_reg copy route (reload1.c:5843-5853, first
 *     spill reg = a3 = the only unused call-used a-reg, reload1.c:3766) would
 *     print EXACTLY the target's `addu a3,a0,zero`, but needs the copy dest
 *     UNALLOCATED, and global.c never leaves a GR_REGS allocno unallocated
 *     (find_reg fails only on class exhaustion, global.c:586-598).
 *   The v0 preference on the copy dest (s56 frontier item 2) is traced:
 *   expand_preferences (global.c:829) merges base's v0 preference (from the
 *   body's `(set v0-local (plus base i))`) into p at the add where p dies.
 * Consequence: on every chassis where the copy dest is an expanded C copy of
 * the guard's pointer with range [copy, add], global.c seats it in v0 (or
 * coalesces it into a0).  The next session must attack the RANGE assumption
 * or the pass that emits the copy, not the allocator.  Details: hypotheses.md
 * s57, evidence.md E-s57-1..7.
 */
/* [s56 REDERIVE - READ THIS FIRST.  Body below UNCHANGED (BASE, 3 at 127/127,
 * re-audited on the HEAD chassis this session, no FAKE construct).  It is still
 * the lowest-SCORING form but NOT the most advanced one:
 *
 *   memory/grind/func_80017848/candidate_alt_s56_k2_combine914_clobber_both_copies_seat_v0_4.c
 *   (cell K2) measures 4 at 127/127 with BOTH preheader copies present and
 *   every instruction target-exact except the copy destination's seat
 *   (v0 for a3, twice).  It supersedes s55's U1 (4 at 125, copies missing).
 *   Start there.  Its header carries the mechanism (combine.c:914 gate fed by a
 *   merged-away set of the copy source: `q = *(u8 **)(ctx + 0x10); lnk = q;`),
 *   the RA-solver readout of the residual (pseudo 78: global, pri 13333,
 *   hard_conf {v1,sp}, pass-0 scan -> v0) and what the a3 seat still needs
 *   (the copy dest must outlive the base add with no bytes, OR carry an a3 copy
 *   preference from a no-op'd hard-reg move).  Full detail: hypotheses.md s56,
 *   evidence.md E-s56-1..5.
 */
/* [s55 ESCALATION - READ THIS FIRST.  The body below is UNCHANGED (BASE, 3 at
 * 127/127, re-audited on the HEAD chassis this session, no FAKE construct).  It
 * is still the lowest-SCORING form, but it is NO LONGER the most advanced one:
 *
 *   memory/grind/func_80017848/candidate_alt_s55_u1_seat_exact_two_copies_missing_4.c
 *   (cell U1) measures 4 at 125/127 and its ENTIRE diff against the target is
 *   the two missing `addu a3,a0,zero` preheader copies and their two consumers
 *   reading a0 instead of a3.  All 125 emitted instructions match the target
 *   register-for-register.  Start there, not here.
 *
 * WHAT s55 FOUND (full detail in evidence.md E-s55-1..8, hypotheses.md s55):
 *  1. s54's pass attribution was WRONG.  The rewrite that turns a surviving
 *     preheader copy into the target's `addu a3,a0,zero` / `addu a0,a1,a3` pair
 *     is `optimize_reg_copy_1` (tools/gcc-2.7.2/local-alloc.c:700, called from
 *     update_equiv_regs at :1007) and it runs between the flow and lreg dumps -
 *     insn 89 reads reg 79 at ings.combine:6974 and ings.flow:9027, and reg 80
 *     at ings.lreg:8889.
 *  2. The preheader copy has a SECOND survival gate nobody tested in 54
 *     sessions: combine.c:914's `(! all_adjacent && use_crosses_set_p (src,
 *     INSN_CUID (insn)))`.  Set the copy's SOURCE between the copy and the base
 *     add and combine cannot merge it.  The target already emits exactly one
 *     instruction there - the ctx+0x10 lnk load - so the clobber is FREE.
 *     Spelling that load into the pointer variable makes the loop-2 preheader
 *     copy appear in both loops (cell V3, 14 at 127/127), and
 *     `ra_solver/inverse_compose.py classify` then reports
 *     `FIRST DIVERGENCE: RA - same instructions, different registers`.
 *  3. The target's seat map says the ctx+0xC pointer variable and the loop base
 *     variable are ONE C variable (both a0), lnk has its own seat (a2), the copy
 *     dest is a3 and the index v1.  Spelling THAT (cell U1) makes every register
 *     exact - but then the base add itself is the only set of the source, so
 *     combine.c:914 does not fire and the copies merge away (125 insns).
 *  4. So the two gates are currently mutually exclusive: V3's clobber buys the
 *     copies and loses the seats; U1's variable identity buys the seats and
 *     loses the clobber.  Closing this function = finding one spelling that has
 *     BOTH, i.e. a set of the pointer pseudo between the copy and the add that
 *     is not the lnk load and does not extend any live range.
 *
 * The pre-s55 derivation of this body follows unchanged.
 */
/* [s54 SYNTHESIS - body unchanged, re-audited at 3 (127/127) on the HEAD chassis
 * (anchor src/ings.c:820); tools/fake_ablate.py reports no FAKE construct here.]
 *
 * THE RESIDUAL, STATED EXACTLY.  The three differing instructions are a pure
 * TRANSPOSITION inside loop 2 - nothing else in the function differs:
 *
 *      loop-2 guard      lw   a0,12(s2)     ->   addu a0,a3,zero
 *      loop-2 preheader  addu a3,a0,zero    ->   lw   v0,12(s2)
 *      loop-2 base add   addu a0,a1,a3      ->   addu a0,a1,v0
 *
 * Both sides emit exactly ONE ctx+0xC load and ONE reg-reg copy per loop.  The
 * target puts the load in the guard and the copy in the preheader; this body
 * puts the copy in the guard and the load in the preheader.  Loop 1, the
 * prologue, both bodies, both latches, the call block, the link stores,
 * rec_a/rec_b and the epilogue are byte-identical at 127/127.
 *
 * WHY, read from tmp/grind/func_80017848/dumps/ings.combine (line 6767 on):
 *   insn  83  (set (reg/v 80) (reg/v 79))                 loop-1 preheader COPY
 *   insn  89  (set (reg/v 81) (plus (reg/v 84) (reg/v 79)))  base add - reads the
 *                                                          copy's SOURCE, not 80
 *   insn 141  (set (reg/v 79) (reg/v 80))                 the `p = q;` exit tail
 *   insn 162  (set (reg 113) (mem (plus (reg/v 72) 12)))  loop-2 preheader LOAD
 *
 * So loop-1's copy survives combine only because reg 80's second use is the
 * tail at insn 141, and that same tail is a JOIN copy that materialises at the
 * merge block - which IS loop-2's guard block, where the target has its load.
 * Loop 1's copy and loop 2's wrong guard instruction are two ends of ONE device.
 * reg 113 is the "local allocno, blk 13, refs 2" the s47 solver typed without
 * being able to name its source statement.
 *
 * THREE DEVICES IN THIS BODY ARE LOAD-BEARING AND MUST NOT BE "TIDIED":
 *   1. `t = sh + (s32)p; t = *(s32 *)(t + 0x1C);` - reusing ONE variable for the
 *      guard address and the guard value clobbers the pseudo holding sh + p
 *      (combine insns 72 and 75 both write reg 86), which is what forces the
 *      preheader to RECOMPUTE the base add.  Spell the guard as a single
 *      expression and cse folds the preheader's add into the guard's address
 *      and leaves a copy of BASE after it (cell XM, 17 at 126).
 *   2. `p = q;` at loop-1's exit - the only thing keeping loop-1's copy alive.
 *   3. loop 2's guard testing through the register `p` rather than a fresh read.
 *
 * WHAT IS DEAD (s54 measurements, all on this chassis, no FAKE anywhere):
 *   - `while`-loop chassis (jump.c duplicate_loop_exit_test synthesises the
 *     guard AND a preheader copy, but loop.c then hoists the latch bound):
 *     WA = 29 at 120.
 *   - preheader statement ORDER: Z (named q2 + lnk2 in loop-1's order) = 3,
 *     byte-identical.  combine's adjacency test is not the gate.
 *   - loop-2 base sourced from loop-1's carrier with a fresh guard read:
 *     X1/X2/X4 = 7 at 127 (join copy moves ABOVE the guard load).
 *   - any second use for loop-2's carrier: address uses fold to base
 *     (ZD = 5 at 128, ZE = 6 at 128); value uses eat one of the target's three
 *     post-loop ctx+0xC reloads (M/M2/M3/F2/F2b/N2/XM all 16-17 at 126).
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

