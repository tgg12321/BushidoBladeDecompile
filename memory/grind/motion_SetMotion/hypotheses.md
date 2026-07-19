# Hypothesis ledger — motion_SetMotion

## [s2] F1 (flagship): the committed build's RTL contains a concrete, honest feature at the -1 sites that blocks find_cross_jump, absent at the 13 sites, and it is transplantable.
- mechanism: sched2 list scheduling hoists case-10's `set s0<--1` (insn 374) up beside `s1<-0` (insn 358) because the block has 6 independent store insns of slack; the pre-jump insn becomes `sb D_800A3350` (insn 371), so find_cross_jump iter1 mismatches set-reg vs set-mem -> 0 counted matches -> no merge either direction (jump.c:2532 needs last1!=0; label bonus cannot rescue a zero-match pair). reorg's fill_simple_delay_slots then pulls the hoisted li back into the j's delay slot, restoring the byte-identical [j; delay li -1] twins.
- probe: Regenerated -da dumps of the committed TU first-hand (dump_jump2.sh -> committed.i.sched2/.jump2, 2026-07-17 17:18); s2b_pairs.py printed the full prev/next-chain neighborhoods of all four blocks; cross-checked final layout in committed.s (line 296-300: sb; j .L244; delay li $16,-1).
- result: Discriminator identified and verified end-to-end: case-8 = [LABEL 288; set -1 (291); j 293], case-10 = [set s1,0 (358); set s0,-1 (374) HOISTED; ...4 stores...; sb (371); j 376]. Half CONFIRMED, half KILLED: the feature is real and honest, but it is NOT transplantable — it requires >=1 independent block-local insn, and target fixes the 13 blocks at exactly [jtbl-label; j; delay li13] / [beq; delay li15; j; delay li13], so any slack insn adds bytes. Byte-count closes the transplant.
- verdict: CONFIRMED

## [s2] Arm re-association (set13 hoisted above the compare: `sel = 0xD; if (v0 == 10) sel = 0xF;`) or branch-sense swap (`if (v0 == 10) sel = 0xF; else sel = 0xD;`) changes the arm's jump2 suffix enough to block the merge while keeping target bytes.
- mechanism: If the pre-jump insn on the arm side were the condjump or a different set, iter1 would mismatch.
- probe: s2b_probe.py r1 and r3 vs m0 baseline (label-normalized cc1 asm diff).
- result: KILLED: both canonicalize to the SAME shape — branch sense flips to bne with li13 in its delay (target has beq + li15) AND the case-9/11 block still merges (361 insns vs m0's 363; 122 diff lines each). Doubly wrong.
- verdict: KILLED

## [s2] Type narrowing sel to s16 (+ honest 0xD) perturbs the set13 RTL mode enough to break rtx_renumbered_equal_p at iter1.
- mechanism: Narrow-typed local could change set modes or insert extension insns between set and jump.
- probe: s2b_probe.py r2.
- result: KILLED: 364 insns, 658 diff lines vs m0 — massive whole-function divergence (extension insns, layout shift), nowhere near byte-neutral.
- verdict: KILLED

## [s2] Block-local const split at case 9/11 (`{ s32 d13 = 0xD; sel = d13; }`) survives to jump2 as a distinct pattern.
- mechanism: A separate pseudo staging the constant could make the pre-jump insn a reg-reg move.
- probe: s2b_probe.py r4.
- result: KILLED: cse/const-prop folds it to the plain set13 before jump2; merged shape (361 insns), identical to the honest-0xD control.
- verdict: KILLED

## [s3] Some structural spelling outside r1-r4 (goto-sharing, physical case order, jump-thunk, sanctioned split-init, hoisted-common-set) survives to jump2 as a distinct pattern or blocks the merge byte-neutrally.
- mechanism: Each candidate perturbs a different canonicalization surface: r5 block duplication, r6 emission order, r7 jump_chain shape (m7's sibling), r8 cse const-fold timing, r9 counted-match count (last1==0).
- probe: tmp/grind/motion_SetMotion/s3/s3_probe.py — label-normalized cc1 asm diff vs m0 for r5-r9.
- result: r5/r6/r7/r8 all canonicalize to the identical merged 361-insn shape (86 diff lines each — same as honest-0xD); r9 does block the merge (block vanishes, last1==0 confirmed from the zero-insn side) but diverges 619 diff lines. The sanctioned split-init family is specifically measured dead here (cse folds before jump2).
- verdict: KILLED

## [s3b] A residual structural spelling class outside r1-r9 (duplicate case-9/case-11 blocks, cond-expr arm, unsigned selector type, pre-switch if-extraction of the jtbl label) survives to jump2 unmerged or blocks the merge byte-neutrally.
- mechanism: Each attacks a distinct closure assumption: r10 the "one shared C block" premise (two source-level duplicates could change jump_chain pairing order), r11 the cond-expr expansion surface (vs if/else statement forms r1/r3/m8), r12 the set13 RTL mode/guard shape via unsignedness (r2 only measured s16), r13 the stream-1 label bonus itself (no jtbl CODE_LABEL heading the block if 9/11 branch in from an if).
- probe: tmp/grind/motion_SetMotion/s3/s3b_probe.py — label-normalized cc1 asm diff vs m0 (363 insns) with honest-0xD control h0 (361 insns / 86 diff lines).
- result: r10 and r11 = byte-identical to the merged h0 signature (361 insns, same 86 diff lines); r12 = 149 diff lines (unsigned vacuates `sel >= 0`, dispatch tail restructured, still merged); r13 = 369 insns / 126 diff lines (2 extra compare pairs + jtbl rewrite — the label bonus is only avoidable at real byte cost). Fourth independent confirmation of the closure theorem.
- verdict: KILLED

## [s3] A residual structural spelling class outside r1-r9 (duplicate case-9/case-11 blocks, cond-expr arm, unsigned selector type, pre-switch if-extraction of the jtbl label) survives to jump2 unmerged or blocks the 13-pair merge byte-neutrally.
- mechanism: r10 attacks the one-shared-C-block premise (source-level duplicates could change jump_chain pairing order); r11 the cond-expr expansion surface (vs if/else forms r1/r3/m8); r12 the set13 RTL mode/guard shape via unsignedness (r2 only measured s16); r13 the stream-1 label bonus itself (no jtbl CODE_LABEL heading the block when 9/11 branch in from a pre-switch if).
- probe: tmp/grind/motion_SetMotion/s3/s3b_probe.py - label-normalized cc1 asm diff vs committed baseline m0 (363 insns) with honest-0xD control h0 (361 insns / 86 diff lines).
- result: r10 and r11 byte-identical to the merged h0 signature (361 insns, same 86 diff lines - cross-jump collapses the 9/11 duplicates first, survivor still merges with the ==3 arm); r12 = 362 insns / 149 diff lines (unsigned vacuates the sel>=0 dispatch guard, new j+move tail, still merged); r13 = 369 insns / 126 diff lines (2 extra beq/li compare pairs + rewritten jtbl entries - the label bonus is only avoidable at real byte cost).
- verdict: KILLED

## [s4] The discarded prior s4 attempt's perm_a output-0-1 (permuter score 0 with --stack-diffs, honest-0xD chassis) is a true byte-match closing form for the 13-pair wall.
- mechanism: Mutation moved load_sel2 (sel2 = D_800A3350) from case 13/17 into the ==3 arm; at jump2 the arm suffix [set13; lbu; j] cross-jumps its [lbu; j] tail into the shared lbu block, redirecting the arm's jump to a different CODE_LABEL than case-9/11's -> the 13-pair merge cannot fire (both li 13 survive, 402 insns).
- probe: Recompiled output-0-1/source.c through the exact build pipeline (cc1|prologue_fix|maspsx|as) and raw instruction-word diffed vs target.o (tmp/grind/motion_SetMotion/s4/verify_find.sh); calibrated noise floor with committed.c (exactly 1 li word + 3 jtbl-reloc lws).
- result: 6 real word diffs: the ==3 arm's beq+j target the lbu block (0x164) instead of sel_dispatch (0x26c) and case-9/11's j targets 0x26c instead of 0x164 — jump destinations SWAPPED vs target. Also semantically wrong both ways (arm dispatches sel2=lbu where target dispatches -1; case-13/17 loses its sel2 load). The permuter scorer's label normalization cannot see swapped branch targets: score 0 != bytes 0 on this function.
- verdict: KILLED

## [s4] A permuter basin (random honest-0xD chassis A / directed PERM_GENERAL chassis B) contains an honest byte-neutral spelling that keeps both 13-sites unmerged.
- mechanism: Random mutation explores CFG/statement geometries outside the hand-enumerated r1-r13/m1-m9 space; any iter1-breaking real insn between set13 and j blocks find_cross_jump.
- probe: Adopted, harvested and stopped both campaigns from the discarded 2026-07-18 attempt (perm_a: 112,618 iterations / 13.1h / 12 finds; perm_b directed: 2,049 iterations / 13 finds, best 100); raw word-diffed ALL 12 perm_a finds vs target (rawdiff_all.sh).
- result: Every find classified: score 100-200 forms = merged-shape layout divergence (119-125 real word diffs); the only sub-100 wins are score-0 (jump-target swap, above) and score-60/120 (SEMANTIC THEFT: a live store moved from case-10's D_800A3207=5 path into case 9/11 — D_800A3350=0 resp. D_800A334C=0x5A — real sb between set13 and j blocks the merge, but the store executes on the wrong path and sits 11 slots from target's position, 6-8 real word diffs). Zero honest finds. The scorer's floor is occupied by false matches, so scorer-guided search on this wall is metric-invalid by construction.
- verdict: KILLED

## [s4] The 'two different CODE_LABELs at jump2 time' unmerge family (s1's original WIP conjecture) is honestly reachable in a byte-matching compile.
- mechanism: If the two set13 jumps target different labels they sit in different jump_chains and find_cross_jump never pairs them; output-0-1 is the first measured instance of this family actually firing.
- probe: Measured via output-0-1's raw diff + source-level analysis against the s1-mapped do_cross_jump (jump.c:2536-2583): the redirect label is placed BEFORE the partner's matched insns, so the redirected jump's target word can only equal target's 0x26c if the matched insns are byte-free.
- result: The family fires only by (a) routing one 13-site through the shared lbu block — semantically forbidden on both 13-paths since both must dispatch with sel2=-1 (target words differ, measured) — or (b) byte-free matched insns, i.e. the USE/CLOBBER manufacture family the Judge ruled unsanctioned (decisions.md 2026-07-17 17:09). Closure theorem now confirmed from a fifth independent direction, this time by blind search.
- verdict: KILLED

## [s5] Seeding the permuter from the UNMERGED side of the merge boundary (s3b r13 pre-switch-if chassis) yields a mutation path that repairs the +6-insn/jtbl divergence without re-entering the 13-pair merge.
- mechanism: s4's campaigns both seeded merged shapes, so the search had to break the merge (proven to need semantic theft or byte-free USE/CLOBBER). An unmerged seed instead asks the search to recover target's 402-insn layout while both li-13 sites survive — a basin no campaign had explored.
- probe: tmp/grind/motion_SetMotion/s5/perm_c — fresh workspace (setup_c.py), campaign s5-r13-unmerged-seed, 54,087 iterations / ~30 min / -j6 --stack-diffs; best finds raw-word-verified via compile.sh + objdump diff vs target.dump.txt (harvest_c.sh).
- result: KILLED. Base 1125 perm-score / 136 word diffs; best find 660 at ~10 min then 20 min dry; verified finds sit at 76-91 word diffs / 403-406 insns (noise floor = 1 word). Calibration also shows the chassis is structurally short a li13: target has 3x 2410000d, r13 emits 2 (the extracted if-block's set13 cross-jumps with the ==3 arm's — the merge picks a different partner when the jtbl label is removed). The basin descends by repairing branch offsets, never by restoring the third li13.
- verdict: KILLED

## [s5] Seeding the permuter from the UNMERGED side of the merge boundary (s3b r13 pre-switch-if chassis) yields a mutation path that repairs the +6-insn/jtbl divergence without re-entering the 13-pair merge.
- mechanism: s4's campaigns both seeded merged shapes, forcing the search to break the merge (proven to need semantic theft or banned USE/CLOBBER). An unmerged seed instead asks the search to recover target's 402-insn layout while both li-13 sites survive - a basin no prior campaign explored.
- probe: Fresh workspace tmp/grind/motion_SetMotion/s5/perm_c (setup_c.py); campaign s5-r13-unmerged-seed via tools/permuter_campaign.py, 54,087 iterations / ~30 min / -j6 --stack-diffs; fresh-seed stop after 20 dry minutes; best 4 finds recompiled through the build pipeline and raw-word-diffed vs target (harvest_c.sh).
- result: Base perm-score 1125 / 136 raw word diffs. Best find 660 at ~10 min, then dry. Verified finds: 76-91 word diffs, 403-406 insns vs target 402 (committed noise floor = 1 word). Calibration micro-fact: target has THREE 2410000d (li s0,13) words, the r13 chassis emits only TWO - with the jtbl label removed, find_cross_jump merges the extracted if-block's set13 with the ==3 arm's set13 instead (both non-jtbl [set13; j] blocks on the same jump_chain). Removing the label bonus does not protect the pair; the merge picks a different partner. No mutation in 54k iterations restored the third li13.
- verdict: KILLED

## [s6] Belt-and-braces exhaustiveness check: a gccdbg find_cross_jump trace on the (arm, case-9/11) pair confirms the merge fires exactly via the chain-partner stream-1 label bonus and no other exit condition is reachable.
- mechanism: Env-gated BB2_XJUMP_DEBUG printf knob in tools/gcc-2.7.2/jump.c (diagnostic in-tree cc1 rebuild; frozen build/cc1 untouched; knob verified codegen-inert and diagnostic cc1 verified byte-faithful to build/cc1 on all three TUs) tracing entry/every-exit-condition/MATCH/verdict/DO_CROSS_JUMP.
- probe: tmp/grind/motion_SetMotion/s6/trace.sh on honest.c (s4/perm_a honest-0xD), r13.c (s5/perm_c chassis), committed.c (control); uid->block maps certified from honest.c.sched2. Logs: s6/{honest,committed,r13}.xjdbg.log; write-up: s6/analysis.md.
- result: CONFIRMED on all three surfaces. (1) honest-0xD: merge = e1-jtbl-side chain attempt, 1 counted set13 match + LABEL-BONUS at jtbl label 403 -> min 0 -> DO_CROSS_JUMP 408/406/211; arm-side and own-label attempts all fail; no other exit path touched. (2) r13: bonus is NOT jtbl-specific — both non-arm 13-blocks cascade-merge via their own heading branch-target labels (236, 757); label-bonus dodging impossible in any honest chassis. (3) committed: set12-vs-set13 PAT-MISMATCH breaks iteration 1 BEFORE the label walk -> bonus unreachable -> pair protected; and the -1 pair shows 0 counted matches (sb leads case-10's stream) = F1 live in-trace.
- verdict: CONFIRMED

## [s6] A gccdbg find_cross_jump trace on the (arm, case-9/11) pair confirms the merge fires exactly via the chain-partner stream-1 label bonus and no other exit condition is reachable.
- mechanism: Env-gated BB2_XJUMP_DEBUG printf knob added to tools/gcc-2.7.2/jump.c (gitignored source; established diagnostic-cc1 precedent); IN-TREE diagnostic cc1 rebuilt, frozen build/cc1 untouched; knob verified codegen-inert (only the options-comment line differs) and the diagnostic cc1 verified byte-faithful to build/cc1 on all three TUs.
- probe: tmp/grind/motion_SetMotion/s6/trace.sh: traced honest.c (s4/perm_a honest-0xD), r13.c (s5/perm_c chassis), committed.c (control) with entry/every-exit-condition/MATCH/verdict/DO_CROSS_JUMP logging; uid->block maps certified from honest.c.sched2.
- result: Honest-0xD: merge = e1-408 (jtbl block) chain attempt, MATCH 406/211 set13 (min 2->1) then LABEL-BONUS at jtbl CODE_LABEL 403 (min->0) -> DO_CROSS_JUMP 408/406/211; arm-side attempt breaks at jump_insn-vs-barrier with min=1, own-label attempts end last1=0 — no other exit path touched. r13: bonus NOT jtbl-specific, both non-arm 13-blocks cascade-merge via their own heading branch-target labels (236, 757). Committed: set12-vs-set13 PAT-MISMATCH at iteration 1 -> label bonus NEVER reached -> pair protected both directions. -1 pair: case-10's stream leads with the sb (set-1 sched2-hoisted) -> 0 counted matches -> F1 confirmed live in-trace.
- verdict: CONFIRMED

## [s7] The n1 union-constructor CLOBBER form (rejected/judge-fail-0717-1708.c) is in-trace verifiable: the cast-to-union assignment emits a byte-free CLOBBER that protects the 13-pair from find_cross_jump in both directions, and the TU is byte-identical to the committed+rule TU except the honest li 13.
- mechanism: expr.c:2996 emits (insn 407 (clobber (reg/v:SI 16 s0)) REG_UNUSED) between the jtbl CODE_LABEL (405) and set13 (409); the byte-free extra insn desynchronizes find_cross_jump's streams so both directions break at the GET_CODE insn-code check (INSN vs JUMP_INSN) at iteration 2 with min=1, BEFORE the stream-1 label walk — the label bonus is never reached. Refinement over the 17:09 wording: it is a stream-desync insn-code break, not a SET-vs-CLOBBER pattern compare. reorg still fills the delay slot to target's [j; delay li 13].
- probe: s7/make_union_tu.py ported the union retype into s6's preprocessed committed.c; trace_union.sh ran the in-tree diagnostic cc1 with BB2_XJUMP_DEBUG + -da (knob verified inert, diagnostic cc1 verified byte-faithful to frozen build/cc1 on this TU); jump2 dump certified uid 407 as the CLOBBER; final asm diffed vs s6/committed.nodbg.s.
- result: Trace: arm-side attempt (e1=223 vs e2=411) = MATCH set13/set13 then break insn-code-mismatch i1=207(jump_insn) i2=insn(CLOBBER 407), min=1, no; jtbl-side attempt (e1=411 vs e2=223) = MATCH 409/211 then break insn-code-mismatch i1=407(insn) i2=jump_insn, min=1, no. Zero DO_CROSS_JUMP events on any 13-site uid; the Wall-1 sel2-tail merge (253/251/238) still fires. Asm diff vs committed = .file line + the single li 0xc-vs-0xd word only.
- verdict: CONFIRMED

## [s7] SOTN / Vagrant Story / ESA master branches contain accepted cast-to-union, single-member-union, or manufactured-CLOBBER spellings that would give the union-CLOBBER family community precedent.
- mechanism: The 17:34 disposition conditions option (a) of the escalation on master-branch community evidence; if any reference project ships the construct as accepted matched code, the family clears the SOTN-evidence bar.
- probe: Shallow-cloned all three masters (sotn-decomp@2472557, rood-reverse@0ff937b, esa@914e9db) to tmp/grind/motion_SetMotion/s7/repos and grepped: cast-to-union in expression position, single-member union decls, the string 'clobber' (case-insensitive, all file types), and inspected every union declaration hit.
- result: ZERO evidence: no (union X) cast appears anywhere in any tree; no single-member unions; no 'clobber' occurrences at all; every union present is a genuine multi-member data union (SOTN PrimBuf prim overlay, VS bitfield/u16-s32 punning, ESA include/structs.h data). The escalation states this plainly per the disposition.
- verdict: KILLED

## [s8] A fresh rederive (m2c / sibling-transplant, structurally different C shape) could break the 13-pair find_cross_jump merge honestly.
- mechanism: The sealed closure theorem (nine confirmations incl. two white-box gccdbg traces, s2-s7) proves any byte-matching compile forces both 13-sites to exactly [set13; j] with the jtbl CODE_LABEL adjacent, guaranteeing the counted-match + stream-1 label-bonus merge; the only byte-free C-reachable breaker is the unsanctioned USE/CLOBBER class. A structurally different shape either canonicalizes to the merged form or pays real bytes (r1-r13 all KILLED).
- probe: Verified this session against docs/grind/decisions.md: the 17:34 Judge ruling certifies 'No honest lever remains unmeasured' and the 2026-07-18 OWNER-ESCALATION (filed by s7) is the final entry with no owner ruling following it; candidate.c banked at memory/grind/motion_SetMotion/candidate.c.
- result: Escalation confirmed on file and unruled; exhaustion Judge-certified complete; rederive falls inside the dead axis covered by the closure theorem, so re-measuring it is prohibited spin per the brief.
- verdict: CONFIRMED

## [s9] The s9-mandated rederive axis (fresh m2c / corpus / sibling transplant producing a structurally different C shape) contains an honest byte-neutral spelling not covered by the sealed closure theorem.
- mechanism: The closure theorem (nine confirmations s2-s7, including two white-box BB2_XJUMP_DEBUG gccdbg traces) proves any byte-matching compile forces both 13-sites to exactly [set13; j] with the jtbl CODE_LABEL adjacent, guaranteeing the counted-match + stream-1 label-bonus find_cross_jump merge; every structurally-different shape enumerated (r1-r13, m1-m9, 166k permuter iterations across merged and unmerged seeds) either canonicalizes to the merged form or pays real bytes. A rederive output is a C shape and therefore lands inside the same partition.
- probe: Verified this session: docs/grind/decisions.md 2026-07-17 17:34 Judge ruling certifies 'no honest lever remains unmeasured'; the 2026-07-18 OWNER-ESCALATION filed by s7 is the final entry with no owner ruling after it (tail of file inspected); ledger hypotheses.md s8 entry already CONFIRMED this same coverage; candidate.c (44,155 bytes, floor-10 form) and all 9 rejected forms verified present in memory/grind/motion_SetMotion/.
- result: Escalation confirmed on file and unruled; exhaustion Judge-certified complete; rederive is covered by the closure theorem's C-shape partition, so re-measuring it would be prohibited spin on a dead axis per the brief's owner-gated clause.
- verdict: KILLED

## [s10] Synthesis: some cross-session combination of banked partial results (a merged attack drawing on two or more axes) escapes the three-basin partition and yields an honest byte-neutral unmerge.
- mechanism: If any two banked negatives were negatives only in isolation (e.g. F1's slack mechanism combined with r13's label removal, or a permuter find repaired by a structural respelling), their composition could sit outside the enumerated basins.
- probe: Full ledger re-read (evidence.md 10 sessions, hypotheses.md 15 entries, 9 rejected forms, decisions.md rulings); composition check against the partition argument (tmp/grind/motion_SetMotion/s10/synthesis.md): every axis's kill is a statement about the CANONICALIZED jump2-time RTL, not about the source spelling, so compositions canonicalize identically — F1+r13 still lacks byte-room for a slack insn AND still grants the bonus via any heading label (s6: bonus not jtbl-specific); permuter finds below the merge penalty are semantics-breaking regardless of chassis (s4); split-init folds before jump2 regardless of surrounding shape (r8).
- result: No composition escapes: the basins are defined over jump2-time RTL invariants (both 13-sites = [set13; j] + adjacent heading label in ANY byte-matching compile), which every source-level combination must still satisfy. The merged attack IS the escalation: option (a) n1 re-proposal or option (b) owner rule-disposition. Frontier reset accordingly.
- verdict: KILLED

## [s10] Some cross-session combination of banked partial results (a merged attack drawing on two or more axes) escapes the three-basin partition and yields an honest byte-neutral unmerge of the 13-pair.
- mechanism: If any banked negative were a negative only in isolation (F1 slack + r13 label removal, permuter find + structural respelling, split-init + reshaped arm), the composition could sit outside the enumerated basins.
- probe: Full ledger re-read (10 sessions of evidence, 15 hypothesis entries, 9 rejected forms, both Judge rulings + the escalation text) and a composition check against the partition argument, written up in tmp/grind/motion_SetMotion/s10/synthesis.md; honest baseline re-measured (sandbox --disable all = 1, 402/402).
- result: No composition escapes: every axis kill is a statement about canonicalized jump2-time RTL invariants (any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent), which all source-level combinations must still satisfy — F1+r13 still lacks slack byte-room AND still grants the label bonus via any heading label (s6: bonus not jtbl-specific); sub-merge-penalty permuter finds are semantics-breaking regardless of chassis; split-init folds before jump2 regardless of surrounding shape. The only merged attack remaining IS the escalation itself.
- verdict: KILLED

## [s11] A structural lever (block-local split, declaration order, type narrowing, statement re-association) remains unmeasured on the 13-pair wall
- mechanism: Any such spelling must either canonicalize to the merged [set13; j] shape (merge guaranteed by jump.c counted-match + stream-1 label bonus per the s6 BB2_XJUMP_DEBUG trace) or pay real bytes; the only byte-free iter1-breaker is the unsanctioned USE/CLOBBER class per the s6/s7 white-box traces
- probe: Ledger audit this session: hypotheses.md r1-r13 all KILLED (s2/s3/s3b structural sweeps), F1 slack-transplant negative banked (s2), label-family negatives m7/r13 banked, closure theorem carries eight independent confirmations; decisions.md 17:34 ruling certifies 'no honest lever remains unmeasured'
- result: No unmeasured structural probe exists; the axis is closed by Judge-certified exhaustion and the brief prohibits re-measuring dead axes
- verdict: KILLED

## [s12] A sanctioned structural lever (block-local split, declaration order, type narrowing, statement re-association) still exists that breaks the 13-pair find_cross_jump merge without bytes
- mechanism: jump2 find_cross_jump counted-match + stream-1 label-bonus path (jump.c:1966-2001, 2371-2533)
- probe: Ledger + decisions.md verification this session: r1-r13 all KILLED (s2/s3/s3b), F1 measured non-transplantable, 17:34 Judge ruling certifies 'no honest lever remains unmeasured', s6 BB2_XJUMP_DEBUG trace confirms merge is guaranteed for any byte-matching compile
- result: Escalation entry confirmed as final unruled entry at decisions.md:761 (read through EOF at line 777, no owner ruling follows); closure theorem carries eight independent confirmations including both structural sweeps and pass-level traces
- verdict: KILLED

## [s13] A new permuter campaign this session could surface a novel sanctioned form not covered by the s4/s5 closure
- mechanism: Fresh-seed discipline says a basin yields early or not at all; s4 closed the merged-chassis basin (only score-0 was a scorer false match with swapped jump targets, every sub-merge-penalty find was semantics-breaking cross-path instruction theft) and s5 closed the unmerged r13 basin (plateau at 82 raw word diffs; the find_cross_jump merge re-forms with a different partner when perturbed). The Judge-certified closure theorem (s6 gccdbg white-box trace) proves any byte-matching C shape must pass through the 13-pair label-bonus merge, which only the unsanctioned USE/CLOBBER-manufacture family breaks.
- probe: Verified the escalation entry exists and is unruled (docs/grind/decisions.md:761-776, final entry, no ruling after); verified both permuter negative results are banked (rejected/s4-permuter-false-zero-swapped-jump-targets.c, rejected/s4-permuter-semantic-theft-store-into-case911.c, rejected/s5-permuter-unmerged-r13-seed-plateau.c) and cited in the escalation's references
- result: Permuter axis remains closed two-sided; re-running a campaign would re-measure a dead axis in violation of the brief's anti-spin directive. Escalation confirmed filed and awaiting owner ruling.
- verdict: KILLED

## [s14] The owner-gate preconditions hold for s14: the filed OWNER-ESCALATION is still the final unruled entry in decisions.md and the mandated permuter axis is banked dead
- mechanism: Brief rule: owner-gated requires a filed OWNER-ESCALATION naming the function plus every remaining sanctioned axis measured dead; re-measuring dead axes when the escalation exists is forbidden spin
- probe: Grep + tail-read of docs/grind/decisions.md (headings scan, lines 755-776); ledger evidence.md s4/s5/s6/s10 entries; sandbox motion_SetMotion --disable all re-run
- result: Escalation block at lines 761-776 is the FINAL entry (file ends at 776), marked 'awaiting owner ruling - do not self-resolve', no ruling after it; permuter closure is two-sided per s4/s5 with the s6 white-box trace certifying the closure theorem; sandbox re-confirmed honest distance 1 (402/402, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU)
- verdict: CONFIRMED

## [s15] The forensics axis (instrumented-cc1 RTL/jump2 tracing) still has an unmeasured decision point that could name a new honest lever.
- mechanism: s6 traced the honest and committed TUs with BB2_XJUMP_DEBUG (merge = 1 counted set13 match + stream-1 label bonus, the only reachable path; the rule's set12-vs-set13 mismatch breaks iteration 1) and s7 traced the union-CLOBBER n1 form end-to-end (byte-free clobber desynchronizes both streams, GET_CODE break with min=1 before the label walk, both directions). Every find_cross_jump exit path at the 13-pair is accounted for.
- probe: Verified this session against the ledger digest and the escalation text (decisions.md:767-769): eight independent confirmations of the closure theorem including both white-box traces; the USE/CLOBBER class is the only byte-free C-reachable breaker and is Judge-forbidden pending the owner ruling.
- result: No unmeasured forensics decision point exists; the axis is dead by the Judge-certified closure theorem and the banked s6/s7 traces.
- verdict: KILLED

## [s16] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion has been ruled by the owner since s15, reopening a grindable axis
- mechanism: An owner ruling appended to docs/grind/decisions.md after line 776 would supersede the parked state and select option (a) or (b)
- probe: Read docs/grind/decisions.md tail this session (lines 755-777, EOF confirmed at 777); escalation block at 761-776 is the final entry, still marked 'awaiting owner ruling - do not self-resolve'
- result: No ruling entry follows the escalation; it remains unruled as of 2026-07-18 s16
- verdict: KILLED

## [s16] The mandated forensics axis has an unmeasured probe left (an RTL/jump2 decision not yet named at pass level)
- mechanism: Instrumented-cc1 dumps could reveal a discriminator not covered by the banked traces
- probe: Ledger audit: s6 traced honest/committed/r13 TUs end-to-end with BB2_XJUMP_DEBUG (exact win path: 1 counted set13 match + stream-1 CODE_LABEL bonus; committed rule = set12-vs-set13 mismatch at jump.c:2469 before the label walk; F1 sched2-slack confirmed in-trace), s7 traced the n1 union-CLOBBER form (stream-desync GET_CODE break, both directions, byte-identical modulo the one li word)
- result: Every find_cross_jump exit condition for both 13-pair directions and both -1-pair directions is trace-accounted; the only C-reachable byte-free breaker is the Judge-banned USE/CLOBBER family - nothing forensics can add without violating the binding constraints
- verdict: KILLED

## [s17] A structurally different re-derived C shape (fresh m2c, decomp.me corpus, sibling/Kengo transplant) could escape the 13-pair cross-jump merge byte-neutrally.
- mechanism: The s6 gccdbg white-box trace certifies the closure theorem over the OUTPUT byte constraint, not over any particular C spelling: any C that compiles to the target bytes presents jump2 with the identical [set13; j] twin suffixes plus the stream-1 label bonus, so find_cross_jump merges them regardless of source shape. s8/s9 already banked rederive-dead-by-construction; slog-kengo-dead-end additionally rules out Kengo transplant material for this file family.
- probe: Verified this session: escalation entry present and unruled at docs/grind/decisions.md:761-776 (tail of file is its References line, no ruling appended); ledger hypotheses.md re-read — closure theorem confirmed from five independent directions (r1-r13 structural, permuter two-sided s4/s5, forensics s6/s7).
- result: No sanctioned probe exists on the rederive axis: the theorem is quantified over all byte-matching shapes, so producing another shape measures nothing new; the only unmeasured space is the unsanctioned USE/CLOBBER-manufacture family, which the binding Judge constraints forbid re-proposing.
- verdict: KILLED

## [s18] A structurally different C shape (fresh m2c / decomp.me / sibling transplant) could break the 13-pair cross-jump merge without the unsanctioned USE/CLOBBER class
- mechanism: The Judge-certified closure theorem (17:34 ruling + s6/s7 white-box traces): ANY byte-matching compile forces both 13-sites to exactly [set13; j] with an adjacent heading CODE_LABEL, making find_cross_jump's merge guaranteed via the counted-match + stream-1 label-bonus path; the only byte-free C-reachable breaker is the USE/CLOBBER class, which is Judge-constrained as unsanctioned in every respelling
- probe: Verified this session: the escalation entry is the final, unruled entry in docs/grind/decisions.md (lines 761-776); the closure theorem carries eight independent confirmations (r1-r13 structural sweeps, F1 slack-transplant negative, m7/r13 label-family negatives, three permuter campaigns totaling 168k+ iterations, s6 BB2_XJUMP_DEBUG pass trace, s7 union-form end-to-end trace) - a rederive session can only produce shapes already inside the sealed three-basin partition (s10)
- result: Escalation confirmed on file and unruled; rederive axis confirmed dead by construction - any new shape either canonicalizes to the merged topology (honest distance 1), pays real bytes, or lands in the Judge-forbidden CLOBBER family
- verdict: KILLED

## [s19] The s19-mandated synthesis axis (a merged attack composing banked partial results across axes) contains an honest byte-neutral unmerge of the 13-pair not covered by the s10 three-basin partition, or the owner has ruled the escalation since s18, reopening a grindable axis
- mechanism: If any banked negative were a negative only in isolation, a composition could sit outside the enumerated basins; alternatively an owner ruling appended after decisions.md:776 would supersede the parked state and select option (a) or (b)
- probe: Full re-verification this session: read docs/grind/decisions.md:755-776 (file is 776 lines; the OWNER-ESCALATION block at 761-776 is the final entry, still marked 'awaiting owner ruling — do not self-resolve', no ruling follows); re-read hypotheses.md (25 entries, s2-s18) and confirmed every ladder axis banked dead (structural r1-r13/m1-m9, permuter two-sided 168k+ iterations, forensics s6/s7 white-box traces, rederive dead-by-construction, synthesis s10 partition); verified candidate.c (44,155 bytes) and all 9 rejected/ forms on disk
- result: No ruling has been appended — the escalation remains unruled; the s10 partition argument stands unrebutted: every axis kill is a statement about canonicalized jump2-time RTL invariants (any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing the counted-match + stream-1 label-bonus merge), which every source-level composition must still satisfy; the only byte-free breaker is the Judge-banned USE/CLOBBER-manufacture family. Any in-session probe would re-measure a dead axis, which the brief forbids while the escalation is on file
- verdict: KILLED

## [s20] The structural axis (block-local splits, decl order, type narrowing, re-association) still holds an unmeasured lever for the last li-13 word
- mechanism: Any structural respelling would have to break find_cross_jump's counted-match + label-bonus merge of the two [set13; j] sites without paying bytes
- probe: Verified the ledger and decisions.md first-hand this session: r1-r13 structural sweeps banked KILLED (s2/s3/s3b), F1 slack-transplant negative banked, s6/s7 pass-level traces seal the closure theorem two-sided; the 2026-07-17 17:34 Judge ruling certifies 'No honest lever remains unmeasured' and s11/s12 already re-verified the structural axis dead
- result: Escalation entry confirmed as the final, unruled entry in docs/grind/decisions.md (lines 761-776, filed 2026-07-18); no owner ruling follows it; all structural forms are either byte-paying or canonicalize to the merged shape
- verdict: KILLED

## [s21] A structural lever (block-local split, declaration order, type narrowing, statement re-association) remains unmeasured that could break the 13-pair find_cross_jump merge without bytes
- mechanism: Judge-certified closure theorem: any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing the merge via jump.c's counted-match + stream-1 label-bonus path; the only byte-free C-reachable iter1-breaker is the unsanctioned USE/CLOBBER class
- probe: Verified against the banked ledger: structural sweeps r1-r13 (s2/s3/s3b) all KILLED, label-family negatives m7/r13 KILLED, F1 slack-transplant KILLED, three permuter campaigns (168k+ iterations) negative, s6/s7 white-box pass traces confirm the theorem end-to-end; escalation entry re-verified as final and unruled in decisions.md this session
- result: No structural axis remains: every spelling in the codegen-technique-index catalog applicable here is banked KILLED; the closure theorem certifies the axis dead by construction, and the Judge's 17:34 ruling states no honest lever remains unmeasured
- verdict: KILLED

## [s22] A fresh permuter campaign on motion_SetMotion could surface a novel byte-free spelling that breaks the case-9/11 find_cross_jump merge
- mechanism: decomp-permuter mutation search over the diverging region around the two [li s0,13; j sel_dispatch] blocks
- probe: Ledger check before launch: s13/s14 banked the permuter axis dead two-sided (112,618 + 2,049 + 54,087 iterations from both sides of the merge boundary); the closure theorem (8 independent confirmations, Judge-certified complete 2026-07-17 17:34) proves any byte-matching compile forces the merged shape with the USE/CLOBBER class the only C-reachable breaker, and that class is Judge-banned pending the owner ruling
- result: Axis already measured dead; re-launching would re-measure a banked negative in violation of the ledger contract. Escalation entry re-verified as filed and unruled this session.
- verdict: KILLED

## [s23] The mandated permuter axis still holds unexplored basins for motion_SetMotion.
- mechanism: A fresh permuter seed could only matter if some C-reachable spelling breaks jump2's find_cross_jump merge of the two [li s0,13; j sel_dispatch] blocks without the unsanctioned USE/CLOBBER class.
- probe: First-hand re-verification of the ledger and decisions.md instead of a campaign: the closure theorem (F1 discharged in evidence.md [s2], r1-r9 all KILLED in hypotheses.md) proves any byte-matching compile forces both 13-sites into the guaranteed-merge shape; the s5 fresh-seed plateau and s4 false-zero/semantic-theft finds are banked; permuter sessions s13/s14/s22 confirmed the axis dead two-sided.
- result: Axis remains dead by construction; the only remaining movers are the two owner acts in the filed escalation. No campaign run - launching one would spin a measured-dead axis, which the brief forbids.
- verdict: KILLED

## [s24] A new forensics session could name a GCC pass/decision not already in the ledger for the last-word divergence
- mechanism: cc1 RTL/ALLOCDBG/GREG dumps of the honest vs committed TU
- probe: Checked ledger: s6 traced honest+committed forms (merge fires via 1 counted set13 match + label bonus; rule's set12-vs-set13 mismatch breaks iteration 1) and s7 traced the union n1 form end-to-end (clobber insn 407 desyncs both stream directions, break at GET_CODE with min=1, before the label walk; reorg fills delay slot to target's exact bytes). Escalation entry itself cites these traces as part of the Judge-certified exhaustion.
- result: The forensics axis is fully measured: the divergence mechanism is named to the source line (expr.c:2996 CLOBBER emission; jump.c find_cross_jump GET_CODE exit family) with artifacts banked in tmp/grind/motion_SetMotion/s6/ and s7/. No un-dumped decision remains between the honest floor and the target bytes.
- verdict: KILLED

## [s25] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion is on file in docs/grind/decisions.md and remains unruled
- mechanism: Owner-gating precondition check: the escalation must exist, name this function, and have no subsequent owner ruling
- probe: Read docs/grind/decisions.md:755-777 and scanned all entry headers; the escalation at :761 is the final entry with no OWNER RULING after it
- result: Escalation present (filed by grind s7 per the 17:34 Judge disposition), presents both owner options, states community evidence NONE FOUND plainly, and is the last entry in the file
- verdict: CONFIRMED

## [s25] The mandated forensics axis (instrumented cc1 RTL dumps naming the exact GCC pass/decision) is already measured dead on the ledger
- mechanism: Forensics deliverable = name the pass and decision producing the divergence; the s6/s7 sessions already did this at pass level: jump2 find_cross_jump merges the two [set13; j] blocks via 1 counted set13 match + stream-1 label bonus (honest form, trace shows exactly one merge event); the only C-reachable byte-free iteration-1 breaker is the USE/CLOBBER class (s7 trace: constructor-cast CLOBBER desynchronizes the two streams so GET_CODE compares INSN vs JUMP_INSN and breaks with min=1 before the label walk), which the Judge ruled unsanctioned twice (17:09, 17:34) and the binding judge_constraints forbid respelling
- probe: Verified banked artifacts exist: tmp/grind/motion_SetMotion/s6/ (honest/committed/r13 traces) and s7/union.xjdbg.log, union.c.jump2, vs_committed.diff; cross-checked the escalation's exhaustion section (eight independent confirmations of the closure theorem)
- result: Nothing remains for forensics to discover: the pass, the exact jump.c exit, both directions of the break, and the reorg delay-slot fill to target's [j; delay li13] are all already traced and banked; any further cc1 dump would re-measure a dead axis, which the brief forbids
- verdict: CONFIRMED

## [s26] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion remains the final, unruled entry in docs/grind/decisions.md
- mechanism: Owner may have ruled since s25; owner-gated is only valid against a live unruled escalation
- probe: Read decisions.md:755-777 and the file tail this session (s26): the escalation block at lines 761-776 ends with its References line and nothing follows it
- result: Escalation present, filed by grind s7 in hirahira_w_frie format with both options and the plainly-stated absent community evidence; no OWNER RULING entry after it
- verdict: CONFIRMED

## [s26] The mandated rederive axis is already measured dead on the ledger and must not be re-measured
- mechanism: The closure theorem quantifies over ALL byte-matching compiles, not source spellings: any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing the find_cross_jump merge; the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class. A fresh m2c/decomp.me/Kengo chassis cannot leave the s10 three-basin partition (merged / unmerged-with-cost / banned byte-free breaker)
- probe: Ledger audit this session: s8/s9 rederive kills, s17/s18 rederive re-verification, s10 synthesis partition, nine independent closure confirmations (r1-r13 structural, F1 slack, 168,754 permuter iterations two-sided, s6/s7 white-box gccdbg traces, zero community evidence in sotn/rood-reverse/esa masters)
- result: All axis-kill evidence intact and internally consistent; no unmeasured honest lever exists per the 17:34 Judge certification
- verdict: CONFIRMED

## [s26] The banked ledger artifacts referenced by escalation option (a) are intact
- mechanism: Option (a) closes via re-proposal of the byte-proven n1 form; a missing or corrupted bank would invalidate the escalation's disposition path
- probe: Verified candidate.c = 44,155 bytes and all 9 rejected/ forms present including judge-fail-0717-1708.c (44,478 bytes, the byte-proven trace-proven n1 union form)
- result: Bank intact; src/code6cac_c_mid.c and regfix.txt:2190 untouched on main per the 17:34 disposition
- verdict: CONFIRMED

## [s27] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion remains the final, unruled entry in docs/grind/decisions.md
- mechanism: Owner may have ruled since s26; owner-gated is only valid against a live unruled escalation
- probe: Read decisions.md:740-EOF this session (s27): the escalation block at lines 761-776 ends with its References line at 776 and nothing follows it
- result: Escalation present and unruled; both owner options and the plainly-stated absent community evidence intact
- verdict: CONFIRMED

## [s27] The mandated rederive axis (fresh m2c / decomp.me corpus / sibling-Kengo transplant) holds an unmeasured honest lever for the last li-13 word
- mechanism: The closure theorem quantifies over ALL byte-matching compiles, not source spellings: any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing the find_cross_jump counted-match + stream-1 label-bonus merge; the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class. A rederived chassis is a C shape and cannot leave the s10 three-basin partition (merged / unmerged-with-byte-cost / banned byte-free breaker). slog-kengo-dead-end additionally rules out Kengo transplant material for this file family.
- probe: First-hand ledger + decisions.md verification this session plus a fresh sandbox measurement: sandbox motion_SetMotion --disable all returned score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU — byte-for-byte identical to the banked s14 measurement; bank verified intact (candidate.c 44,155 B, 9 rejected forms incl. the n1 form 44,478 B)
- result: Rederive axis dead by construction per the s8/s9/s17/s18/s26 kills and the 17:34 Judge certification that no honest lever remains unmeasured; producing another shape would re-measure a dead axis, forbidden while the escalation is on file
- verdict: KILLED

## [s27] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion has been ruled by the owner since s26, reopening a grindable axis
- mechanism: An owner ruling appended after docs/grind/decisions.md:776 would supersede the parked state and select option (a) sanction-the-family or (b) refuse-the-family
- probe: Read decisions.md:740-EOF this session; the OWNER-ESCALATION block at lines 761-776 ends with its References line at 776 and nothing follows it
- result: No ruling entry follows the escalation; it remains the final, unruled entry, still marked 'awaiting owner ruling - do not self-resolve'
- verdict: KILLED

## [s27] The mandated rederive axis (fresh m2c / decomp.me corpus / sibling-Kengo transplant producing a structurally different C shape) holds an unmeasured honest lever for the last li-13 word
- mechanism: The closure theorem quantifies over ALL byte-matching compiles, not source spellings: any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing find_cross_jump's counted-match + stream-1 label-bonus merge (s6 BB2_XJUMP_DEBUG trace); the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class (s7 trace). A rederived chassis is a C shape and cannot leave the s10 three-basin partition; slog-kengo-dead-end additionally rules out Kengo transplant material for this file family
- probe: First-hand ledger + decisions.md verification plus a fresh gradient measurement: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. judge-fail-0717-1708.c 44,478 B)
- result: Rederive axis confirmed dead by construction per the s8/s9/s17/s18/s26 kills and the 17:34 Judge certification that no honest lever remains unmeasured; producing another shape would re-measure a dead axis, which the brief forbids while the escalation is on file
- verdict: KILLED

## [s28] The mandated synthesis axis (a merged attack composing banked partial results across axes) contains a composition outside the s10 three-basin partition, or the owner has ruled the 2026-07-18 escalation since s27, reopening a grindable axis
- mechanism: Every axis kill in the ledger is a statement about canonicalized jump2-time RTL invariants (any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing find_cross_jump's counted-match + stream-1 label-bonus merge per the s6 BB2_XJUMP_DEBUG trace), so every source-level composition must still satisfy them; the only byte-free breaker is the Judge-banned USE/CLOBBER class (s7 trace). An owner ruling appended after decisions.md:776 would supersede the parked state
- probe: Full ledger re-read this session (hypotheses.md 30+ entries s2-s27, both Judge rulings, the escalation text); read decisions.md:755-EOF (the OWNER-ESCALATION block at 761-776 ends with its References line and nothing follows it); fresh gradient measurement: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU — identical to the banked s14/s27 measurements; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. judge-fail-0717-1708.c 44,478 B)
- result: No ruling has been appended — the escalation remains the final, unruled entry; the s10 partition argument stands unrebutted (re-verified s19, s28): compositions of banked negatives (F1 slack + r13 label removal, permuter finds + structural respelling, split-init + arm reshape) all canonicalize inside the three basins, so the only merged attack remaining IS the escalation itself — option (a) n1 re-proposal or option (b) owner rule-disposition. Synthesis frontier stays reset to those two owner options
- verdict: KILLED

## [s28] The mandated synthesis axis contains a composition of banked partial results outside the s10 three-basin partition, or the owner has ruled the 2026-07-18 escalation since s27, reopening a grindable axis
- mechanism: Every axis kill is a statement about canonicalized jump2-time RTL invariants (any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing find_cross_jump's counted-match + stream-1 label-bonus merge per the s6 BB2_XJUMP_DEBUG trace), which every source-level composition must still satisfy; the only byte-free breaker is the Judge-banned USE/CLOBBER class (s7 trace). An owner ruling appended after decisions.md:776 would supersede the parked state
- probe: Full ledger re-read (hypotheses.md s2-s27, both Judge rulings, escalation text); read decisions.md:755-EOF — the OWNER-ESCALATION block at 761-776 ends with its References line and nothing follows; fresh gradient: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. the byte-proven n1 form judge-fail-0717-1708.c 44,478 B)
- result: No ruling appended — escalation remains the final, unruled entry; the s10 partition stands unrebutted: compositions (F1 slack + r13 label removal, permuter finds + structural respelling, split-init + arm reshape) all canonicalize inside the three basins; the only merged attack remaining IS the escalation itself — option (a) n1 re-proposal or option (b) owner rule-disposition
- verdict: KILLED

## [s29] The mandated structural axis (block-local splits, decl order, type narrowing, statement re-association) holds an unmeasured lever for the last li-13 word, or the owner has ruled the 2026-07-18 escalation since s28, reopening a grindable axis
- mechanism: Any structural respelling must break find_cross_jump's counted-match + stream-1 label-bonus merge of the two [set13; j] sites without paying bytes (s6 BB2_XJUMP_DEBUG trace: the merge is guaranteed for any byte-matching compile; s7 trace: the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class). An owner ruling appended after decisions.md:776 would supersede the parked state
- probe: First-hand verification this session: decisions.md tail read — the OWNER-ESCALATION block at 761-776 still ends the file with its References line, no ruling follows; structural kills re-audited (r1-r13 s2/s3/s3b sweeps, F1 slack-transplant negative, m7/r13 label-family negatives, s11/s12/s20/s21 re-verifications, 17:34 Judge certification 'no honest lever remains unmeasured'); fresh gradient: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU — identical to banked s14/s27/s28; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. the byte-proven n1 form judge-fail-0717-1708.c 44,478 B)
- result: No ruling appended — escalation remains the final, unruled entry; every structural spelling in the codegen-technique-index catalog applicable to the 13-pair wall is banked KILLED, so the axis is dead by Judge-certified exhaustion and re-measuring it would be prohibited spin while the escalation is on file
- verdict: KILLED

## [s29] The mandated structural axis (block-local splits, declaration order, type narrowing, statement re-association) holds an unmeasured lever for the last li-13 word, or the owner has ruled the 2026-07-18 escalation since s28, reopening a grindable axis
- mechanism: Any structural respelling must break find_cross_jump's counted-match + stream-1 label-bonus merge of the two [set13; j] sites without paying bytes (s6 BB2_XJUMP_DEBUG trace certifies the merge is guaranteed for any byte-matching compile; s7 trace certifies the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class). An owner ruling appended after decisions.md:776 would supersede the parked state.
- probe: First-hand verification this session: decisions.md tail read (the OWNER-ESCALATION block at 761-776 still ends the file with its References line, no ruling follows); structural kills re-audited (r1-r13 s2/s3/s3b sweeps, F1 slack-transplant negative, m7/r13 label-family negatives, s11/s12/s20/s21 re-verifications, the 17:34 Judge certification 'no honest lever remains unmeasured'); fresh gradient: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. the byte-proven n1 form judge-fail-0717-1708.c 44,478 B).
- result: No ruling appended — the escalation remains the final, unruled entry; every structural spelling in the codegen-technique-index catalog applicable to the 13-pair wall is banked KILLED, so the axis is dead by Judge-certified exhaustion and re-measuring it would be prohibited spin while the escalation is on file.
- verdict: KILLED

## [s30] The mandated structural axis (block-local splits, declaration order, type narrowing, statement re-association) holds an unmeasured lever for the last li-13 word, or the owner has ruled the 2026-07-18 escalation since s29, reopening a grindable axis
- mechanism: Any structural respelling must break find_cross_jump's counted-match + stream-1 label-bonus merge of the two [set13; j] sites without paying bytes (s6 BB2_XJUMP_DEBUG trace certifies the merge is guaranteed for any byte-matching compile; s7 trace certifies the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class). An owner ruling appended after docs/grind/decisions.md:776 would supersede the parked state.
- probe: First-hand verification this session (s30): decisions.md confirmed 776 lines with the OWNER-ESCALATION block at 761-776 ending the file at its References line, no ruling follows; structural kills re-audited on the ledger (r1-r13 s2/s3/s3b sweeps all KILLED, F1 slack-transplant negative, m7/r13 label-family negatives, s11/s12/s20/s21/s29 re-verifications, 17:34 Judge certification 'no honest lever remains unmeasured'); fresh gradient: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. the byte-proven n1 form judge-fail-0717-1708.c 44,478 B).
- result: No ruling appended — the escalation remains the final, unruled entry; every structural spelling in the codegen-technique-index catalog applicable to the 13-pair wall is banked KILLED, so the axis is dead by Judge-certified exhaustion and re-measuring it would be prohibited spin while the escalation is on file.
- verdict: KILLED

## [s31] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion has been ruled by the owner since s30, reopening a grindable axis
- mechanism: An owner ruling appended after docs/grind/decisions.md:776 would supersede the parked state and select option (a) sanction-the-family or (b) refuse-the-family
- probe: Read decisions.md:740-EOF this session (s31): the OWNER-ESCALATION block at lines 761-776 ends the file at its References line; nothing follows
- result: No ruling entry follows the escalation; it remains the final, unruled entry, still marked 'awaiting owner ruling - do not self-resolve'
- verdict: KILLED

## [s31] The mandated permuter axis still holds an unexplored basin that could surface an honest byte-neutral spelling breaking the case-9/11 find_cross_jump merge
- mechanism: A fresh seed matters only if some C-reachable spelling breaks jump2's counted-match + stream-1 label-bonus merge of the two [set13; j] blocks without bytes; the s6/s7 BB2_XJUMP_DEBUG traces certify the merge is guaranteed for any byte-matching compile and the only byte-free breaker is the Judge-banned USE/CLOBBER class
- probe: Ledger audit instead of a campaign (re-launching would re-measure a banked-dead axis, forbidden while the escalation is live): s4 merged-seed campaigns (112,618 + 2,049 iterations; only false-zero and semantic-theft finds below the merge penalty; scorer metric-invalid on this wall) and s5 unmerged r13-seed campaign (54,087 iterations, 20-min dry plateau at 76-91 word diffs) banked with rejected/ forms; s13/s14/s22/s23 re-confirmations; fresh gradient this session: sandbox --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU - identical to banked s14/s27-s30; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. judge-fail-0717-1708.c 44,478 B)
- result: Permuter axis confirmed dead two-sided; both basins closed with verified raw-word evidence; the only remaining movers are the two owner acts in the filed escalation. Artifact: tmp/grind/motion_SetMotion/s31/verification.md
- verdict: KILLED

## [s31] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion has been ruled by the owner since s30, reopening a grindable axis
- mechanism: An owner ruling appended after docs/grind/decisions.md:776 would supersede the parked state and select option (a) sanction-the-family or (b) refuse-the-family
- probe: Read decisions.md:740-EOF this session (s31): the OWNER-ESCALATION block at lines 761-776 ends the file at its References line; no ruling entry follows
- result: Escalation remains the final, unruled entry, still marked 'awaiting owner ruling - do not self-resolve'
- verdict: KILLED

## [s31] The mandated permuter axis still holds an unexplored basin that could surface an honest byte-neutral spelling breaking the case-9/11 find_cross_jump merge
- mechanism: A fresh seed matters only if some C-reachable spelling breaks jump2's counted-match + stream-1 label-bonus merge of the two [set13; j] blocks without bytes; the s6/s7 BB2_XJUMP_DEBUG traces certify the merge is guaranteed for any byte-matching compile and the only byte-free breaker is the Judge-banned USE/CLOBBER class
- probe: Ledger audit instead of a campaign (re-launching would re-measure a banked-dead axis, forbidden while the escalation is live): s4 merged-seed campaigns (112,618 + 2,049 iterations; only false-zero swapped-jump-targets and semantic-theft finds below the merge penalty, zero honest finds, scorer metric-invalid on this wall) and s5 unmerged r13-seed campaign (54,087 iterations, 20-min dry plateau at 76-91 word diffs, chassis structurally short one li13) both banked with rejected/ forms; s13/s14/s22/s23 each re-confirmed the axis dead; fresh gradient this session: sandbox --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU - identical to banked s14/s27-s30 measurements
- result: Permuter axis confirmed dead two-sided; both basins (merged and unmerged sides of the merge boundary) closed with verified raw-word evidence; the only remaining movers are the two owner acts in the filed escalation
- verdict: KILLED

## [s32] The filed OWNER-ESCALATION for motion_SetMotion is still open and unruled, making owner-gated the mandated disposition for this session.
- mechanism: The contract permits owner-gated only when the escalation exists in docs/grind/decisions.md AND every remaining sanctioned axis is measured dead; the driver otherwise rejects the claim.
- probe: Read docs/grind/decisions.md:761-776 first-hand this session and checked the file tail: the OWNER-ESCALATION entry (filed by grind s7, 2026-07-18, both options presented, community evidence NONE FOUND stated plainly) is the FINAL entry — no owner ruling follows it.
- result: Escalation verified on file and unruled. Permuter axis dead per banked s4/s5 measurements (three campaigns, zero honest finds, scorer metric-invalid: score-0 find had swapped jump targets = 6 real word diffs) and the s6 BB2_XJUMP_DEBUG pass-level trace confirming the USE/CLOBBER class is the only byte-free C-reachable iter1-breaker. Sessions s22, s23, s31 already banked this same permuter-axis determination.
- verdict: CONFIRMED

## [s33] The owner may have ruled on the filed OWNER-ESCALATION since s32, reopening a grind axis
- mechanism: An OWNER RULING entry appended after docs/grind/decisions.md:761-776 would supersede the parked state and either sanction option (a) (re-propose the banked n1 form) or refuse via option (b)
- probe: Enumerated all '## ' headings in docs/grind/decisions.md this session: file is 776 lines; the final heading is line 761, the 2026-07-18 motion_SetMotion OWNER-ESCALATION marked 'awaiting owner ruling - do not self-resolve'; nothing follows it
- result: Escalation remains the final entry and unruled; both frontier items are owner acts unreachable by any grind modality
- verdict: KILLED

## [s33] The mandated forensics modality has an unmeasured deliverable left (an unnamed GCC pass/decision producing the divergence)
- mechanism: Forensics = instrumented-cc1 RTL dumps naming the exact pass and decision; if any divergence mechanism were still unnamed, a new dump session could bank it
- probe: Ledger audit: s6 BB2_XJUMP_DEBUG white-box traces (honest/committed/r13 TUs) name the exact win path (jump2 find_cross_jump chain-partner attempt: one counted set13 match at jump.c:2528 + stream-1 CODE_LABEL bonus at 2406-2410) and the committed rule's mechanism (iteration-1 set12-vs-set13 pattern mismatch at jump.c:2469 before the label walk); s7 traced the n1 union form end-to-end (CLOBBER stream-desync GET_CODE break, both directions); F1 (-1-pair sched2-slack protection) confirmed in-trace
- result: Every divergence mechanism on this wall is already named at pass level with banked artifacts (tmp/grind/motion_SetMotion/s6/, s7/); the forensics deliverable exists in full; the brief forbids re-measuring dead axes when the filed escalation exists
- verdict: CONFIRMED

## [s34] The owner has ruled on the 2026-07-18 OWNER-ESCALATION since s33, reopening a grindable axis
- mechanism: An OWNER RULING entry appended after decisions.md:776 would supersede the parked state and reset the frontier
- probe: Read docs/grind/decisions.md tail past line 776 (requested through 799; file ends at 777) plus the full escalation block at 761-776
- result: The OWNER-ESCALATION remains the FINAL entry; no owner ruling follows it as of s34 (2026-07-18)
- verdict: KILLED

## [s34] The mandated forensics modality has an unmeasured deliverable left (exact GCC pass + decision producing the divergence)
- mechanism: Instrumented-cc1 RTL/pass tracing could name a new mechanism only if the closure theorem left a gap
- probe: Ledger audit: s6 traces name jump2 find_cross_jump counted-match + stream-1 label-bonus (jump.c:2469/2532) as the merge path and the committed rule as a first-iteration set12-vs-set13 pattern-mismatch manufacturer; s7 trace names the n1 CLOBBER stream-desync GET_CODE break both directions; artifacts banked under tmp/grind/motion_SetMotion/s6/ and s7/
- result: The forensics deliverable already exists at pass level with nine independent closure-theorem confirmations; the brief forbids re-measuring dead axes when the filed escalation exists
- verdict: CONFIRMED

## [s35] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion has been ruled by the owner since s34, reopening a grindable axis
- mechanism: An owner ruling appended after docs/grind/decisions.md:776 would supersede the parked state and select option (a) sanction-the-family or (b) refuse-the-family
- probe: Read decisions.md:755-EOF this session (s35): the OWNER-ESCALATION block at lines 761-776 ends with its References line at 776 and the file ends at 777; no ruling entry follows
- result: Escalation remains the final, unruled entry, still marked 'awaiting owner ruling - do not self-resolve', presenting both options with community evidence stated plainly as NONE FOUND
- verdict: KILLED

## [s35] The mandated rederive axis (fresh m2c / decomp.me corpus / sibling-Kengo transplant producing a structurally different C shape) holds an unmeasured honest lever for the last li-13 word
- mechanism: The closure theorem quantifies over ALL byte-matching compiles, not source spellings: any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing find_cross_jump's counted-match + stream-1 label-bonus merge (s6 BB2_XJUMP_DEBUG trace); the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class (s7 trace). A rederived chassis is a C shape and cannot leave the s10 three-basin partition (merged / unmerged-with-byte-cost / banned byte-free breaker); slog-kengo-dead-end additionally rules out Kengo transplant material for this file family
- probe: First-hand ledger + decisions.md verification this session plus a fresh gradient measurement: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU - byte-for-byte identical to the banked s14/s27-s34 measurements; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. the byte-proven n1 form judge-fail-0717-1708.c 44,478 B)
- result: Rederive axis confirmed dead by construction per the s8/s9/s17/s18/s26/s27 kills and the 17:34 Judge certification that no honest lever remains unmeasured; producing another C shape would re-measure a dead axis, which the brief forbids while the escalation is on file
- verdict: KILLED

## [s36] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion has been ruled by the owner since s35, reopening a grindable axis
- mechanism: An owner ruling appended after docs/grind/decisions.md:776 would supersede the parked state and select option (a) sanction-the-union-CLOBBER-family or (b) refuse-the-family
- probe: Read decisions.md:740-EOF this session (s36): the OWNER-ESCALATION block at lines 761-776 ends the file with its References line; no entry follows it
- result: No ruling appended; the escalation remains the final, unruled entry, still marked 'awaiting owner ruling - do not self-resolve'
- verdict: KILLED

## [s36] The mandated rederive axis (fresh m2c / decomp.me corpus / sibling-Kengo transplant producing a structurally different C shape) holds an unmeasured honest lever for the last li-13 word
- mechanism: The Judge-certified closure theorem quantifies over ALL byte-matching compiles, not source spellings: any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing find_cross_jump's counted-match + stream-1 label-bonus merge (s6 BB2_XJUMP_DEBUG trace); the only byte-free C-reachable breaker is the Judge-banned USE/CLOBBER class (s7 end-to-end trace). A rederived chassis is a C shape and cannot leave the s10 three-basin partition (merged / unmerged-with-byte-cost / banned byte-free breaker); slog-kengo-dead-end additionally rules out Kengo transplant material for this file family
- probe: First-hand ledger + decisions.md verification plus a fresh gradient measurement: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU - byte-for-byte identical to the banked s14/s27/s28/s29 measurements; bank verified intact (candidate.c 44,155 B; 9 rejected forms incl. the byte-proven n1 form judge-fail-0717-1708.c 44,478 B)
- result: Rederive axis confirmed dead by construction per the s8/s9/s17/s18/s26/s27/s35 kills and the 2026-07-17 17:34 Judge certification that no honest lever remains unmeasured; producing another shape would re-measure a dead axis, which the brief forbids while the escalation is on file
- verdict: KILLED

## [s37] A synthesis pass over the full ledger (evidence s1-s36, hypotheses, 9 rejected forms) can produce a merged attack that escapes the s10 three-basin partition
- mechanism: Cross-axis composition of banked levers (F1 sched2-slack, r13 chassis, permuter finds, split-init family, label placement) into a spelling not yet measured
- probe: Re-read the entire ledger; check every banked lever against the closure theorem's jump2-time RTL invariants (any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing the counted-match + label-bonus merge; only byte-free iter1-breaker is the USE/CLOBBER class)
- result: No composition escapes: the theorem quantifies over all byte-matching compiles, not source spellings, so every synthesis output canonicalizes into basin 1 (merged, wrong bytes), basin 2 (real-insn cost, wrong bytes), or basin 3 (USE/CLOBBER manufacture, banned by binding Judge constraints). Nine independent confirmations banked incl. two BB2_XJUMP_DEBUG white-box traces (s6, s7). Frontier already reset by s10 to the two owner acts.
- verdict: KILLED

## [s37] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion is on file and still unruled, and every remaining sanctioned axis is measured dead
- mechanism: owner-gated validity per the session contract requires the filed escalation plus all-axes-dead on the ledger
- probe: Read docs/grind/decisions.md tail this session; verify ledger bank (candidate.c + rejected/) on disk
- result: CONFIRMED: the escalation (filed by grind s7 per the 2026-07-17 17:34 Judge disposition) occupies lines 761-776 and is the FINAL entry, no owner ruling follows; all five ladder axes (structural s2/s3/s3b r1-r13, permuter s4/s5 168,754 iters two-sided, forensics s6/s7 white-box traces, rederive s8/s9 dead by construction, synthesis s10 partition) are measured dead; candidate.c (44,155 bytes) and all 9 rejected forms verified present
- verdict: CONFIRMED

## [s38] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion remains on file and unruled, so the owner-gated exit condition holds for session 38.
- mechanism: Per the brief, owner-gated requires a filed OWNER-ESCALATION in docs/grind/decisions.md plus every remaining sanctioned axis measured dead; the escalation was filed by grind s7 per the 2026-07-17 17:34 Judge disposition and is marked do-not-self-resolve.
- probe: Read docs/grind/decisions.md:755-776 and the file tail this session: the OWNER-ESCALATION block is the FINAL entry and the file ends with its References line - no owner ruling follows it as of 2026-07-18 s38.
- result: Escalation present and unruled; both mutually exclusive owner options (a) sanction the union-constructor CLOBBER family / (b) refuse and sign a disposition for regfix.txt:2190 stand open; community-evidence absence stated plainly in the entry.
- verdict: CONFIRMED

## [s38] The mandated structural axis has no unmeasured lever left; re-measuring it would only re-derive banked kills.
- mechanism: Ledger: r1-r4 (s2), r5-r9 (s3), r10-r13 (s3b) all KILLED - every structural spelling either canonicalizes to the merged 361-insn shape or pays real bytes; the s10 three-basin partition quantifies over ALL byte-matching compiles; the 17:34 Judge ruling certifies exhaustion complete with F1 discharged; binding Judge constraints forbid the only byte-free breaker (USE/CLOBBER manufacture).
- probe: Verified the rejected bank on disk (9 forms including structural-13pair-axis-closed.c, s3-structural-sweep-goto-reorder-thunk-splitinit-hoist-dead.c, s3b-dupcase-ternary-u32-preswitch-if-dead.c) and the evidence.md axis-kill entries s2/s3/s3b/s10.
- result: Structural axis measured dead; the closure theorem carries nine independent confirmations including two BB2_XJUMP_DEBUG white-box traces (s6, s7); no honest structural spelling remains unmeasured.
- verdict: CONFIRMED

## [s39] An unmeasured structural spelling class for the 13-pair wall exists outside the banked r1-r13 sweep, making structural work still grindable this session.
- mechanism: The structural catalog (block-local splits, declaration order, type narrowing, re-association, case duplication/reorder, goto-sharing, cond-expr, pre-switch extraction) would have to contain a spelling that survives to jump2 unmerged byte-neutrally.
- probe: Re-read memory/grind/motion_SetMotion/hypotheses.md and docs/grind/decisions.md:755-776 first-hand this session; cross-checked every structural family in the codegen-technique-index against the banked KILLED set and the binding Judge constraints.
- result: No unmeasured class exists: r1-r13 cover the full catalog and are all KILLED (merged 361-insn canonical shape or real byte cost, e.g. r13 +6 insns); the sanctioned split-init family is specifically measured dead (cse folds before jump2); the closure theorem (any byte-matching compile forces both 13-sites to [set13; j] with adjacent CODE_LABEL, merge guaranteed, USE/CLOBBER the only byte-free breaker) holds with eight independent confirmations including the s6 BB2_XJUMP_DEBUG pass-level trace. The sole remaining lever is the union-CLOBBER family, forbidden by binding Judge constraints pending the owner ruling.
- verdict: KILLED

## [s40] The filed OWNER-ESCALATION for motion_SetMotion remains the final, unruled entry in docs/grind/decisions.md
- mechanism: owner-gated requires the escalation to exist and be unruled; only the owner can advance either option (a) sanction the union-constructor CLOBBER family or (b) refuse and sign a disposition for regfix.txt:2190
- probe: Read docs/grind/decisions.md tail this session (s40): the 2026-07-18 OWNER-ESCALATION block occupies lines 761-776, the file ends at line 777, and no OWNER RULING entry follows it
- result: Escalation present, final, and unruled as of 2026-07-18 s40
- verdict: CONFIRMED

## [s40] The mandated permuter axis is already measured dead and a fresh campaign cannot produce a sanctioned honest find
- mechanism: s10 three-basin partition: every honest chassis is in the merged basin (wrong bytes) or the unmerged-with-cost basin (>=2 insns / 76+ word diffs); the only byte-free breaker is the Judge-banned USE/CLOBBER family. s4's 112,618-iter random + 2,049-iter directed campaigns (merged seeds) yielded only false-zero (scorer-blind, 6 real word diffs) and semantic-theft sub-100 finds; s5's 54,087-iter unmerged-r13-seed campaign plateaued at 82 raw word diffs; the s6 BB2_XJUMP_DEBUG trace certifies the closure theorem at pass level
- probe: Ledger review per the brief's no-respin rule: rejected/s4-permuter-false-zero-swapped-jump-targets.c, rejected/s4-permuter-semantic-theft-store-into-case911.c, rejected/s5-permuter-unmerged-r13-seed-plateau.c all banked; permuter-modality sessions s13/s14/s22/s31/s32 independently re-verified the same closure
- result: Both basins measured dry; no permuter chassis class remains; brief forbids re-measuring a dead axis when the filed escalation exists
- verdict: CONFIRMED

## [s41] A new permuter campaign this session could surface a novel byte-free iter1-breaker for the 13-pair cross-jump merge
- mechanism: find_cross_jump merge at the [li s0,13; j sel_dispatch] pair is guaranteed by the counted-match + label-bonus path per the s6 BB2_XJUMP_DEBUG trace; the closure theorem (8 independent confirmations) proves the only C-reachable byte-free breaker is the unsanctioned USE/CLOBBER class
- probe: Ledger re-verification instead of re-launch: three prior campaigns from both sides of the merge boundary (s4 112,618 iters, s4b 2,049 iters, s5 54,087 iters) plus banked rejected forms s4-permuter-false-zero-swapped-jump-targets.c, s4-permuter-semantic-theft-store-into-case911.c, s5-permuter-unmerged-r13-seed-plateau.c
- result: Axis already measured dead two-sided; Judge constraints bind any USE/CLOBBER-manufacture respelling a campaign could find; OWNER-ESCALATION at decisions.md:761-776 verified present, final entry, unruled
- verdict: KILLED

## [s42] The forensics axis (instrumented-cc1 pass attribution of the 1-word divergence) still contains an unmeasured question for motion_SetMotion.
- mechanism: If any RTL/jump2/xjump-debug measurement were missing, a new dump could name a new pass/decision and reopen an honest lever.
- probe: Re-verified first-hand this session: docs/grind/decisions.md:761-776 is the final entry for the function (no owner ruling after it) and its Exhaustion + s7 white-box sections already carry the complete pass-level attribution: honest form merges via jump.c find_cross_jump 1-counted-match + stream-1 label bonus (s6 trace); the union n1 form breaks iteration 1 via byte-free CLOBBER stream desynchronization at the GET_CODE check in both directions (s7 trace, artifacts tmp/grind/motion_SetMotion/s6/ and s7/). Judge constraints bind: the only remaining C-reachable breaker is the unsanctioned USE/CLOBBER family, which must not be probed or committed.
- result: No unmeasured forensic question exists; any new cc1 dump would re-measure a Judge-certified-dead axis, which the brief forbids.
- verdict: KILLED

## [s43] The forensics deliverable (exact GCC pass + decision producing the divergence) is already banked, so no new instrumented-cc1 run can add information
- mechanism: s6 BB2_XJUMP_DEBUG traces named the merge path (jump2 find_cross_jump: one counted set13 match + stream-1 CODE_LABEL bonus), the committed rule's mechanism (set12-vs-set13 first-iteration pattern mismatch at jump.c:2469 before the label walk), and F1 (sched2 slack-hoisting protects the -1 pair); s7 traced the n1 union form's CLOBBER stream-desync GET_CODE break both directions
- probe: Verified fresh this session: decisions.md tail read end-to-end (776 lines; escalation block 761-776 is final, no OWNER RULING follows); ledger bank checked on disk (candidate.c 44,155 bytes; 9 rejected forms present including judge-fail-0717-1708.c)
- result: Escalation on file and unruled; forensics artifacts banked under tmp/grind/motion_SetMotion/s6/ and s7/ per the escalation's References section; binding Judge constraints forbid the only remaining C-reachable mechanism (USE/CLOBBER manufacture family)
- verdict: CONFIRMED
