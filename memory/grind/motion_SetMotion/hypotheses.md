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
