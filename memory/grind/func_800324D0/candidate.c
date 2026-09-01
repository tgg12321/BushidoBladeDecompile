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
