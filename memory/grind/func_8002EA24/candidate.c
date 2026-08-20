/* func_8002EA24 -- grind session 4 candidate (permuter modality).
 *
 * Honest sandbox floor of THIS form: 2   (HEAD = 18; s1 honest respelling = 20;
 * s2/s3 authorized-sibling GTE respelling = 9; THIS session 9 -> 6 -> 3 -> 2).
 * Build 104 insns vs target 104 -- the instruction COUNT and every instruction
 * OPCODE now match; the entire residual is two register choices on one compare.
 *
 * WHAT CHANGED vs the session-3 candidate (three levers, all permuter-found and
 * then re-measured by hand in the cheat-invisible sandbox):
 *
 *   L1  tail 0/1 diamond (H6, was 3 of the 9 points).  `return 0;` in the LAST
 *       reject arm became `{ z = 0; return z; }`.  9 -> 6, and the build's
 *       instruction count went 102 -> 104 = target's.  Two statements in the
 *       arm break jump.c's store-flag if-conversion single-set precondition, so
 *       the `slt`/`xori $v0,$v0,1` fold does not happen and target's unfolded
 *       `bnez / addu $v0,$zero,$zero / addiu $v0,$zero,1` diamond is emitted.
 *       Measured load-bearing: dropping L1 from this body costs 3 points (5 vs 2).
 *
 *   L2  the compare-chain value `x` (H5, 3 of the 6 remaining points).  There is
 *       no separate `x` local at all any more: the SAME local that later carries
 *       the upper-Y bound carries the rotated-X test value first.  6 -> 3, and
 *       `x` moves from $a0 into target's $a1 (`lw a1,256(t0)` / `mult a1,a1`).
 *       This adds NO statement -- it removes a variable.  Measured identical
 *       (score 3) whether written as the permuter's `(max_y = x) < ...` staging
 *       or as the clean one-local form used here.
 *
 *   L3  `neg_threshold` (H5, 2 of the 3 remaining points).  The first range
 *       test's boolean is staged through `a0_var`:
 *       `a0_var = max_y < neg_threshold; if (a0_var || threshold < max_y)`.
 *       3 -> 2, and `neg_threshold` moves from $a0 into target's $t1
 *       (`negu t1,a2`, `slt v0,v1,t1`).  This is exactly session 3/4's H5
 *       route (a) mechanism, reached from the other end: staging a LIVE value
 *       into `a0_var` -- the function's only $a0-preferring allocno -- makes it
 *       live across the range-test chain WITHOUT hoisting the mult/mflo pairs
 *       (which is what killed the whole accearly/accshare family), so $a0 is
 *       denied to `neg_threshold` and first-fit hands it $t1.
 *       Measured load-bearing: dropping L3 costs 1 point (3 vs 2).
 *       The staging variable matters: a0_var 2, y_low 3, z 4, y 4, sp_var 6,
 *       min_y 10 -- only the $a0-preferring allocno produces the effect.
 *
 * REMAINING RESIDUAL (score 2, ONE instruction pair):
 *       ours    slt a0,a1,t1 ; bnez a0,<reject>
 *       target  slt v0,a1,t1 ; bnez v0,<reject>
 * i.e. target ALSO has `a0_var` in $a0 (`addu $a0,$v0,$v1` later) and ALSO has
 * neg_threshold in $t1, but its first range-test boolean lives in an ordinary
 * $v0 temp rather than in a0_var.  The next lever must make `a0_var` live
 * across the chain from a set whose VALUE is not the boolean -- see
 * hypotheses.md H5' for the exact statement.
 *
 * SESSION-6 FORENSICS (body UNCHANGED; floor still 2).  The residual is now
 * named exactly, in the compiler rather than in the source: pass `global_alloc`
 * (tools/gcc-2.7.2/global.c), decision = the pass-0 hard-register exclusion set
 * computed in `find_reg` (:1012-1044) for the `neg_threshold` allocno (pseudo
 * 103).  Measured with the BB2_FINDREG_DEBUG hook on THIS body and on the
 * `plain1` control (score 3, no staging): both have someone_prefers {6,7} and
 * EMPTY own preferences, and their conflict sets differ in exactly one member,
 * hard register 4 ($a0).  With the bit, find_reg's first free register is 9
 * ($t1) = target; without it, 4 ($a0).  find_reg can set that bit only four
 * ways, and all four are now measured: a hard-reg conflict with $a0 needs
 * neg_threshold live at function entry (s5: 108 insns, the negu loses the
 * load-delay slot); `regs_someone_prefers` is closed by GCC's own priority
 * order (both $a0-preferring allocnos, 97 = a0_var and 102 = y, OUTRANK 103);
 * an own-preference override needs a reg-reg copy seeding $t1, which nothing
 * supplies; and a conflict with the $a0 holder is instruction-free ONLY when
 * a0_var carries a value the chain already computes -- i.e. L3 below.  L3 is
 * therefore not one option among many: it is the unique instruction-free
 * generator of the one bit that separates this build from target.
 *
 * SESSION-7 FORENSICS (body UNCHANGED; floor still 2, re-measured this session).
 * Session 6's account of WHICH allocator mechanism supplies the bit is corrected
 * here from the dumps: the exclusion of $a0 from allocno 103 (neg_threshold) is
 * an ordinary ASSIGNED-CONFLICT, not a `regs_someone_prefers` effect.  Allocno 97
 * (a0_var) is allocated BEFORE 103 in every one of the 11 bodies dumped this
 * session (order `101 96 97 100 109 108 72 102 117 103 74 99 75`, identical in
 * all but the negtail family), so by the time find_reg runs for 103, 97 already
 * HOLDS hard reg 4 and conflicts with it.  103 lands in $t1 in exactly the builds
 * whose `103 conflicts:` list contains 97 and in $a0 in exactly those where it
 * does not; 103's own preferences are empty and someone_prefers[103] is {6,7} in
 * every build.  Consequence: no allocno-PRIORITY lever can substitute for the
 * conflict (measured: reference-count lifts are inert, CSE eats them before
 * global_alloc counts refs), and L3 remains the cheapest generator -- the second
 * range test's boolean carries the same conflict but costs 3 points (b2/b2or = 5),
 * and letting neg_threshold itself carry a tail value costs 20 (negtail1 = 22).
 * The one shape still unexplored is target's own: target's window carries the SAME
 * six conflicting values we do and still allocates $t1, so the original compile
 * excluded $a0 through an $a0-preferring allocno that conflicted with 103 from
 * BELOW it in the order -- see hypotheses.md H5''''.
 *
 * SESSION-8 REDERIVE (body UNCHANGED; floor re-measured at 2 this session).
 * A fresh m2c decompile of asm/funcs/func_8002EA24.s and eleven structurally
 * distinct rewrites built from it were measured.  Two results matter.
 *   (1) The score-2 plateau is SOURCE-SHAPE-INVARIANT.  Six genuinely different
 *       bodies -- neg_threshold written inline with no local at all; the local
 *       initialised after the x load instead of in the declaration list; the
 *       four range tests nested (m2c's own shape) instead of two early-return
 *       ifs; the four range tests fused into ONE short-circuit `if` with z
 *       assigned inside the condition; the sum-of-squares split off into its
 *       own local (m2c splits a0_var into temp_a0 / temp_a0_2 / var_a0); and
 *       that split with the staged boolean moved into the sum local -- ALL
 *       score exactly 2 at 104 instructions.  The residual does not move with
 *       source shape; it is one allocator bit, as sessions 6-7 concluded.
 *   (2) The "seventh live value" frontier axis is DEAD, with a mechanism.
 *       Reading `y` off the already-computed `vout` pointer (obj+0x100) keeps a
 *       seventh value live across the range-test window at zero instruction
 *       cost.  The .greg dump shows why it cannot work: the added carrier
 *       (allocno 77) is the LOWEST-priority allocno in the function -- long
 *       live range, few references, so allocno_compare puts it LAST, after 103
 *       -- and it has no register preferences at all.  It therefore cannot
 *       supply an assigned-conflict for 103 (it is allocated after 103) and
 *       cannot supply a prune_preferences exclusion either (no preferences to
 *       union).  What it actually does is STEAL target's $t1: 77 lands in $t1
 *       and 103 falls into $a0.  Any zero-cost added carrier has this shape by
 *       construction, so the axis is closed generally, not just for `vout`.
 * The consequence is that allocno 97 (a0_var) remains the ONLY allocno that can
 * deny $a0 to 103, and an enumeration of the values the C could put in it
 * before the chain finds none that is both real and read afterwards -- the sum
 * of squares is the only candidate and hoisting it is the measured-dead
 * accearly family.  L3 is therefore still the unique instruction-free generator.
 *
 * SESSION-9 REDERIVE (body UNCHANGED; floor re-measured at 2 at the start and
 * the end of the session).  The H5''''' enumeration that sessions 6-8 built is
 * now COMPLETE, in both directions, and the decomp.me corpus leg of the
 * rederive modality is measured dead.
 *   (1) Forward direction -- a real value computed BEFORE the range chain and
 *       read AFTER it, occupying allocno 97 (the function's only $a0-preferring
 *       allocno) at zero instruction cost.  The four remaining unmeasured
 *       carriers were measured against a control regenerated through the same
 *       template (control = 2): r_sq 19, r_sq with the boolean displaced onto
 *       `y` 19, the rotated X itself 6, threshold 24 -- all at 104 insns, i.e.
 *       pure register losses.  `v2` (X as the carrier) is the informative one:
 *       it obtains EVERYTHING the allocator model asks for -- 97 rises to first
 *       in the allocation order, keeps `preferences: 4`, is assigned $a0, and
 *       `103 conflicts:` contains 97 -- and 103 still misses $t1, landing in
 *       $a1, because with X folded into a0_var nothing occupies $a1 across the
 *       chain.  The only pre-chain values with enough references to outrank 103
 *       are the compare operands themselves, so consuming one as the carrier
 *       vacates the register the other one needs.  With the sum of squares (19),
 *       the delta temps (16-28), vin/vout (6/5), y hoisted (11) and -threshold
 *       (which IS allocno 103), the forward enumeration is complete and empty.
 *   (2) Dual direction -- a carrier that overlaps 103 but DIES inside the chain,
 *       so it never competes for $a1.  Its only non-parameter candidate is the
 *       GTE output pointer: reading X and Z as vout[0]/vout[1] scores 2 with L3
 *       and 3 without, i.e. exactly the banked body and exactly the no-L3
 *       control, and the .greg shows the allocno set and allocation order are
 *       IDENTICAL to this body's -- GCC re-materialises obj+0x100/0x104 off $t0
 *       and `vout` never becomes an allocno at all.
 *   (3) decomp.me corpus (3754 GCC-2.7.2 scratches, 1751 MATCHING).  "negu into
 *       $t/$s consumed by an slt" = 0 of 1751; the loose "negu into $t/$s" = 8,
 *       of which 7 are $s0/$s1 (callee-saved across a jal; this is a leaf) and
 *       the one $tN case is a whole-function argument copy in a function with
 *       five division expansions and a 7-argument call, plus an explicit
 *       register-asm pin.  Separately, 39 matched scratches keep an unfolded 0/1
 *       diamond (15 show the xori fold); 12 have no loop; every one inspected
 *       keeps the diamond because the arm holds REAL work -- a loop body, a
 *       store, or a call.  None has this function's shape (bare `return 0;`
 *       arm, bare `return 1;` fall-through).  Independent confirmation of
 *       session 2's pure-C tail exhaustion from a different direction.
 * Consequence: L3 is the unique instruction-free generator of the one allocator
 * bit (cheapest alternative 5) and L1 the unique defeater of the store-flag
 * fold, so the function's disposition is now a REVIEW question, not a search
 * question.  The one untried mechanical axis left is a permuter campaign from a
 * chassis OTHER than this body (session 5's basin exhaustion was measured from
 * this chassis only; seven structurally distinct score-2 bodies are now known).
 *
 * CHEAT VETTING (this body carries TWO annotated exceptions; layer-2 must rule).
 *   L1 and L3 are both instances of [[staged-value-reused-variable]]
 *   (SANCTIONED 2026-07-03): a REAL value, READ by the very next expression,
 *   staged through an EXISTING local whose previous value is provably dead at
 *   the staging point.  Neither is a dead store: `z` is returned, `a0_var` is
 *   the `if` condition.  They are therefore strictly milder than
 *   [[dead-store-fake-exception]], whose documented closure for L1's exact
 *   symptom (target keeps an unfolded 0/1 diamond) is a genuinely DEAD
 *   `ret = 1;` in the else arm.  Lever-exhaustion receipts: session 2 measured
 *   six pure-C tail shapes for L1 (shared end label, reversed final comparison,
 *   if/else both arms, ternary, test-order swap, double-goto-reject; three
 *   byte-identical, three worse) and sessions 2-4 measured ~20 structural
 *   shapes for the H5 register assignment (declaration order x3, the
 *   accearly/accmid/accpre/accsplit/xzptr family, accshare, vinlive,
 *   minmaxearly, the four delta-temp-sharing variants, xtop).
 *   L2 adds nothing and needs no exception: it deletes a local.
 *   NOT self-approved -- the driver's default-FAIL Judge and a fresh layer-2
 *   cheat-reviewer decide.  If either exception is refused, the fallback body
 *   is banked at candidate_alt_score3_no_fake.c (score 3, L3 removed) and
 *   without L1 as well the floor returns to 6.
 *
 * The GTE blocks, their two-statement split, the "=m"(sp_var) 0($sp) hardcode
 * and the whole H4 canonical-asm disposition are UNCHANGED from session 3 --
 * see that session's candidate header and hypotheses.md H4.  A sandbox
 * disassembly of a kept canonical block is missing the four GTE pipeline nops
 * (the cheat-invisible sandbox strips bare `nop` lines); they are in the
 * templates and in a real build.
         * Lever-exhaustion: session 2's six pure-C tail shapes. *
 * SESSION-10 SYNTHESIS (body UNCHANGED; floor re-measured at 2 this session).
 * The merged reading of sessions 2-9 plus a fresh read of global.c:793-899
 * opened ONE genuinely new axis and measured it.  Session 7 had proved that
 * the original compile cannot have excluded $a0 from allocno 103
 * (neg_threshold) by an assigned conflict -- target writes $a0 nowhere in the
 * range-test window -- so its exclusion must have come from
 * `regs_someone_prefers`, i.e. from an allocno that PREFERS $a0, CONFLICTS
 * with 103 and ranks BELOW it.  Sessions 7-9 treated the set of $a0-preferring
 * allocnos as fixed (97 = a0_var and 102 = y, both outranking 103) and
 * therefore treated that channel as unreachable.  It is not fixed:
 * `expand_preferences` (global.c:797-841) IORs preference sets BOTH ways
 * across any single_set insn that kills a NON-CONFLICTING allocno, and hard
 * reg 4 is in this function at all only because `obj` arrives in $a0 and the
 * preference has been propagated outward from it.
 *   (1) MECHANISM CONFIRMED, and it produces target's register pair without
 *       L3.  Writing the last range test's value into the dead `threshold`
 *       parameter (`threshold = y + a0_var; if (threshold < min_y) ...`) makes
 *       the insn that kills `y` (102, preferences {4}) set `threshold` (74,
 *       which conflicts with 103 and ranks below it).  Dump: `74 preferences:
 *       4 6`, and 103 lands in hard reg 9 = $t1 = TARGET with the first range
 *       test's boolean back in $v0 -- on the NO-L3 base.  Ten sessions of
 *       search had never reached that configuration.
 *   (2) It does not yet repay: the IOR is symmetric, so `y` inherits 74's $a2
 *       preference and leaves target's $v1 (score 6 vs the no-L3 control's 3;
 *       the `r_sq` spelling is also 6; both with L3 kept are 8).
 *   (3) The obvious fix -- a FRESH local as recipient, with no argument
 *       preference to leak back -- is inert (3) and the dump gives a second,
 *       sharper constraint: the recipient must not be preference-connected to
 *       103 itself, because prune_preferences line 893 removes from
 *       regs_someone_prefers[103] every register 103 itself prefers.  The
 *       fresh local hands 103 `preferences: 2 4` and the route cancels.
 * The banked body is unchanged because nothing measured below 2, but the
 * frontier is no longer "the residual is unreachable by any source shape": it
 * is a NAMED, MEASURED, reachable allocator channel with two stated
 * side-conditions -- see hypotheses.md H8'.
 */
s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    s32 *vin;
    s32 *vout;
    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
    vin = (s32 *)(obj + 0xF8);
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "lwc2 $0, 0($t4)\n"
        "lwc2 $1, 4($t4)\n"
        "nop\n"
        "nop\n"
        ".word 0x4A486012"
        : : "r"(vin) : "$12", "memory");
    vout = (s32 *)(obj + 0x100);
    __asm__ volatile(
        "addu $t4, %0, $zero\n"
        "swc2 $25, 0($t4)\n"
        "swc2 $26, 4($t4)\n"
        "swc2 $27, 8($t4)"
        : : "r"(vout) : "$12", "memory");

    {
        s32 z;
        s32 a0_var;
        s32 sp_var;
        s32 min_y;
        s32 max_y;
        s32 y_low;
        s32 y;
        s32 neg_threshold = -threshold;

        /* max_y carries the rotated X here and the upper Y bound below -- one
         * local, two jobs, no extra statement (see L2 in the header). */
        max_y = *(s32 *)(obj + 0x100);
        /* FAKE: stages the first range test's boolean -- a real value, read by
         * the very next `if` -- through a0_var, whose own value (the squared
         * distance) is not set until after the chain and is therefore dead
         * here.  Mechanism: GCC 2.7.2 global.c allocation; a0_var is this
         * function's ONLY $a0-preferring allocno, so making it live across the
         * range-test chain is what denies $a0 to neg_threshold and lets
         * find_reg's first fit hand neg_threshold target's $t1.  Family:
         * [[staged-value-reused-variable]].  Lever-exhaustion: hypotheses.md
         * sessions 2-4 (accearly/accmid/accpre/accsplit/xzptr, accshare,
         * vinlive, minmaxearly, tshare/tshare1/zshare/negshare, xtop, and the
         * five alternative staging variables measured this session). */
        a0_var = max_y < neg_threshold;
        if (a0_var || threshold < max_y) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        a0_var = max_y * max_y + z * z;
        if (r_sq < a0_var) return 0;
        a0_var = r_sq - a0_var;

        if ((u32)a0_var < 0x400) {
            a0_var = (u32)*(((u8 *)&D_8008D118) + a0_var) >> 3;
        } else {
            s32 lzcr = 0;
            if (a0_var >= 0) {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, $sp, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(a0_var) : "$12");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)a0_var >> shift));
                a0_var = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_y = 0;
        min_y = 0;
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else {
            max_y = y_low;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - a0_var) return 0;
        /* FAKE: stages the return value 0 through z -- a real value, read by
         * the very next statement -- whose own value (the rotated Z) last
         * mattered at the `z * z` above and is dead here.  Mechanism: jump.c's
         * store-flag if-conversion requires a SINGLE-SET arm, so a two-statement
         * arm keeps target's unfolded 0/1 diamond instead of folding it to
         * `slt` + `xori $v0,$v0,1`.  Family: [[staged-value-reused-variable]]
         * (a live-value cousin of [[dead-store-fake-exception]], which documents
         * this exact symptom with a DEAD store; this form has none).
 *
 * SESSION-11 STRUCTURAL (body UNCHANGED; floor re-measured at 2 this session).
 * Session 10's H8' -- the `regs_someone_prefers` preference route -- is now
 * CLOSED, by an exhaustive enumeration of the possible recipients rather than
 * by another failed spelling.  Read off this body's own .greg, the allocnos
 * that CONFLICT with 103 (neg_threshold) are exactly {72 obj, 74 threshold,
 * 75 r_sq, 96 z, 100 max_y}, and the allocation order is
 * `101 96 97 100 109 108 72 102 117 103 74 99 75`.  Every candidate recipient
 * is therefore one of six cases, and all six are now measured:
 *   72 (obj)      -- preferences pruned by its own hard-reg self-conflict (s3).
 *   100 (max_y)   -- CONFLICTS with the donor 102 (`y`), and expand_preferences
 *                    is gated on `! CONFLICTP` in both directions, so nothing
 *                    propagates: the .greg for the max_y recipient is
 *                    BIT-IDENTICAL to the control's (score 5).
 *   74 / 75       -- the session-10 parameters.  They work (103 -> $t1) but the
 *                    symmetric IOR hands `y` their argument register (6 / 6).
 *   fresh local   -- poisons `103 preferences` and re-ranks 103 to first; the
 *                    session-10 spelling carried the first range test's boolean
 *                    and this session's carried the other half of the same
 *                    short-circuit (which never mentions neg_threshold) -- both
 *                    inert at 3, so the poisoning is not operand-level.
 *   96 (z)        -- the only recipient that satisfies BOTH side-conditions:
 *                    `96 preferences: 4` (forward leg fires) with
 *                    `102 preferences: 4` UNCHANGED (harmless reverse leg) and
 *                    `103 preferences` still empty.  It fails ONLY on rank: 96
 *                    is allocated SECOND, so it takes $a0 itself instead of
 *                    denying it, and the assignment cascades (score 16).
 * The two ways out of the last case are both measured dead.  Demoting 96 below
 * 103 needs floor_log2(n_refs)*n_refs/live_length to fall by 3-8x; cutting z's
 * references (L1 staged through `y_low`, then L1 removed entirely) leaves the
 * allocation order BIT-IDENTICAL, and z's live range cannot start earlier
 * because `*(s32 *)(obj + 0x104)` is WRITTEN by the GTE store block directly
 * above the load.  Blocking `y` from the parameter's argument register needs an
 * allocno that conflicts with 102, outranks it and holds $a2; the only
 * candidate (99 = min_y) can only be made to conflict by an earlier birth, and
 * allocno_compare puts live_length in the DENOMINATOR, so the same edit demotes
 * it to LAST, where it steals target's $t1 outright (score 13 at 105 insns).
 * A conflict placed on 102 to stop the reverse leg also stops the forward leg,
 * since the IOR is gated on `! CONFLICTP` symmetrically.
 * One positive finding: L1's staging variable is FREE.  Staging the returned 0
 * through `y_low` instead of `z` measures 2 on this body and 3 on the no-L3
 * body -- identical, at target's 104 instructions -- so the store-flag defeat
 * is a property of the two-statement arm, not of `z`.  Banked as
 * candidate_alt_L1_via_ylow.c for whoever rules on the construct.
 */
        if (y + a0_var < min_y) { z = 0; return z; }
        return 1;
    }
}

/* ---------------------------------------------------------------------------
 * SESSION 13 (escalation, 2026-08-20) -- BODY UNCHANGED, FLOOR RE-MEASURED ON
 * THE POST-MIGRATION CHASSIS.
 *
 * The 2026-08-19 asm-until-matched migration replaced this function's on-main
 * representation with INCLUDE_ASM (src/code6cac_b.c:1160) and retired its 10
 * legacy regfix rules (archived at retired-chassis-2026-08/rules.txt), so every
 * conclusion banked before that date was chassis-relative and needed a re-read
 * before it could be spent.  Session 13 applied lines 229-373 of THIS file to
 * src/code6cac_b.c and measured:
 *
 *     sandbox func_8002EA24 --disable all
 *     {"score": 2, "target_insns": 104, "build_insns": 104, "scorable": true,
 *      "rules_dropped": 0, "cheat_asm_stripped": 237}
 *
 * Floor 2, identical to the pre-migration floor.  The plateau is therefore
 * chassis-invariant as well as source-shape-invariant, and the whole ledger
 * (44 rejected forms, ~118k permuter iterations, the find_reg forensics)
 * carries over intact.  Residual unchanged: slt $a0 vs target's slt $v0.
 *
 * DISPOSITION: both endgame-lock gates re-evaluated this session and both FAIL
 * (scan_hand_coded TIGHT_C 3/8, no S1/S2/S6; SOTN construct index census for
 * conflict/allocno/find_reg/global_alloc/zero-cost returns ZERO hits).  Filed
 * as REFUSED / OWNER-ACCEPTED INCOMPLETE per the owner's 2026-07-27 standing
 * ruling -- docs/grind/decisions.md:7843.  Re-attempt-eligible; on reopening,
 * start from THIS body, not from HEAD.
 * ------------------------------------------------------------------------- */
