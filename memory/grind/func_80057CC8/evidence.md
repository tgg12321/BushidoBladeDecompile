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
