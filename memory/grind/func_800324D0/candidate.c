/* [s20 2026-09-01] FORENSICS. Body UNCHANGED and re-measured this session on a
 * pristine-HEAD reference (build SHA1 == oracle, s20/build_head_reference.log):
 * score 15, 68 == 68, rules_dropped 0. THE FLOOR IS 15.
 *
 * THE FIND_REG PRIORITY CHANNEL IS NOW CLOSED IN CLOSED FORM. GCC 2.7.2
 * global.c sorts allocnos by
 *     pri = floor_log2(nrefs) * nrefs * size / reg_live_length * 10000
 * (tools/ra_solver/simulate.py:43) and hands each the first free hard register;
 * $v0 is local-alloc scratch and $a0 is the pad parameter's preference, so the
 * FIRST non-pad global allocno takes $v1. The target needs that to be the
 * walker. Over all thirteen instrumented-cc1 models measured to date
 * (s19 A,B,C,D,E,F,G,H,I,L,base + s20 M,N; tables in s20/priority_bound.log and
 * s20/simultaneity.log):
 *     walker  73: nrefs 24-25, livelen 62-66, pri 14769-15873  in EVERY body
 *     arm carrier: nrefs 26-28, livelen 21-34, pri 31764-49523 in EVERY body
 * The carrier would need livelen 67-76 to fall below the walker; its measured
 * ceiling is 34. Lifting the walker instead needs nrefs 50 (+13 RTL refs, >= +6
 * instructions) on a budget that is exact at 68. The twelve switch arms
 * concentrate 24 weighted references (in-loop refs count x2) into ONE pseudo by
 * the shape of the jump table, and that pseudo is born in the payload arm and
 * dead at the last arm store - respelling MOVES the references, it cannot
 * reduce or stretch them. Measured: the single-store shapes (M: arms set an
 * offset; N: arms set a destination pointer) demote val 47272 -> 2222 and change
 * the walker's register by nothing, because a new pseudo 77 inherits the seat at
 * pri 40000 (and they cost 71 / 70 insns).
 *
 * This bounds s18's 40320-order enumeration: its 336 target-reaching orders are
 * abstract permutations, but the order is the descending sort of a function of
 * (nrefs, livelen) whose reachable domain, measured here, excludes them all.
 *
 * Also killed this session: H's +1 cannot be paid by either banked -1 lever
 * (hoist + H = 69 insns; base + u32 c = 67 but the widening is worth -3 on the
 * H chassis, s19 probe K = 66). The levers interact; nothing lands on 68 with
 * H's allocation.
 *
 * ONLY UNMEASURED AXIS LEFT: splitting the twelve arm references across TWO
 * carriers of <= 15 weighted refs each (each would then need livelen > 28.4,
 * which IS inside the measured envelope) while keeping one 12-entry jump table
 * and 68 instructions. Evidence says a single jump table forces a single
 * carrier; nobody has measured it. Full write-up: evidence.md [s20],
 * hypotheses.md [s20]. */
/* [s19 2026-08-31] FORENSICS. Body UNCHANGED and re-measured this session on a
 * pristine-HEAD reference (build SHA1 == oracle, s19/build_head_reference.log):
 * score 15, 68 == 68, rules_dropped 0 (s19/sandbox_base.log).
 *
 * TWO THINGS IN THIS HEADER SUPERSEDE THE s18/s17 HEADERS BELOW.
 *
 * (1) THE PSEUDO MAP EVERY SESSION SINCE s5 HAS QUOTED IS WRONG. Read from the
 *     instrumented-cc1 .lreg RTL (tmp/grind/func_800324D0/dumps/code6cac_b.lreg,
 *     segment ";; Function func_800324D0"), insn by insn:
 *         72 = pad                       (insn 4, the 11 preheader stores)
 *         73 = walker ptr                (insn 11, increments 64/83/95/110/220)
 *         74 = stream byte c             (insn 61, tail insn 217, loop test 70)
 *         76 = operand byte val          (insn 107 + the TWELVE arm stores)
 *         75 = the biased command c-0x80 (insn 104, uses at 189 and 195)
 *         85 = zero_extend(c), a COMPILER TEMP (insn 76), no source variable
 *     The old map said 74 = val, 76 = c, 75 = "cmd head web", 85 = "cmd arm
 *     web". There is NO head-web allocno: combine collapses the staged tail
 *     (cmd = *ptr; c = cmd;) into one load into 74 before .lreg, so the FAKE
 *     construct below buys its 27->15 drop without owning an allocno.
 *     Re-read the s18 relief curve against this map: it asks for
 *     livelen(biased cmd) >= 20, livelen(val) >= 68, livelen(zext temp) >= 16.
 *
 * (2) THE s1 "ARITHMETICALLY DEAD" PRIORITY CLAIM IS FALSIFIED BY MEASUREMENT.
 *     Making the biased command LOOP-CARRIED (computed in the preheader and
 *     again at the loop latch) drops pseudo 75 from pri 75000 to pri 9333 -
 *     an 8x demotion, below the walker's 14769 - and seats 75 in its target
 *     register $a2, with allocno 85 vanishing entirely. That is the first
 *     spelling in 19 sessions where the walker is allocated before the cmd web
 *     (s19/model_H_cmd_staged.json). It costs exactly ONE preheader
 *     instruction: 69 vs the 68-insn target, so it is banked as rejected
 *     (rejected/cmd-subtraction-staged-across-back-edge-69insns.c).
 *     A 68-insn spelling of the loop-carried cmd DOES exist - re-express the
 *     head tests on the biased value - but it hands the saved references
 *     straight back (nrefs 7 -> 11, pri 23571) and scores 33
 *     (rejected/staged-cmd-head-tests-on-cmd-68insns.c).
 *     THE WHOLE REMAINING QUESTION for s20: pay for H's one instruction WITHOUT
 *     adding in-loop references to cmd. Attack val (76) or pad (72) or the
 *     preheader, not the tests.
 *
 * Also measured dead this session: in-body hoisting (2x too weak AND the two
 * short webs are anti-correlated), head-test order (jump.c normalises it to a
 * bit-identical model), the nrefs channel (cheapest abstract solution costs 25
 * units but nrefs converts 1:2 into instruction count), and val's live length
 * (ceiling ~31 against the required 68, because it dies in 12 places). Full
 * write-up: evidence.md [s19], hypotheses.md [s19]. */
/* [s18 2026-08-31] READ THIS BEFORE THE s17 HEADER BELOW: THE FUNCTION IS NOT
 * FORECLOSED. The s17 note "Do not grind this body" is withdrawn. s18 (solver)
 * enumerated ALL 40320 allocation orders against the exact ra_solver find_reg
 * model and found 336 that reach the FULL 8/8 target disposition, so the
 * foreclosure record's priority leg ("the walker needs a 4.84x lift, which is
 * arithmetically dead") is answering the wrong question: the target does NOT
 * require the walker to beat allocno 75. It requires pri(73) > pri(76) >
 * {pri(75), pri(85)}, with 72 and 74 unconstrained -- reachable by DEMOTING the
 * three short-lived allocnos rather than promoting the loop-carried walker. At
 * the walker's measured live length of 62, unchanged, the requirement is
 * livelen(75) >= 20 (from 4), livelen(76) >= 68 (from 22), livelen(85) >= 16
 * (from 7); shortening the walker relaxes all three (full Pareto curve:
 * tmp/grind/func_800324D0/s18/relief_curve.json). Live length IS
 * source-order-controllable -- measured, s18 H47. Correction entry filed at
 * docs/grind/decisions.md:17257. Body BELOW is unchanged and re-measured this
 * session at score 15, 68 == 68, rules_dropped 0
 * (tmp/grind/func_800324D0/s18/sandbox_candidate.log). Attack pseudos 75/76/85
 * (the two cmd webs and the stream byte c), not the walker. */
/* [s17 2026-08-31] RE-VERIFIED on a fresh reference (pristine checkout ->
 * build SHA1 == oracle -> this body -> sandbox --disable all): score 15,
 * 68 == 68, rules_dropped 0 (tmp/grind/func_800324D0/s17/
 * sandbox_candidate_freshref.log). FUNCTION FORECLOSED this session:
 * docs/grind/decisions.md:17155 (OWNER-ESCALATION — RESOLVED BY STANDING
 * RULING (2026-07-27): FORECLOSED). The pure-C residual is closed over
 * find_reg's entire pass-0 input space and s17 added the prototype
 * argument: a void leaf with one pointer param and no calls has no $v1
 * copy site, so set_preference can never plant the missing preference in
 * ANY spelling. Remaining work is an operator commit of the banked
 * INCLUDE_ASM migration (tmp/grind/func_800324D0/s17/migration.diff,
 * re-verified oracle-green on HEAD b94a65de). Do not grind this body. */
/* [s16 2026-08-31] RE-VERIFIED UNCHANGED on a fresh reference: pristine
 * checkout -> build (SHA1 == oracle) -> this body -> sandbox --disable all =
 * 15, 68 == 68, rules_dropped 0 (tmp/grind/func_800324D0/s16/
 * sandbox_candidate_freshref.log). s16 closed the foreclosure over find_reg's
 * COMPLETE pass-0 input space (global.c:998-1001): conflicts (s15), plus the
 * two non-conflict channels - ~regs_used_so_far is saturated ($3 is call-used,
 * leaf fn, global.c:363-368) and regs_someone_prefers is measured EMPTY for
 * every allocno that matters (s16/model.json) because $v1 has no copy site in a
 * void leaf with one pointer param, and is self-defeating by rank via
 * prune_preferences even if it were not. Do not spend another session on a
 * register-seat spelling; see hypotheses.md H38/H39 and the s17+ frontier. */
/* [s15 2026-08-31] CHASSIS CORRECTED - THE FLOOR IS 15, NOT 17. The s14 "+2
 * drift" was a MEASUREMENT ARTIFACT with a fully identified cause, not codegen:
 * `sandbox` scores against the REFERENCE object build/src/code6cac_b.o, and s14
 * left that object built from its own INCLUDE_ASM/INCLUDE_RODATA migration of
 * this TU. In the migrated object the jump-table address is loaded through
 * `%hi/%lo(jtbl_800105A0)` - relocations against a NAMED GLOBAL symbol, whose
 * immediate fields engine/score.py does NOT mask - so the reference stream reads
 * `lui a3,0x0` / `addiu a3,a3,0`, while any C build reaches the same table
 * through a `.rodata` SECTION-relative reloc that score.py DOES mask to
 * `@.rodata`. Two insns of pure false distance, hence 15 -> 17. This is NOT the
 * [[sandbox-lo16-text-addend-false-distance]] artifact the s14 entry suspected:
 * score.py has masked section-relative HI16/LO16 addends since that memory was
 * written (engine/score.py:63 _SECTION_ADDEND_RELOCS). Rebuilding the reference
 * from pristine HEAD (`& tools/wteng.ps1 main build`, SHA1 == oracle) and
 * re-measuring this file gives score 15, 68 == 68, rules_dropped 0
 * (tmp/grind/func_800324D0/s15/sandbox_candidate_freshref.log), and the 15 diffs
 * are a pure v1<->a2 rename with zero ordering or count difference
 * (s15/insn_diff_candidate_freshref.log).
 * STANDING PROCEDURE for every future session on this function: if the floor
 * reads anything other than 15, run `& tools/wteng.ps1 main build` from a
 * pristine `git checkout -- src/code6cac_b.c` FIRST, then apply this body and
 * sandbox. A stale build/src/<stem>.o silently poisons the absolute score.
 * Main still carries the legacy four-pin body: the owner-ruling-5 INCLUDE_ASM
 * migration is mechanically unreachable from a grind session (hypotheses.md s15
 * H36). To measure this candidate, replace that body with the one below. */
/* func_800324D0 — BEST FORM (s5 ledger entry, brief-session 4, 2026-08-20):
 * sandbox --disable all = 15, build_insns 68 == target 68. FIRST floor drop
 * since s1 (27 -> 15).
 *
 * Chassis: the s4 combined respelling (probe-A no-cmd-copy + V1 bare-switch +
 * V2 while-form + probe-B literal-0xFF — each measured flat 27 individually in
 * s2/s3, combination measured flat 27 this session) PLUS the load-bearing
 * staged loop-tail read: `c = *ptr` written as `cmd = *ptr; c = cmd;`.
 *
 * The staged read is a permuter find (campaign #2, output-105-1) vetted and
 * hand-measured: it borrows the EXISTING, currently-dead u32 `cmd` for the
 * tail stream byte. Family: staged-value-reused-variable
 * (.claude/rules/staged-value-reused-variable.md, SANCTIONED 2026-07-03) —
 * bounds check: (1) value real, consumed next line by `c = cmd`; (2) cmd
 * exists for a real job (payload command selector); (3) borrow provably safe
 * (cmd's arm value dead at the tail — it is re-derived from c at the next
 * arm entry; staged value not needed after cmd's next assignment); (4) FAKE
 * annotation in place at the site; (5) receipts = s1-s4 exhaustion + two dry
 * campaigns. SOTN PSX precedent: the `// fake reuse of i?` staged-load shape,
 * docs/reference/sotn-construct-index.md:51,81,92,97,109 (i = *scriptCur++
 * through an existing variable). CITATION QUESTION RESOLVED [s7, synthesis]:
 * the rule's six bounds are mechanism-silent; bound 4 requires naming the
 * ACTUAL pass (ours: global.c allocno census — the extra cmd set splits cmd
 * into head web 75 + arm web 85 and re-seeds find_reg); the sched.c
 * reference is descriptive Origin text, not a bound; the 2026-08-17
 * clarification (no-new-park-categories.md:193-214) establishes SOTN
 * acceptance is shape-based, and the SOTN exemplar is literally this shape.
 * Full argument: tmp/grind/func_800324D0/s6/probes_and_partition.md — quote
 * it in the submitting session's self-vet.
 *
 * u32 cmd is load-bearing for the staging (borrowing u8 val instead: 27;
 * u8 c for val's load: flat; placement `ptr++` between read and copy: 28/69).
 *
 * Residual 15 = exact 2-register swap: walker ours $6 / target $3($v1), both
 * cmd webs ours $3 / target $6($a2). find_reg ground truth (s4/findreg*.log):
 * order 75,76,85,72,74,73,91,86; 75 takes first-free 3. For $3 to survive to
 * the walker, 75 AND 76 AND 85 must all skip it => a conflicting allocno with
 * hard-reg-pref $3 => the walker itself (defs are self-increments + lw from
 * mem(pad): set_preference CANNOT plant — s2 proof unchanged) or an invented
 * overlapping pair (the BANNED base/ff family). Priority inversion (walker
 * allocated first WOULD yield the full target cascade with zero constructs —
 * verified against the exclusion sets) needs walker density ~5x: arithmetically
 * dead (s1). The wall is the SAME single sufficient condition as s1, one
 * construct short of it.
 */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u32 cmd;
    u8 val;

    ptr = *(u8 **)(pad + 0x58);
    pad[0xA1] = 0xFF;
    pad[0xA3] = 0xFF;
    pad[0xA2] = 0xFF;
    pad[0xA4] = 0xFF;
    pad[0xAA] = 0;
    pad[0xA7] = 0;
    pad[0xA8] = 0;
    pad[0xA5] = 0;
    pad[0xA6] = 0xFF;
    pad[0xAB] = 0xFF;
    pad[0xAC] = 0xFF;

    c = ptr[4];
    ptr += 5;
    while (c != 0) {
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            cmd = c - 0x80;
            val = *ptr;
            ptr++;
            {
                switch (cmd) {
                    case 0: pad[0xA1] = val; break;
                    case 1: pad[0xA3] = val; break;
                    case 2: pad[0xA7] = val; break;
                    case 3: pad[0xA8] = val; break;
                    case 4: pad[0xA9] = val; break;
                    case 5: pad[0xA5] = val; break;
                    case 6: pad[0xA6] = val; break;
                    case 7: pad[0xA2] = val; break;
                    case 8: pad[0xA4] = val; break;
                    case 9: pad[0xAA] = val; break;
                    case 10: pad[0xAB] = val; break;
                    case 11: pad[0xAC] = val; break;
                }
            }
        }
        /* FAKE: loop-tail stream byte staged through the currently-dead cmd
         * (c = *ptr written as cmd = *ptr; c = cmd), mechanism: global.c
         * allocno census - the extra cmd set splits cmd into head/arm webs and
         * re-seeds find_reg (val/byte/holders land in target regs; floor 27->15,
         * 68/68), lever-exhaustion: memory/grind/func_800324D0/hypotheses.md
         * s1-s4 + two dry permuter campaigns (evidence.md s4/s5) */
        cmd = *ptr;
        c = cmd;
        ptr++;
    }
}
