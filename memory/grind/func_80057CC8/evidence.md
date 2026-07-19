# Evidence bank — func_80057CC8

- WIP rejected_form: {'form': 'explicit s32 offset local reused for both p computations', 'score': 4, 'reason': 'not a cheat, just worse — adds an insn vs the reassociated inline form'}

- WIP rejected_form: {'form': 'ternary/single-def prev_idx with explicit sidx local', 'score': 5, 'reason': 'not a cheat, worse than baseline-3'}

- WIP rejected_form: {'form': 'pointer arithmetic `table + (sidx<<1)` instead of byte-offset cast', 'score': 9, 'reason': 'not a cheat; the <<1 element-stride form changes codegen for the worse'}

- WIP rejected_form: {'form': 'prev_idx declared s32 with (u16) casts', 'score': 8, 'reason': 'not a cheat; worse'}

- WIP rejected_form: {'form': 'second p left as (table + offset) [HEAD form]', 'score': 9, 'reason': 'not a cheat; this is the un-reassociated baseline the candidate improves on'}

- WIP rejected_form: {'form': 'split-init-off-accumulation: `off = (((s32)(prev_idx<<16)>>16)<<2); off += (s32)table; p = (s16*)off;` (memory/wip/func_80057CC8/rejected/split-init-off-accumulation.c)', 'score': 0, 'reason': "REJECTED 2026-06-22 by user decision after layer-2 cheat-reviewer FAIL: (a) the broad split-init-accumulation sanction's SOTN evidence base did not survive verification (semantic-asymmetry misread of e_coffin.c, in-loop false-positive in dra/6BF64.c, semantic-purpose misread of e_bone_ark.c, naming-role failure of sp20), (b) the EXACT form was already rejected by layer-1 cheat-reviewer on 2026-06-16, and (c) the 2026-06-13 narrow sanction precedent (func_80049C24) may itself bear re-examination. Future workers MUST NOT re-attempt this form. The mechanism (refs-count bump on same-variable accumulation) is the kind of GCC-internals-only justification that the project's cheat-by-any-spelling posture forbids in the absence of clear SOTN-master evidence — which 2026-06-22 verification showed the project does not actually have for this specific construct."}

- == imported from memory/wip notes.md ==
# func_80057CC8 — WIP checkpoint (BLOCKED — continue search; not permanently parked)

## TL;DR (2026-06-22, post-adjudication)
Masked floor **9 -> 3** by reassociating both p-pointer adds from
`(table + offset)` to `(offset + table)` — pure C, no cheat. That is the
cleanest progress. The remaining 3-insn gap is a register-COALESCING decision
the masked sandbox cannot drive below 3, and the function additionally needs
its `asm("s3")` pin and all 7 regfix-subst rules to drop for COMPLETED-C.

**2026-06-22 ruling:** the split-init-off-accumulation form
(`off = (...); off += (s32)table;`) was proposed as a broad-family sanction
and **REJECTED** by user decision after layer-2 cheat-reviewer FAILed the
supporting SOTN-evidence base. Do NOT re-attempt that form. Card moves to
**blocked** (no-quit grind continues); not permanently parked.

## Resume steps
1. Apply `candidate.c` to `src/text1b.c` (replaces func_80057CC8, ~line 11840).
2. Confirm floor: `& tools/wteng.ps1 <id> sandbox func_80057CC8 --disable all` -> 3.
3. Iterate on the coalescing gap from THERE — see "Avenues still open" below.
4. Do NOT re-derive any of the rejected forms (see meta.json:rejected_forms).

## The remaining gap (the 3)
Target first `p`:  `addu v0,v0,a2` (offset+table, dest = the offset reg v0),
then `lh a0,0(v0)` / `lh a1,2(v0)`.
Ours:              `addu v1,v0,a2` (allocates a NEW reg v1), `lh ...0(v1)` etc.
=> GCC does not coalesce the offset temp into the pointer dest for the FIRST p.
The SECOND p coalesces correctly (v1->v1). The only structural asymmetry is
the `if ((s16)prev_idx < 0) { prev_idx = arg0[3]-1; }` reload: prev_idx has a
two-def (phi) live range, next_idx (set in a block) has a single def.

Also masked-out but real for the full SHA1: target holds prev_idx in `a0` and
table in `a2`; ours has them swapped. And `next_idx` must land in `s3`
naturally (the pin currently forces it).

## Ruled out (rejected_forms in meta.json — do not re-derive)
- explicit `s32 off`/`sidx` local reused for both p adds — floor 4
- ternary / single-def prev_idx with explicit sidx local — floor 5
- pointer arithmetic `table + (sidx<<1)` — floor 9
- prev_idx declared `s32` with `(u16)` casts — floor 8
- HEAD form (second p un-reassociated) — floor 9
- **`off = X; off += (s32)table;` split-init-accumulation — REJECTED**
  (rejected/split-init-off-accumulation.c). Was proposed for a broad-family
  sanction 2026-06-22; layer-2 cheat-reviewer FAILed the supporting SOTN
  evidence (mischaracterizations of e_coffin.c, dra/6BF64.c, e_bone_ark.c,
  sp20 — see meta.json:reviewer.evidence). User decision: NOT sanctioned;
  the EXACT form was already rejected by layer-1 on 2026-06-16, and
  retroactive sanction would be the self-sanctioning antipattern that
  [[review-discipline-before-commit]] was codified to prevent.

## Avenues still open (next session)
- `-da` / ALLOCDBG dump from the candidate-floor-3 baseline to see WHY the
  first p's offset temp is not coalesced into the pointer dest while the
  second's is. The two-def vs single-def live-range asymmetry is the
  prime suspect.
- Force prev_idx to a single def so its live range matches next_idx's
  shape — without contriving named-intermediate aliases. Look at branchless
  forms: `prev_idx = ((s16)tmp < 0) ? arg0[3] - 1 : tmp;` and the family of
  conditional-init shapes; verify these don't trigger the cheat-by-spelling
  catalog (e.g. ternary that compiles to a cmov sequence is OK; ternary that
  forces an unused-arm computation may not be).
- Targeted permuter run from candidate.c base (CAVEAT: text1b.c is the
  highest-cascade file; ~50% apply-failure rate per
  docs/PERMUTER_PIPELINE.md and permuter-closability-evaluated memory).
- Investigate whether one of the COMPLETED-C sibling functions in the same
  text1b cluster (`func_8006133C`, `func_800613C8`, `func_80061454`,
  `func_800614E0` — pointer-walk + mask-last family) carries a structural
  asymmetry-fix lever that could transfer to this function's prev_idx case.

## Why blocked (not matched, not park)
COMPLETED-C needs 0 rules + 0 pins + SHA1==oracle. The 7 regfix-subst rules
rename exactly the registers this gap is about (a0<->a2, dest v0<->v1) and
the s3 pin forces next_idx; the explored pure-C forms do not make GCC
produce that allocation. Genuine register-allocation plateau. Not a
canonical-asm candidate (no hand-coded signals); not a sanctioned
pure-C-impossible plateau (the gap is RA-coalescing, which is
pure-C-reachable in principle — we just haven't found the lever).

Per user 2026-06-22: keep working it; not permanently parked.


- [s1] Floor 3 replays with candidate.c edits in src/text1b.c (offset+table form, no s3 pin) — sandbox --disable all reports score=3, rules_dropped=7, cheat_asm_stripped=395.

- [s1] Canonical gate: verdict=C, distance=3 (pure-C target, no ASM routing).

- [s1] Objdump of built .o localizes the 3 remaining diffs to the FIRST p's pointer add: our `addu v1,v0,a2` (b798) vs target `addu v0,v0,a2` (80057D54); cascades to `lh a0,0(v1)` / `lh a1,2(v1)` (b7ac/b7b0) vs target `lh a0,0(v0)` / `lh a1,2(v0)` (80057D68/80057D6C). The SECOND p already coalesces correctly in both (addu v1,v1,a0 at b7cc / 80057D88).

- [s1] Structural asymmetry (from ledger, unmeasured): prev_idx has a two-def live range from `if ((s16)prev_idx<0) { prev_idx = arg0[3]-1; }`; next_idx is set inside a block { ... = tmp; if(..) ...=0; } but its uses reach through a phi too — however next_idx's use flows through s3 (in target/pin); the pin masks this. Even with the pin removed, only the FIRST-p addu coalesces poorly, suggesting the phi shape at the prev_idx use-site is the differentiator.

- [s1] Rejected forms (do not re-derive): block-scope-alias-p1, duplicate-address-expr-pseudo-inline, separate-p1-p2-function-scope-aliases, split-init-off-accumulation, split-lh-px0-px1-locals. Also from evidence.md: explicit s32 off/sidx (floor=4), ternary+explicit sidx (floor=5), pointer-arith table+(sidx<<1) (floor=9), prev_idx s32 with (u16) casts (floor=8), un-reassociated HEAD form (floor=9).

- [s2] Candidate.c applied to src/text1b.c line 11837 (register asm("s3") pin removed, offset+table reassociation for both p adds). sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 — replays s1 finding cleanly on current main HEAD.

- [s2] Objdump of candidate baseline localizes the 3-insn gap unchanged from s1: b798 `addu v1,v0,a2` (ours) vs 80057D54 `addu v0,v0,a2` (target); cascading to lh base regs at b7ac/b7b0 (v1) vs 80057D68/80057D6C (v0). Second p addu coalesces correctly in both (b7cc v1,v1,a0 vs 80057D88 v1,v1,a0).

- [s2] Two-def-phi asymmetry is NOT the discriminator. Mirroring next_idx's block-scoped `s32 tmp; = tmp; if(..) = 0;` shape onto prev_idx did NOT fix p1 coalescing — both variables are two-def yet only p1 addu misses. The real discriminator is deeper (probably shifted-offset pseudo lifetime across the next_idx bnez delay-slot scheduling window, or a reg_n_refs priority tiebreaker in global.c on the p1 pseudo).

- [s2] Statement re-ordering (moving prev_idx def, swapping IF blocks, hoisting p1 add) all failed. Target's shape is delay-slot-schedule-critical: swap made it floor 31; hoist p1 made it floor 12. The (prev_idx if-reload) -> (next_idx block with delay-slot fillable bnez) -> (p1 add reading a2=table and a0=prev_idx from the reload's phi) sequence is FIXED at the C level; the RA lever is inside that structural window.

- [s2] Type width (u16 vs s16) of prev_idx and shift-vs-cast spelling of the sign-extend chain are both folded identically by GCC — combine.c reduces them to the same pattern. Not levers.

- [s2] Sibling matched functions in text1b.c (func_8006133C, 800613C8, 80061454, 800614E0) are structurally DIFFERENT — they are s32* pointer-walk-with-post-increment (`D_800F1140 = *p++; ...`) with no phi-shape index. No transferable lever.

- [s3] [s3] Candidate.c applied to src/text1b.c line 11837 replays cleanly on current main HEAD: sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395.

- [s3] [s3] Statement-order structural window is FULLY CONSTRAINED at the C level: any reordering that moves the next_idx block, scale computation, or table use out of its current position pushes the score sharply UP (32/35/41). The prev_idx-if -> next_idx-block -> p1-addu -> jal1 -> p2-addu -> jal2 sequence is a fixed point.

- [s3] [s3] Named-intermediate scheduling levers (dx/dy locals around calls, moved table def, cx/cy type-narrow) are all folded by GCC's combine/CSE to bytes identical to the inline candidate form. These structural axes are not levers.

- [s3] [s3] The s2 finding that the block-scope-alias-p1 form scores 0 + SHA1-matches but was rejected by cheat-reviewer as live-range-shaping alias (rejected/block-scope-alias-p1.c) further narrows the search: the RA-coalescing gap CAN be closed by shaping the p1 pseudo's lexical lifetime, but that spelling is a cheat-by-any-spelling. A legitimate structural form must arise from a semantically-meaningful C restructure, not lifetime shaping.

- [s3] [s3] Combined s1+s2+s3 kill-count: 12 hypotheses KILLED across statement order, type width, declaration order, block scoping, and named-intermediate. Cheap structural axes are exhausted; remaining probe surface is (a) forensic cc1 -da greg dump to name the pseudo/copy-pref, (b) FAKE duplicated-statement-into-arms with mandatory cheat-reviewer, (c) directed permuter from the candidate baseline.

- [s4] [s4] Applied candidate.c to src/text1b.c line 11837 (register asm("s3") pin removed, offset+table reassociation for both p adds). sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 — replays s1-s3 finding cleanly.

- [s4] [s4] Built clean per-function permuter workspace: base_full.c (12070 lines) via cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips ... src/text1b.c; compile.sh does cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 | prologue_fix | maspsx | fix_lwl | sed align3->2 | multu_pad, then awk-extracts .ent func_80057CC8 ... .end block, and assembles alone. target.o built from asm/funcs/func_80057CC8.s + prelude_r3k (prelude.inc minus .set gp=64). Sanity: base=target=111 insns, diff signature matches ledger exactly (v1 vs v0 addu + 2 cascading lh).

- [s4] [s4] Permuter campaign s4-cand-baseline (pid 426, launched 21:04:38Z): -j 6 --stop-on-zero. Reached 411 iterations at harvest, ~5780+ (last-seen output-8627-1 iter index); 1039 output-* dirs generated in ~25 min wall.

- [s4] [s4] Score-0 close: `long new_var2 = ((((s32)(prev_idx<<16))>>16)<<2) + (s32)table; p = (s16*)new_var2;` — REJECTED as pointer-address alias for lifetime shaping, same class as rejected/block-scope-alias-p1.c. Saved as rejected/permuter-long-new_var2-p1-alias.c.

- [s4] [s4] Score-15 family (best non-cheat find): pure declaration-order reorder (e.g. move `s32 scale;` decl after `s16 *table;`). No score improvement over candidate floor 3; not a lever.

- [s4] [s4] Score-25/35/78 family: `T *new_var2 = ...; *new_var2` / `u16 new_var2 = cx; ...new_var2 in place of cx` — pointer/scalar alias variants of the score-0 cheat class. Each identified and vetted; none legitimate.

- [s4] [s4] Score-30: `scale = (s32)table; cx = *((u16*)((scale + arg1*4) + 0));` — variable-reuse for codegen. Under [[named-local-fake-exception]] carve-out would require FAKE annotation + layer-2 review AND only reached masked-15 not floor-0; not competitive with candidate.

- [s4] [s4] Score-45: `table++; table--;` no-op RA nudge — clear cheat, plainly rejected.

- [s4] [s4] Score-40: statement-split of `ang_mid = ((s32)(ang_next-ang_prev)/2)+ang_prev;` — no score improvement; not a lever.

- [s4] [s4] Frontier is now: (a) forensic cc1 -da greg dump to name the p1 pseudo/copy-pref (still unrun this session — mandated modality was permuter, not forensics), (b) FAKE-annotated duplicated-statement-into-arms per [[duplicated-statement-into-arms]] applied to the p1 pointer add's arms (also unrun), (c) two of the previously-live frontier items (permuter + one probe of the greg-dump) are now discharged.

- [s5] [s5] Applied candidate.c to src/text1b.c line 11837 (register asm('s3') pin removed, offset+table reassociation for both p adds). sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 -- replays s1-s4 baseline cleanly on current main HEAD.

- [s5] [s5] Cloned tmp/grind/func_80057CC8/s4/perm workspace to s5, reset campaign state (removed prior output-*, campaign.log, campaign_meta.json), verified base.c==base_full.c (both 12070 lines, candidate form).

- [s5] [s5] Chassis 1 (s5-directed): PERM_GENERAL over (prev_idx if-reload, p1 addu, p2 addu). 24-iteration deterministic enumeration; base_score=15 permuter-scored; NO improvement below baseline. All 3-alt x 4-alt x 2-alt combinations scored 15 or 25. Enumerated space is EXHAUSTED and DOES NOT contain a score-0 close. Campaign harvested + stopped clean; procs_killed=0, pid_alive_at_harvest=false (self-exited).

- [s5] [s5] Chassis 2 (s5-random+directed): PERM_RANDOMIZE wrapping the same PERM_GENERAL alternatives. 237 iterations in 134s wall; one score-0 close at iter 237. Score progression: base 15 -> repeatedly 15/25 -> transient explosions to 300-5042 during aggressive mutations -> 0. Campaign harvested + stopped clean.

- [s5] [s5] Score-0 form vetted against cheat-by-any-spelling per [[no-new-park-categories]] + [[review-discipline-before-commit]]: rejected in-session as pointer-alias holder = same class as block-scope-alias-p1.c (dual layer FAIL) and permuter-long-new_var2-p1-alias.c (s4 rejected). Saved as rejected/permuter-s16-new_var2-p1-alias.c with cheat-lens annotations.

- [s5] [s5] Combined s4+s5 permuter kill: TWO independent chassis (undirected random on candidate baseline; directed PERM_GENERAL enumeration; directed PERM_GENERAL + PERM_RANDOMIZE random overlay) each converge on THE SAME conclusion -- the only score-0 closing form for the p1 addu RA-coalescing gap is a pointer/scalar alias holder whose ONLY purpose is to shift the pseudo lifetime. This is the cheat-by-any-spelling class that [[no-new-park-categories]] enumerates and rejects. The remaining frontier item 'directed permuter finds a novel legitimate lever' is now KILLED by measurement across two structurally distinct directed chassis.

- [s5] [s5] Kill-count update: cumulative KILLED hypotheses across s1-s5 = 14 (s1 CONFIRMED baseline; s2 5 KILLED; s3 7 KILLED; s4 2 KILLED; s5 2 KILLED). Cheap structural axes (statement order, type width, declaration order, block scoping, named-intermediate, directed spelling alternatives, directed+random overlay) all exhausted at the C level. Remaining probe surface (unchanged from s4): (a) forensic cc1 -da greg dump to name the p1 pseudo's copy-pref biasing pseudo, (b) FAKE-annotated duplicated-statement-into-arms per [[duplicated-statement-into-arms]] applied to p1's pointer-add duplicated inside both prev_idx reload arms.

- [s6] Applied memory/grind/func_80057CC8/candidate.c to src/text1b.c line 11837; sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 (unchanged from s1-s5 baseline).

- [s6] Preprocessed src/text1b.c with build's exact CPP_FLAGS/CPP_DEFS (mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C).

- [s6] Ran cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -da on the .i and extracted func_80057CC8 blocks from every per-pass dump (rtl/jump/cse/loop/cse2/flow/combine/sched/lreg/greg/jump2/sched2/dbr).

- [s6] greg dump header names 16 global pseudos to allocate (79 86 78 168 73 183 80 72 76 77 88 164 84 87 74 75). Only 4 have copy-preferences: 73 pref 5, 86 pref 3, plus 79 and 78 (via other insns).

- [s6] Pseudo 86 = C variable p (verified: both insn 89 p1 addu dest and insn 124 p2 addu dest are (reg/v:SI 86) in lreg; C uses ONE variable p).

- [s6] Pseudo 86 conflict list (greg): ';; 86 conflicts: 72 74 75 77 84 86 87 2 4 16 17 29' — includes hard reg 2 (v0). Preferences: ';; 86 preferences: 3' — includes hard reg 3 (v1) only.

- [s6] Pseudo 86 disposition: 86 in 3 (allocated to v1). Both p1 addu (insn 89) and p2 addu (insn 124) become `addu $v1, ..., ...`.

- [s6] Insn 115 (and:SI reg/v:SI 78 = reg:SI 2 v0 & 0xFFF, producing ang_prev) is at linked-list position (114,117) in rtl/jump/cse/loop/cse2/flow/combine dumps but at (145,147) in sched/lreg/greg dumps. sched1 is the exclusive reorderer.

- [s6] The hoist places insn 115 immediately before call_147 (getEnemyCharId call2), fitting in the scheduling window between the last p2-support insn (145) and the second call. This extends v0's live range across insn 124 (p2 addu, 86 redef), producing the 86-vs-2 conflict.

- [s6] Pseudo 129 (shifted next_idx offset for p2) is local-allocated to hard reg 3 (v1). Its only use is insn 124 as p2 addu source, dying there. expand_preferences propagates the v1 placement into pseudo 86's hard_reg_copy_preferences via the addsi3 SET operand relation.

- [s6] Target's p1 addu is `addu $v0, $v0, $a2` — target's p1 pointer occupies v0, which is impossible for pseudo 86 given the above conflict. Target must use a DIFFERENT pseudo for p1's dest than for p2's dest, implying target's C source declares two variables (or otherwise ensures the RTL keeps p1's def in a pseudo distinct from p2's).

- [s6] All previously-tried 'declare two C variables' spellings (block-scope-alias-p1, separate-p1-p2-function-scope-aliases, permuter-long-new_var2-p1-alias, permuter-s16-new_var2-p1-alias, ternary-direct-bind-no-local, split-init-off-accumulation) reached sandbox 0 but were rejected as pointer-alias holders whose sole purpose is lifetime shaping (cheat-by-any-spelling per [[no-new-park-categories]] and layer-1/2 reviewer FAILs).

- [s7] s7 baseline replay: candidate.c applied to src/text1b.c line 11837; sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395.

- [s7] s7 dumps regenerated to tmp/grind/func_80057CC8/s7/ via dump.sh + extract.sh (cc1 -da over candidate baseline).

- [s7] greg header on candidate baseline: 16 global pseudos to allocate; 86 conflicts: 72 74 75 77 84 86 87 2 4 16 17 29; 86 preferences: 3; 86 in 3.

- [s7] Pseudo 86 is /v-marked (`reg/v:SI 86`) across every dump — user variable, DECL_RTL of C local `p`.

- [s7] Both p1 SET (insn 89, plus 112 + 88) and p2 SET (insn 124, plus 129 + 130) target pseudo 86; two independent def-use chains within BB4.

- [s7] lreg summary: 'Register 86 used 6 times across 10 insns in block 4; dies in 2 places; GR_REGS or none; pointer.' — single-BB but multi-def, promoted to global-alloc pool.

- [s7] p1 chain dispositions: 110 in v0 (subreg-of-prev_idx << 16), 112 in v0 (110>>14 via quantity reuse), 88 in a2 (table pointer). p1's operand hard regs (v0, a2) DO NOT appear in 86's preference set.

- [s7] p2 chain dispositions: 127 in v1 (subreg-of-next_idx << 16), 129 in v1 (127>>14 via quantity reuse), 130 in a0 (*(arg0+4) reload). Only 129's v1 propagates into 86's copy-prefs — 130's a0 does NOT.

- [s7] Insn 115 (and:SI reg 78 = v0 & 0xFFF) is at linked-list position (114,117) in every pre-sched dump; sched dump shows it moved to (145,147), one position before call2 (insn 147/143). sched1 is the sole reorderer (unchanged from s6, reconfirmed on candidate baseline).

- [s7] Consumer-of-ang_prev-early axis is a NECESSARY-but-NOT-sufficient lever: even without the hoist, pseudo 86's pref {3} still steers global-alloc away from v0.

- [s8] [s8] Applied candidate.c to src/text1b.c line 11837 (register asm('s3') pin removed, offset+table reassociation for both p adds). Sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395 — replays s1-s7 baseline cleanly on current main HEAD.

- [s8] [s8] Inline-both-call-sites form (no `p` local declared, 4x address-expression duplication for [0]/[1] across both calls): sandbox score=0. Rejected as cheat-by-any-spelling per [[no-new-park-categories]] — identical class to rejected/duplicate-address-expr-pseudo-inline.c (2026-06-15). Saved as rejected/inline-both-call-sites-no-p-local.c with cheat-lens annotations.

- [s8] [s8] p2 PLUS operand swap alone (write `table + offset` at p2, keep `offset + table` at p1): sandbox score=9 (regression matching HEAD-form's un-reassociated distance). Confirms source-level operand order at p2 controls reassociation reach, NOT expand_preferences source-slot selection.

- [s8] [s8] p1 PLUS operand swap alone (write `table + offset` at p1, keep `offset + table` at p2): sandbox score=3 (byte-neutral). p1's operand ordering is fold-equivalent when p2 is offset+table.

- [s8] [s8] Combined s1-s8 kill-count: 16 hypotheses KILLED across statement order, type width, declaration order, block scoping, named-intermediate, directed spelling alternatives, directed+random permuter overlay, consumer-of-ang_prev-early axis, inline-both-call-sites, and p2-operand-swap. Both s7-named open frontier axes are now measured dead.

- [s8] [s8] Remaining un-attempted frontier item (unchanged from s5/s6/s7): FAKE-annotated duplicated-statement-into-arms per [[duplicated-statement-into-arms]] applied to the p1 pointer add duplicated inside both prev_idx reload arms. Requires structural modality (draft duplicated-arms C, verify byte-neutrality via objdump vs candidate.c, sandbox; if floor drops with byte-neutral duplication and arms merge identically post-cross-jump, invoke layer-1 + layer-2 cheat-reviewer for FAKE-annotation policy compliance).

- [s9] s9 baseline replay: candidate.c applied to src/text1b.c line 11837; sandbox --disable all reports score=3, target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395.

- [s9] s9 rederive kill #1 (walking-pointer): `p = base; p += off*2;` for both slots => score=7 (regression). GCC 2.7.2 does not fold compound-assign back to single-add; the two-SET form breaks RA coalescing further. Saved rejected/rederive-p-plus-equals-walk.c.

- [s9] s9 rederive kill #2 (2-iter for-loop with idx[2]/ang[2] arrays): for(i=0;i<2;i++) block replacing both p compute + call sites => score=77, build_insns=118. GCC 2.7.2 -O2 does not unroll; the loop compiles as an actual branch+induction loop, +7 insns beyond target. Saved rejected/rederive-for-loop-idx-array.c.

- [s9] Combined s1-s9 kill-count: 18 hypotheses KILLED across statement order, type width, declaration order, block scoping, named-intermediate, directed spelling alternatives, directed+random permuter overlay, consumer-of-ang_prev-early, inline-both-call-sites, p2-operand-swap, and s9's walking-pointer and 2-iter-for-loop rederives. Both structural-rederive collapse-forms (walking-pointer, unrolled-loop-collapse) are now measured dead.

- [s9] Remaining un-attempted frontier item (unchanged from s5-s8): FAKE-annotated duplicated-statement-into-arms per [[duplicated-statement-into-arms]] applied to the p1 pointer add duplicated inside both prev_idx reload arms. Requires STRUCTURAL modality; NOT a rederive-modality probe. This session was mandated rederive.

- [s9] Both Judge-listed 'open axes' were already measured dead in s8: axis (1) inline-both-call-sites eliminates the bound p local at score=0 but rejected as cheat-by-any-spelling (character-for-character 30+-char address-expression duplication); axis (2) p2 PLUS operand swap regressed 3->9. Neither is a live rederive axis for this session.

- [s9] Sibling functions in text1b.c (func_8006133C, 800613C8, 80061454, 800614E0) confirmed structurally different in s2 (pointer-walk with post-increment, no phi-index); no transferable rederive lever.

- [s10] SYNTHESIS session (no new sandbox measurements). Merged the s1-s9 ledger; see tmp/grind/func_80057CC8/s10/synthesis.md for the full merged attack.

- [s10] Impossibility surface named: (i) s7 CONFIRMED that ONE C-variable `p` cannot reach target's p1=v0 (pseudo 86's {3} pref + v0 conflict both trace to single-pseudo properties); (ii) Judge binding constraint closes ALL two-C-local spellings; (iii) both Judge-suggested axes measured dead in s8 (inline-both-calls -> cheat class at 0; p2-operand-swap -> regression to 9). Remaining sanctioned axes are structural F1 (duplicated-statement-into-arms with cross-jump byte-merge prerequisite), rederive F2 (arg0 struct header-type correction via caller sweep), rederive F3 (Kengo transplant, accessibility unproven).

- [s10] Frontier reset to 3 items (F1 primary structural, F2 secondary rederive, F3 tertiary rederive). F1 mechanism note: reg_n_refs bump on pseudo 86 via source-level duplication enters global.c allocno priority calc; but expand_preferences propagation to 86's copy-pref set is computed from static SET-operand analysis BEFORE realized placements, so a priority-order bump alone likely does NOT alter 86's {3} pref. F1 must actually flip either the {3} pref origin or the v0 conflict to move; refs-count-only is a weak lever unless combined with a schedule shift. Prerequisite for F1: objdump vs candidate must show cross-jump merged the arms byte-identically; if not, SOTN sanction prerequisite fails.

- [s10] Kill-order for the next 1-3 sessions: s11 structural F1 (draft duplicated-arms, objdump-verify byte-neutrality, sandbox; on close invoke layer-1+layer-2 review); s12 rederive F2 (grep callers, evaluate struct evidence, KILL if no consistency); s13 rederive F3 (Kengo accessibility check, KILL if unfindable). If all three KILLED with measurements, function has exhausted sanctioned pure-C axes and OWNER-ESCALATION becomes correct next step — NOT this session (owner-gated requires pre-filed docs/grind/decisions.md entry, which does not yet exist for func_80057CC8).

- [s10] s7 CONFIRMED: pseudo 86 = DECL_RTL(p) with both p1 (insn 89) and p2 (insn 124) SETs targeting the SAME pseudo 86; /v-marked in every dump.

- [s10] s6/s7 CONFIRMED: 86's copy-pref = {3}, originating ONLY from pseudo 129 (v1, shifted next_idx offset) propagating via expand_preferences at insn 124's addsi3. p1 operands (112 v0, 88 a2) and p2 other operand (130 a0) do NOT propagate.

- [s10] s6 CONFIRMED: 86 conflicts with v0 because sched1 hoists insn 115 (and $v0,$v0,0xFFF finalizing ang_prev) from (114,117) to (145,147), extending v0 live across insn 124.

- [s10] s7 EXPLICIT COROLLARY: any pure-C form that keeps `p` as ONE C variable cannot reach target's p1=v0 -- 86 lands in v1 by positive pref {3} and v0 is conflict-blocked.

- [s10] Judge s10 binding constraint: no shared-pointer split into two source-level locals under any spelling (semantic naming, numeric suffix, block-scope, function-scope, or FAKE-annotated). Verified against the 17-form rejected bank.

- [s10] s8 measured both Judge-suggested axes dead: inline-both-call-sites -> score 0 but cheat-class (30+ char address-expr duplicated 4x, identical class to duplicate-address-expr-pseudo-inline); p2 PLUS operand swap -> score 3->9 regression.

- [s10] Combined s1-s9 KILLED count = 18 hypotheses spanning statement order, type width, declaration order, block scoping, named-intermediate, directed permuter + directed+random permuter overlay, consumer-of-ang_prev-early, inline-both-calls, p2-operand-swap, walking-pointer rederive, 2-iter-for-loop rederive.

- [s10] The three remaining unmeasured sanctioned axes are: F1 duplicated-statement-into-arms (structural), F2 arg0 header-type correction (rederive, requires caller sweep), F3 Kengo transplant (rederive, requires corpus access).

- [s10] F1 SOTN-sanction prerequisite: cross-jump find_cross_jump must byte-merge both arms; verify via objdump vs candidate.c BEFORE claiming closure.

- [s11] s11 baseline replay: candidate.c form on src/text1b.c line 11837 measures sandbox --disable all = 3 (target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395).

- [s11] F1 variant A (duplicate p1 into prev_idx-if arms): score 3 -> 12. Matches s3's hoist-p1-before-nextidx-block kill; cross-jump does NOT merge byte-neutrally when p1 is moved out of its post-next_idx scheduling window.

- [s11] F1 variant B (duplicate p1 into next_idx if/else arms, preserving position): score 3 -> 8. Rewriting the next_idx default+override into if/else disturbs sched1's delay-slot fill; the duplicated p1 does not compensate.

- [s11] F1 variant C (duplicate table load into prev_idx-if arms per split-read pattern): score 3 -> 3 (byte-neutral). GCC's cse1 pass merges the redundant reloads before global-alloc; no reg_n_refs bump reaches pseudo 86.

- [s11] F1 mechanism is DEAD by measurement across 3 variants: (a) any duplication that moves p1 out of its scheduling window regresses (matches prior hoist kill); (b) duplicating around the natural p1 position requires disturbing the next_idx block shape, which regresses; (c) duplicating a non-p statement gets CSE-folded before RA. The reg_n_refs priority-lift mechanism cannot be realized on pseudo 86 for this function without regression.

- [s11] Total s1-s11 KILLED hypotheses: 21 (s1-s10=18 + s11 F1 variants A/B/C = 3 more).

- [s11] Structural modality is now exhaustively measured: statement order, type width, declaration order, block scoping, named-intermediate, directed spelling alternatives, directed+random permuter overlay, consumer-of-ang_prev-early, inline-both-call-sites, p2-operand-swap, walking-pointer rederive, 2-iter-for-loop rederive, and the F1 duplicated-arms family (3 variants). All C-level structural axes accessible at zero policy cost are dead.

- [s12] s12 baseline replay: candidate.c on src/text1b.c line 11837 measures sandbox --disable all = 3 (target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395).

- [s12] s12 caller sweep: func_80057CC8 has ZERO C callers — only asm callers in asm/funcs/func_80057E84.s (three call sites at 80057FCC, 80057FF8, 80058230; all pass $s1 as arg0 where $s1 = a1_outer + 8*index, an 8-byte struct table).

- [s12] s12 arg0 access pattern in func_80057CC8: byte 0 (unused in this fn), byte 2 as u8 (scale_units, `arg0[2]*40`), byte 3 as u8 (count, `arg0[3]` boundary tests), offset 4 as s16* (table, `*(s16**)(arg0+4)`). Consistent with an 8-byte ArenaHdr struct { u8 flags; u8 _p1; u8 scale_units; u8 count; s16 *table; }.

- [s12] s12 F2 struct-typed rewrite: sandbox --disable all = 3 (unchanged from candidate baseline); target=build=111 insns. Byte-neutral: struct member access `arg0->table` folds to identical `lw $rD, 0x4(arg0)` as the byte-cast form; `arg0->count` folds to identical `lbu $rD, 0x3(arg0)` as `arg0[3]`. GCC 2.7.2 combine.c collapses both forms to the same RTL substitution surface at insn 124.

- [s12] s12 sanction-check: [[header-type-correction-from-use-sites]] four-prong requires (a) grep-consistent use sites with at least one signed-specific — func_80057CC8 has ONE C use site and ZERO other C callers; prong is trivially satisfied but produces no evidence. Prong (c) requires a single extern edit — there is no shared header extern for this parameter type. The rule was designed for globals, not for parameter types on a function with only asm callers. Sanction fit fails independent of the byte-neutrality kill.

- [s12] Combined s1-s12 KILLED hypotheses: 22 (s1-s10 = 18 [statement order, type width, declaration order, block scoping, named-intermediate, directed permuter, directed+random permuter overlay, consumer-of-ang_prev-early, inline-both-call-sites, p2-operand-swap, walking-pointer rederive, 2-iter-for-loop rederive]; s11 = 3 [F1 dup-arms A/B/C]; s12 = 1 [F2 struct-typed arg0]).

- [s12] F1 duplicated-statement-into-arms (s11) and F2 header-type-correction (s12) both measured DEAD. Only F3 (Kengo transplant, corpus-accessibility unproven) remains from the s10 synthesis frontier.

- [s12] s7 CONFIRMED single-pseudo impossibility (pseudo 86 = DECL_RTL of `p`) still binding: any single-C-variable `p` binding cannot reach target's p1=v0 (86's {3} pref + v0 conflict). Judge's binding constraint still forbids all two-C-local spellings across the 20-form rejected bank (now +1 with the s12 struct form).

- [s13] s13 baseline replay: candidate.c on src/text1b.c line 11837 measures sandbox --disable all = 3 (target_insns=111, build_insns=111, rules_dropped=7, cheat_asm_stripped=395).

- [s13] s13 permuter campaign s13-downstream-ternary: 199 iters, elapsed_s=149.7, base_score=15 (permuter-scored), one score-0 find at 55.5s post-launch. Harvest reported: finds_total=1, finds_new=1, best_new_score=0, procs_killed=0 (self-exited on --stop-on-zero).

- [s13] The score-0 close is a NOVEL sub-form of the alias-holder cheat family: prior permuter finds (s4 permuter-long-new_var2-p1-alias.c, s5 permuter-s16-new_var2-p1-alias.c) all aliased the p1 SET; s13's find aliases the p2 SET via a fresh s16* local `new_var2` binding `p = new_var2`. Same intent — shift pseudo 86's copy-pref origin — different side of the shared pointer.

- [s13] Cumulative permuter data across s4/s5/s13: THREE independent chassis (undirected random + directed PERM_GENERAL enumeration + directed PERM_GENERAL + PERM_RANDOMIZE, now + a fresh downstream-ternary chassis) each converge on the SAME conclusion — the only score-0 basin reachable by the permuter over the candidate-baseline structural window is a pointer-alias holder (either p1 or p2 side). No structural mutation of the ternary or *arg2 write order surfaced any legitimate score-0 close.

- [s13] Combined s1-s13 KILLED hypotheses count: 23 (s1-s10 = 18; s11 = 3 [F1 dup-arms A/B/C]; s12 = 1 [F2 struct-typed arg0]; s13 = 1 [downstream-ternary + p2-alias-basin permuter]).

- [s13] The s5 conclusion 'directed permuter finds a novel legitimate lever' is now re-KILLED by a novel chassis on a fresh axis (downstream-ternary, not previously permuted). Permuter modality is exhaustively measured dead for func_80057CC8: the only score-0 basin reachable by the permuter in ANY chassis explored so far is the pointer-alias-holder family, closed by both [[no-new-park-categories]] and the s10 Judge constraint.

- [s13] s7 CONFIRMED single-pseudo impossibility (pseudo 86 = /v-marked DECL_RTL of `p`) still binding: any single-C-variable `p` binding cannot reach target's p1=v0 (86's {3} pref + v0 conflict); s6/s7 forensics traced this to expand_preferences propagation via pseudo 129 (v1) at insn 124's addsi3 SET.

- [s13] Existing 2026-07-19 17:09 decisions.md entry for func_80057CC8 is a Judge FAIL ruling on the two-variable form, NOT an OWNER-ESCALATION with pending owner disposition. No OWNER-ESCALATION entry for this function exists in docs/grind/decisions.md; owner-gated result is not authorized this session.

- [s14] s14 baseline: candidate.c form applied (offset+table reassoc, no s3 pin, single `p` local) -> permuter workspace base_full.c=base.c. base_score=15 (permuter-scored) matches the s13 baseline exactly — no chassis-level drift.

- [s14] s14 chassis is structurally distinct from all 3 prior chassis: s4 undirected random on candidate baseline; s5-1 PERM_GENERAL over (prev_idx if-reload, p1 addu, p2 addu); s5-2 PERM_RANDOMIZE + PERM_GENERAL overlay of the same; s13 downstream-ternary (ang_mid ternary + *arg2 write order). s14 targets the SHIFT-EXPRESSION spelling of both p1 and p2 offsets, AND the p2 table-source spelling (`*(s16 **)(arg0+4)` vs cached `table`) — axes never previously permuted.

- [s14] Campaign telemetry: elapsed_s=168.5, iterations=19, finds_total=1, finds_new=1, best_new_score=0, procs_killed=0, pid_alive_at_harvest=false (self-exited on --stop-on-zero at iter 19). Score progression 1045, 1868, 15, 1471, 15, 550, 15, 1868, 1471, 15, 550, 4295, 703, 550, 550, 15, 15, 550, then 0.

- [s14] Close-form structural analysis: the ONLY C-level change between the score-15 base and the score-0 form is the introduction of `s16 *new_var2 = <p1 addr expr>; p = new_var2;` bridging pseudo 128 (or similar fresh anchor) into pseudo 86 via a distinct RTL SET — the exact `no shared-pointer split into two source-level locals` construct closed by the Judge s10 binding constraint and by the 4-form rejected bank (block-scope-alias-p1, permuter-long-new_var2-p1-alias, permuter-s16-new_var2-p1-alias, permuter-s16-new_var2-p2-alias).

- [s14] Cumulative permuter data across s4/s5-1/s5-2/s13/s14: FOUR independent chassis (undirected random + 4-alt directed enumeration + directed+random overlay + downstream-ternary + shift-spelling+table-source) each converge on the SAME conclusion — the only score-0 basin reachable by the permuter over the candidate-baseline structural window is a pointer-alias holder (p1 or p2 side, s16* or long*, either alias-only or through a fresh downstream variable). No structural mutation of the shift-expression spelling or the p2 table-source spelling surfaces any legitimate score-0 close.

- [s14] The mechanism-level reason is single-pseudo (from s7): pseudo 86 = DECL_RTL(p) is /v-marked and receives both p1 (insn 89) and p2 (insn 124) SETs. Its {3} preference originates from pseudo 129 (v1) at insn 124's addsi3 SET, and v0 is in its conflict set from sched1's insn-115 hoist. No spelling change to the p1 shift/mask, no cached-vs-reload swap on p2's table source, alters that copy-pref origin or the v0 conflict, because both depend on pseudo 129's local-alloc placement (unchanged by the surface-level respellings) and on sched1's hoist (unchanged by respelling anything before insn 115).

- [s14] Permuter modality is now KILLED across FOUR structurally distinct directed chassis. All prior directed chassis (s5-1, s5-2, s13, s14) plus the undirected s4 have converged on the pointer-alias-holder cheat family; no permuter axis is now proposed as un-tried without a rederive-level structural change that itself would have to clear the Judge constraint independently.

- [s15] Baseline: candidate.c applied to src/text1b.c line 11837 (s3 pin removed, offset+table reassoc). sandbox --disable all = 3; target_insns=111; build_insns=111; rules_dropped=7; cheat_asm_stripped=395.

- [s15] greg dump: ';; 86 conflicts: 72 74 75 77 84 86 87 2 4 16 17 29' includes hard reg 2 (v0). ';; 86 preferences: 3' includes only hard reg 3 (v1).

- [s15] greg dispositions: 86 in 3 (v1); 112 in 2 (v0); 88 in 6 (a2); 129 in 3 (v1); 130 in 4 (a0). Verified insn 89 rendered as (set (reg v1) (plus (reg v0) (reg a2))) and insn 124 as (set (reg v1) (plus (reg v1) (reg a0))) post-alloc.

- [s15] set_preference (global.c:1591): GET_RTX_FORMAT(GET_CODE(src))[0]=='e' branch walks src = XEXP(src, 0) and sets copy=0. Only hard_reg_preferences (not hard_reg_copy_preferences) receives the bit.

- [s15] dump_conflicts (global.c:1702-1748): the ';; N preferences:' line reports hard_reg_preferences[i], NOT hard_reg_copy_preferences[i]. s6/s7 phrasing 'copy-preference' was imprecise.

- [s15] global_alloc call order (global.c:504-551): global_conflicts -> AND_COMPL with eliminable_regset -> expand_preferences -> allocno_order sort -> prune_preferences -> dump_conflicts. Dump reflects post-prune state.

- [s15] find_reg (global.c:921-1140): pass-0 selects best_reg by iterating reg_alloc_order excluding `used` (which includes hard_reg_conflicts + regs_someone_prefers). Then checks hard_reg_copy_preferences first, then hard_reg_preferences, keeping best_reg if it matches or upgrading if a preferred reg in same class exists.

- [s15] MIPS reg_alloc_order: v0 (2) precedes v1 (3) among caller-save GPRs. Confirms that if both v0 and v1 are in 86's post-prune pref set, find_reg picks v0.

- [s15] hard_reg_copy_preferences[86] is empty: neither insn 89 nor 124 has a bare-REG SET_SRC; expand_preferences never fires for 86; set_preference's copy=1 branch never fires for 86.
