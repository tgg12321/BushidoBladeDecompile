/*
 * CANDIDATE â€” func_8003C714 (src/code6cac_c2.c) â€” s1 (2026-09-01), honest floor 15
 * (sandbox func_8003C714 --disable all == 15 with this body in place of the
 * INCLUDE_ASM line; chassis = HEAD at session s1, target 104 insns, build 105).
 *
 * 100% ordinary C, zero cheats, zero FAKE constructs. The pre-migration d33 body
 * used register-asm pins (t0/a2/a1) â€” this supersedes it entirely.
 *
 * Structure discovered this session (see evidence.md):
 *  - dst/src MUST be recomputed from i each iteration ("index-derived pointers").
 *    loop.c then recognizes them as general givs of biv i and strength-reduces
 *    them to reduced-giv registers (init `addu a1,s0,zero` / `lui+addiu a2,%D`,
 *    step 4/8) with the address offsets 0x21..0x24 / 4,0 preserved in-place.
 *    Plain pointer bivs (src+=8/dst+=4 spelling) instead get biv-75 ELIMINATED
 *    into a biased combined giv (a1=s0+36, offsets -3..0) and an extra IV for
 *    src+4 â€” that spelling is dead (rejected/plain-pointer-bivs.c).
 *  - i += 1 MUST be the last statement of the body. Mid-body increment splits
 *    the giv value and forces a `move a0,a3` copy. sched1 hoists the addiu into
 *    the 4th mult's mfhi shadow exactly as target.
 *
 * [s2 2026-09-01 UPDATE] This body is BYTE-EXACT (distance 0, 104==104) ï¿½ but
 * only when cc1 is told the PS1 has no FPU. Add `-msoft-float` (or the 32
 * `-ffixed-$f0..$f31`) to CC_FLAGS and this exact C scores 0; with the shipped
 * CC_FLAGS it scores 15. Mechanism: loop.c:532
 * `threshold = 2*(1+n_non_fixed_regs)` is 122 with the FP regs free and 58 with
 * them fixed; after the `threshold -= 3` that follows the first hoist
 * (loop.c:1719/1904) the /1800 magic faces 55 against the loop's insn_count 56
 * and stays in-loop in the split lui/ori form the target has. NO C SPELLING can
 * reach that on the shipped chassis (it would need a >119-insn loop inside a
 * 104-instruction function). Do not grind this body further; see
 * hypotheses.md H4/K5 and the 2026-09-01 docs/grind/decisions.md entry.
 *
 * [s3 2026-09-01 UPDATE] Re-verified unchanged on the current chassis: score 15,
 * 104 target / 105 build, movable table identical. s3 added the first POSITIVE
 * evidence that this body is the right C: when loop.c:1631 is made to decline
 * the 0x91A2B3C5 movable (diagnostic form s3/body_callD.c), the SHIPPED cc1
 * emits `lui v0,0x91a2 / lw v1 / ori v0,0xb3c5 / mult v1,v0` ï¿½ the target's
 * 8003C754..8003C760 quartet, instruction for instruction. The residual is the
 * desirability arithmetic alone; savings and lifetime are both pinned at their
 * structural minimum of 1 (loop.c:791/793), so the product equals `threshold`.
 * See hypotheses.md H6/H7 and K10/K11/K12.
 *
 * [s4 2026-09-01 UPDATE] Re-verified again on the current chassis: score 15,
 * 104 target / 105 build, movable table unchanged. Two additions this session,
 * both negative and both now banked: (a) the FIRST decomp-permuter campaign for
 * this function ran 66,016 iterations over this exact body and improved on its
 * base score ZERO times (workspace recipe in tmp/grind/func_8003C714/s4/mkws.sh,
 * validated so its base-vs-target diff is exactly the d15 residual); (b) the
 * inherited K9 claim that defeating the hoist needs 23 further hoists is
 * arithmetically wrong -- threshold reduction and insn_count inflation compound,
 * and the true crossover is 13 (measured: the 0x91A2B3C5 movable flips to "not
 * desirable" at k=13 added invariants). The channel is REAL on the shipped
 * chassis with no call in the loop, and is byte-foreclosed only because each of
 * the 13 hoists emits a preheader instruction and the target preheader
 * (8003C73C..8003C750, six instructions) has room for zero. See hypotheses.md
 * K13/K14 and rejected/invariant-hoist-threshold-decrement-needs-13-preheader-insns.c.
 *
 *
 * [s5 2026-09-01 UPDATE, synthesis modality] Re-verified once more on the
 * current chassis: score 15, 104 target / 105 build. This session merged s1-s4
 * into one law instead of opening a new spelling axis, and measured the one
 * channel the ledger had left half-open. BYTE-COUNT COUPLING (H8): on this
 * chassis loop.c's `insn_count` is a faithful proxy for the emitted instruction
 * count -- byte-neutral C is count-neutral C. Mechanism, read from
 * tools/gcc-2.7.2/toplev.c this session: every deletion opportunity cheap enough
 * to be byte-free happens BEFORE loop_optimize (expand folding, cse1 at
 * toplev.c:2865, delete_dead_from_cse at toplev.c:2866), and everything that
 * survives loop_optimize (toplev.c:2895) also survives flow (2984), combine
 * (3004), reload and the assembler. Two new measured nulls support it: dead
 * in-loop ALU chains at k = 4/8/16/32 leave insn_count PINNED at 56 with a
 * byte-identical function (K15), and pointer/giv chains at k = 1..16 do the same
 * (K16) -- with the identical harness reproducing K14's k=13 flip as a positive
 * control. Consequence: declining the 0x91A2B3C5 movable needs +64 RTL insns
 * (no extra hoists) or +28 RTL insns plus 13 extra preheader instructions, and
 * by H8 both convert 1:1-or-worse into bytes against a 104-instruction target
 * with a 6-instruction preheader. loop.c:1631 is closed BY CONSTRUCTION, not by
 * search. Do not open another spelling, structural or search modality on this
 * body. See hypotheses.md H8/K15/K16 and the s5 frontier.
*
 * [s6 2026-09-01 UPDATE, synthesis modality] READ THIS BEFORE SPENDING ANY
 * EARLIER PARAGRAPH. Chassis re-checked first: this body still scores 15
 * (104 target / 105 build). Everything above about loop.c:1631 being "closed
 * by construction" is now WRONG and is superseded: s6 MEASURED sandbox
 * distance 0 on the SHIPPED chassis. The s5 BYTE-COUNT COUPLING law (H8) is
 * FALSIFIED. A free insn_count channel exists: LOOP-CARRIED arithmetic on a
 * local whose only use is post-loop survives cse1/delete_dead_from_cse
 * (which is what killed every earlier attempt), is counted by
 * count_loop_regs_set, and is then deleted inside loop_optimize by biv
 * elimination for ZERO emitted bytes. Sweep: 32 balanced `z += c; z -= c;`
 * pairs take insn_count 56 -> 121 with the emitted function BYTE-IDENTICAL at
 * every intermediate k. At insn_count >= 120 the 0x91A2B3C5 movable is
 * declined and the whole function matches: score 0, 104 == 104.
 * That exact form is an INADMISSIBLE CHEAT (see
 * rejected/balanced-biv-noise-64-insns-d0-but-inadmissible.c) and was NOT
 * submitted. This body remains the honest ordinary-C best at 15. The function
 * is now a SPELLING problem with an exact target -- see hypotheses.md
 * H9/K17/K18 and the s6 frontier.
 *
 * [s7 2026-09-01 UPDATE, solver modality] Chassis re-checked first: this body
 * still scores 15 (104 target / 105 build). s7 closed all three s6 frontier
 * items with measurements and added the exact desirability arithmetic from the
 * baseline .loop dump: the 0x91A2B3C5 movable is the SECOND entry in the
 * movable list, so it is tested at `119 * savings * lifetime >= insn_count`
 * with savings = lifetime = 1 (both already at their structural minimum,
 * loop.c:791/793) against insn_count 56. Requirement: c > 63 - 3h (c = extra
 * insn_count, h = extra movables moved BEFORE it). Both terms are now measured
 * closed for ordinary C:
 *   K20 the loop.c:1609 `moved_once` DOUBLING fires (59 -> 118, 60 -> 120) but
 *       is self-defeating -- the inner-loop hoist that sets moved_once inflates
 *       m->lifetime 1 -> 14/15, and lifetime multiplies the threshold side.
 *   K21 there is NO natural carrier: a dead data-dependent carrier is +0 count
 *       (cse1 removes it), a LIVE one is +1.12N..+2.5N emitted instructions.
 *       strength_reduce only deletes carriers whose exit value it can FOLD,
 *       i.e. CONSTANT-step bivs; anything reading loop data is not a biv.
 *   K22 free HOISTS do not exist: a step constant big enough to be hoisted
 *       makes the biv increment a REGISTER and disqualifies biv elimination
 *       (~9 emitted instructions each); a step small enough to stay free folds
 *       into addiu and creates no movable. Mutually exclusive channels.
 * The ra_solver's object-level classifier reports an RA component ($t2 -> $t1
 * x10) and a nop-only SCHED component; both are CONSEQUENT on the hoist (s6
 * K18 measured every seat landing at score 0), so do not open an RA search.
 * This body remains the honest ordinary-C best at 15. See hypotheses.md
 * H11/H12/K20/K21/K22 and evidence.md section s7.
 *
 * [s8 2026-09-01 UPDATE, forensics modality] Chassis re-checked first: this body
 * still scores 15 (104 target / 105 build, cheat_asm_stripped 10, rules_dropped
 * 0). s8 closed the last two open items on the loop.c:1631 axis, so the
 * inequality `threshold * savings * m->lifetime >= insn_count` is now measured
 * closed TERM BY TERM for ordinary C on the shipped chassis:
 *   K23 ADMISSION is dead (s7 frontier item 2). scan_loop's admission gate is
 *       exactly three tests and a compiler-generated CONST_INT set passes all
 *       three unconditionally: loop.c:649 `may_not_optimize` (written only by
 *       count_loop_regs_set at loop.c:3037/3044, and two source-level /1800
 *       divisions give two DISTINCT pseudos so it never fires), loop.c:695-700
 *       whose `(! REG_USERVAR_P && ! REG_LOOP_TEST_P)` disjunct is true for
 *       every compiler temp, and loop.c:715 whose may_trap_p is 0 for a
 *       CONST_INT. m->cond and m->global are NOT admission gates: m->global
 *       (loop.c:790) is read only for m->savemode on partial movables
 *       (loop.c:888), and m->cond is structurally 0 because
 *       invariant_p(const_int) returns 1. Five spellings measured (division in
 *       a conditional BB, a second /1800 in the else arm, a /1800 before the
 *       loop, one after it, a second one in the same BB): all still hoist, all
 *       cost +2..+16 asm lines. The two-basic-block form BACKFIRES -- loop.c
 *       MATCHES the two magic loads and savings goes 1 -> 2.
 *   K24 the movable ORDER dial is FREE and ordinary C but caps at -6. Plain
 *       statement reordering moves the magic from movable slot 2 to slot 3 with
 *       insn_count pinned at 56 and 107 asm lines -- but this loop has only
 *       THREE movables, so threshold reaches 116 against a < 56 requirement,
 *       and the reordering relocates the emitted /1800 quartet away from the
 *       target's order (which this body already has).
 *   K25 loop FORM is insn_count-neutral: do-while / for / while all give 56
 *       real insns and 107 asm lines; the loop_top counting route (loop.c:592)
 *       adds only NOTEs, not 'i'-class insns.
 *   K26 `threshold` (loop.c:532) is not a C-reachable dial: n_non_fixed_regs
 *       moves only via CC_FLAGS (barred) or globalize_reg (regclass.c:530,
 *       i.e. 32 file-scope register-asm pins -- the forbidden pin family), and
 *       loop_has_call only halves it to 61, where 61 - 3 = 58 >= 56 still
 *       hoists.
 * With s7's H11 (savings and lifetime both pinned at 1) and s7's K21
 * (insn_count's only free carrier is dead code by construction), nothing on
 * this axis is open. This body remains the honest ordinary-C best at 15. See
 * hypotheses.md H13/K23/K24/K25/K26 and evidence.md section s8.
 *
 *
 * [s9 2026-09-01 UPDATE, escalation/disposition modality] Chassis re-checked
 * first: this body still scores 15 (104 target / 105 build, rules_dropped 0,
 * cheat_asm_stripped 10). No new grinding axis was opened; s9 ran the two
 * endgame-lock AND-gates and both FAILED. (a) tools/scan_hand_coded.py --single
 * reports tier=LOW score=0/8 with none of the STRONG signals S1/S2/S6 firing --
 * measured, where s1-s8 had only assumed it -- so the canonical-asm grant path
 * is closed. (b) A precedent census of docs/reference/sotn-construct-index.md
 * (sotn-decomp master aa53500226) returns ZERO hits for insn_count / LICM /
 * hoist / invariant / biv / loop-carried across all twelve construct classes, so
 * the s6 distance-0 form (balanced loop-carried counting noise) has no
 * SOTN-master precedent and stays an auto-reject. Under the owner's 2026-07-27
 * standing ruling the function is therefore FORECLOSED, recorded at
 * docs/grind/decisions.md:19815 with three named re-activation triggers. THIS
 * BODY REMAINS THE HONEST ORDINARY-C BEST AND IS THE FORM TO RESTORE if any
 * trigger fires. See hypotheses.md K27/K28/H14 and evidence.md section s9.
 * Remaining d15 residual = ONE loop.c decision + its seat fallout:
 *  build hoists the 0x91A2B3C5 (/1800 magic) const load to the preheader
 *  (movable, savings 1, life 1, threshold 122 vs insn_count ~56); target has it
 *  IN-LOOP as a TWO-insn lui/ori pair interleaved by sched1 with the first lw
 *  (lui v0 / lw v1 / ori v0 / mult). With the hoist gone, t1 frees and the mfhi
 *  temp (our t2, target t1) is expected to fall into place -> plausibly d0.
 *  See hypotheses.md for the attack frontier on that single decision.
 */
void func_8003C714(void) {
    u8 buf[4];
    s32 *s0;
    s32 i;
    u8 *src;
    u8 *dst;

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    do {
        src = (u8 *)&D_80106A58 + i * 8;
        dst = (u8 *)s0 + i * 4;
        dst[0x21] = *(s32 *)(src + 4) / 1800;
        dst[0x22] = (*(s32 *)(src + 4) / 30) % 60;
        dst[0x23] = ((*(s32 *)(src + 4) % 30) * 100) / 30;
        dst[0x24] = *src;
        i += 1;
    } while (i < 3);
    func_8001CD68(buf);
    *((u8 *)s0 + 0x2D) = *(u16 *)buf;
    *((u8 *)s0 + 0x2E) = buf[2];
    *((u8 *)s0 + 0x2F) = buf[3];
    *((u8 *)s0 + 0x30) = *(u16 *)&D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
