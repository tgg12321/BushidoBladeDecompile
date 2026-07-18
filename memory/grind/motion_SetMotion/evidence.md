# Evidence bank — motion_SetMotion

- WIP rejected_form: {'form': 'case 9: case 11: sel = 0xD; (with rule removed)', 'score': 0, 'reason': 'WRONG: full build SHA1 mismatch 82e50f76. Cross_jump fires when both case9/11 and D_800A3207==3 non-10 have sel=0xD â€” minimum-2 (`li s0,13; j sel_dispatch`) met â†’ blocks merged â†’ different binary.'}

- WIP rejected_form: {'form': 'swap declaration order (sel2 before result)', 'score': 9, 'reason': "Masked-sandbox 11->9 noise; RA swap persisted. Tiebreaker (allocno order) doesn't help since priorities differ (300 vs 242), not equal."}

- WIP rejected_form: {'form': 'pins removed + regfix kept', 'score': 0, 'reason': 'PROVEN this session: SHA1 46e36200 (RA mismatch). resultâ†’$s2 (wrong) without pins.'}

- WIP rejected_form: {'form': 'func_8006BEC4(0xA, sel2) instead of func_8006BEC4(0xA, -1) at early exit', 'score': 0, 'reason': "Would add 4th ref to sel2 (raising priority to 645 > result's 300), but changes binary: oracle has `li a1, -1` at dcc NOT `move a1, s2`. Cannot use this lever."}

- == imported from memory/wip notes.md ==
# motion_SetMotion (code6cac_c_mid.c) — WIP, Wall 1 SOLVED, Wall 2 open

## TL;DR (2026-07-01)
The coupled wall split and half fell. **Wall 1 (RA priority) is SOLVED
in pure C** — both pins retired (commit: cheat-cleanup 2026-07-01); full
SHA1 == oracle with the 1 remaining rule applied. **Wall 2 (cross-jump
merge) remains**: 1 regfix rule (case-9/11 sel 12→13 subst @
regfix.txt:2484). Masked sandbox is BLIND here — full SHA1 only.

## Wall 1 recipe (landed — reusable for the cluster)
Duplicate load_sel2's `sel2 = D_800A3350;` into case 0 (FAKE-annotated)
and move the label to case 13/17's copy. Flow counts the REAL second def
(sel2 reg_n_refs 3→4 → pri 851 > result 412) → RA lands sel2→$s2 /
result→$s3 naturally; jump2 cross-jump re-merges the identical
[lbu; j] tails → zero byte delta. **Label placement steers merge
direction**: label on case 13/17 ⇒ cross-jump rewrites case 0's copy ⇒
target layout exactly. Mirror labeling (m6) emits a 4-diff flipped
layout. Measured this session: dead stores are INERT for global RA
(m1: cse folds const re-stores; m3: `sel2 = v0` deleted by flow WITHOUT
being counted — 2.7.2 counts refs post-deletion). Real duplication is
the working spelling.

## Wall 2 — cross-jump no-merge (open)
Target keeps BOTH `[li s0,13; j sel_dispatch]` sites (==3 arm at ea0 as
j-delay form; case 9/11 at fc0). Writing 0xD in C merges them (SHA1
82e50f76). Ruled out this session:
- consecutive-label aliasing (sel_dispatch2:) — jump1 unifies
  same-address labels, then merge fires ANYWAY and deletes 2 insns (m7).
- ==3 arm reshaped to the two-goto form (`sel=0xF; if(==10) goto disp;
  sel=0xD; goto disp;`) matching target's delay-slot structure — still
  merges (m8).
**REG_DEAD lead REFUTED (2026-07-02):** the death-note comparison in
find_cross_jump is `#ifdef STACK_REGS` only (jump.c:2436-2467) — MIPS
never compares them. **Pairing algorithm fully mapped** (jump.c:
1966-2001 + 2371-2533): for each simplejump, (a) minimum=1 attempt vs
code-before-its-own-label, (b) minimum=2 attempts vs every other jump
in the SAME label's jump_chain. Stream-1 hitting a CODE_LABEL does
`--minimum; break` — so a [jtbl-label; set13; j] block as stream 1
gets 1 match + label bonus = merge GUARANTEED against any same-label
[set13; j] partner. ⇒ the original compile must have had the two 0xD
jumps targeting DIFFERENT CODE_LABELs at jump2 time (labels are free
in bytes — they coalesce at assembly). Remaining lever family: a
second label for case 9/11's goto that is NOT adjacent to sel_dispatch
(adjacent → m7 showed unification/merge). Known cost problem: any
real-code separation between the labels either emits bytes or gets
cross-jump-merged itself leaving a thunk `j` (+1 insn). UNSOLVED
puzzle: what C makes GCC keep two labels apart through jump1's
tensioning yet emit them at the same final address. Also note: in the
CURRENT source's cc1 output the ==3 arm's 0xD/0xF sets are NOT visible
as separate li's near the dispatch (only case-10's 13 at a bne delay)
— map the arm's actual emission before theorizing further.

## Ruled out (session 1-2, do not re-derive)
- Declaration-order swap (masked noise, swap persisted).
- `func_8006BEC4(0xA, sel2)` at early exit (bytes: li a1,-1 vs move).
- Real 4th-ref forms that add bytes; livelen restructures (init-later).

## Pointers
- tmp/probe_msm.py (m0-m9 probe harness; regenerate from git if gone).
- Wall-1 mechanics: dead-store-fake-exception.md (chain-extender note),
  register-alloc-pure-c.md, register-alloc-deep-dive.md.
- Cluster siblings to apply the Wall-1 recipe to: saEft00Add,
  marionation_Exec, cpu_side_move_dir_4 (local-alloc variant — see its
  WIP), func_8007C97C.


- [s1] [fable-blitz 2026-07-07] Current state: 0 pins (Wall 1 SOLVED 2026-07-01 via duplicated-statement-into-arms - sel2->s2/result->s3 natural, sel2-store duplicated into case 0, label on case 13/17's copy), exactly 1 rule left: regfix.txt:2483-2484 `subst "addiu\t$16,$zero,12" -> 13 @ 149` on the case-9/11 site - a semantic-lie value subst (C says sel=0xC, binary says 0xD) whose sole purpose is preventing the jump2 cross-jump merge. Queue distance 10 is masked noise: the honest-0xD form scores masked 0 but full SHA1 = 82e50f76 != oracle. Only full-build SHA1 / raw function byte-diff are valid metrics on this wall.

- [s1] [fable-blitz 2026-07-07] The wall's target topology (asm/funcs/motion_SetMotion.s): TWO identical simplejump blocks [j .L80038EDC; delay addiu s0,0xD] - the ==3 arm at L85-86 and case-9/11's jtbl block .L80038EC8 at L170-172 - jumping to the SAME label. Writing 0xD honestly in both arms merges them (WIP m1, SHA1 82e50f76, 2 insns deleted). The ==3 arm's shape [beq s1,v0,.L80038EDC delay li s0,0xF; j .L80038EDC delay li s0,0xD] (L83-86) corresponds to the two-goto C `sel=0xF; if(v0==10) goto disp; sel=0xD; goto disp;` - m8 measured that form: still merges.

- [s1] [fable-blitz 2026-07-07] THE -1-PAIR PARADOX (new this recon, the key evidence): target ALSO contains a second byte-identical same-label pair - case 8 .L80038E10 [j .L80038EDC; delay addiu s0,-1] (L119-120) and the case-10 D_800A3350!=0 subpath (L158-159, same two insns). Our committed C writes BOTH honestly (`sel = -1; goto sel_dispatch;` at src/code6cac_c_mid.c:999 and :1020-1021) and the build BYTE-MATCHES with both blocks unmerged. So the original's blocking condition is already reproduced by our C at the -1 sites while absent at the 13 sites. The discriminating feature between our -1 blocks and our (honest-0xD) 13 blocks in the SAME RTL dump IS the transplantable lever.

- [s1] [fable-blitz 2026-07-07] find_cross_jump fully mapped from source (tools/gcc-2.7.2/jump.c:2371-2533): stream1 (before e1, the jump being processed) walks prev_nonnote_insn - labels NOT skipped; hitting a CODE_LABEL does `--minimum; break` (2406-2410) = the DIRECTIONAL label bonus. Stream2 (before the partner) skips notes AND labels (2392-2393). Chain-partner attempts start at minimum=2 (1993); each rtx_renumbered_equal_p-matched real insn decrements (2528); USE/CLOBBER patterns match on the win path but do NOT count (2524-2529); first pattern mismatch (GET_CODE or rtx_renumbered_equal_p, 2469-2470) breaks matching. Merge iff minimum<=0 AND at least one counted match (2532: last1 != 0). The jump_chain contains ONLY simplejumps (jump.c:217); partner walk is first-match-wins (1986-1994); do_cross_jump has NO additional guards (2536-2583) - it deletes the e1-side matched insns and redirects e1's jump into the partner's copy.

- [s1] [fable-blitz 2026-07-07] Model VALIDATED against landed bytes: the Wall-1 sel2-tail merge replays exactly under this trace - e1 = case-13/17's j (its lbu preceded by the load_sel2 label -> 1 match + label bonus -> minimum 0), the 13/17-side copy is deleted and its jump redirected into case 0's copy = target's .L80038DE0 [j .L80038DD4; delay li s0,6] with .L80038DD0 falling into .L80038DD4 [lbu s2; j]. Mechanically confirms 'label placement steers merge direction': the label-bonus side is the DELETED side. CONSTRAINT for any Wall-2 lever: it must be 13-pair-LOCAL - suppressing cross-jump globally would unmerge the sel2 tails and break the landed Wall-1 byte shape.

- [s1] [fable-blitz 2026-07-07] By the same validated model the -1 pair MUST merge (e1 = case-8's j: iter1 set-1 == set-1 counts 1, iter2 prev_nonnote = the jtbl CODE_LABEL -> bonus -> minimum 0 -> merge) - yet it does not merge, in our build NOR in the original. Therefore a real blocking condition sits outside jump.c-as-read: candidates visible in one dump = pattern divergence at jump2 time (one set-1 not `(set (reg s0) (const_int -1))`), an intervening real/byte-free insn adjacent to one jump post-sched2, REG-note structure, or the jumps not actually sharing a JUMP_LABEL rtx.

- [s1] [fable-blitz 2026-07-07] Two-label route CLOSED by source: mark_jump_label's consecutive-label canonicalization (jump.c:3177-3193) keeps labels separated by NOTE_INSN_LOOP_BEG distinct at jump1 (cross_jump=0, lines 3189-3192), but jump2 re-runs it with cross_jump=1 where ALL notes AND USE/CLOBBER insns are skipped (3183-3188) - so byte-free-separated labels UNIFY at exactly the pass that merges, and any real separator insn emits bytes. m7's failure (consecutive labels unified, merge fired anyway) is fully explained; NO byte-free label-separation spelling exists. The WIP's conclusion 'the original must have had two different CODE_LABELs at jump2 time' is therefore almost certainly WRONG - the -1 pair proves a different protection mechanism exists.

- [s1] [fable-blitz 2026-07-07] Pass-order facts: the ONLY cross-jump-enabled call is jump_optimize(insns, 1, 1, 0) at toplev.c:3142, AFTER sched2 (toplev.c:3125) and BEFORE reorg/final - so suffix adjacency at merge time is post-sched2 order; USE/CLOBBER insns still exist at that point, emit nothing at final, and reorg's backward delay-slot scan skips them - a [set13; CLOBBER; j] block still yields target's [j; delay set13] shape while breaking find_cross_jump's iter1 from BOTH directions (stream2 skips only notes/labels, so the CLOBBER is compared and GET_CODE(SET) != GET_CODE(CLOBBER) breaks at 2469 before any label bonus).

- [s1] [fable-blitz 2026-07-07] Byte-free C-reachable emitters located by grep: (a) standalone `(use return_reg)` emitted for every value-return (stmt.c:2532) - explains marionation's 'cross-jump does NOT fire' note (their 2-insn [move v0,a2; j epi] return tails: the USE matches but doesn't count -> 1 counted match < minimum 2 -> no merge; NOT a global config fact - cross-jump demonstrably fires here per m1 and per the landed sel2-tail merge); not deployable mid-function. (b) bare `(clobber target)` from union-typed constructor expansion (expr.c:2996, 'Inform later passes that the whole union value is dead') and struct-return paths (calls.c:1784) - the C-reachable insertable-break candidate; survival past flow1/local-alloc/reload to jump2 is UNMEASURED.

- [s1] [fable-blitz 2026-07-07] Rejected inventory (do not re-derive): m1 honest-0xD both arms = merge (82e50f76); m7 adjacent second label (sel_dispatch2:) = jump1 unifies, merge fires; m8 two-goto arm reshape = merge; REG_DEAD-note lead REFUTED (death comparison is #ifdef STACK_REGS only, jump.c:2433-2467); func_8006BEC4(0xA, sel2) at the early exit = wrong bytes (target has li a1,-1 at 0x80038CD4, not move a1,s2). Wall-1-era rejecteds (decl-order swap, pins-off+regfix-on 46e36200) are moot - Wall 1 landed. The 2026-06-24 owner-ruling levers: 'single-shared-store letting cross-jump RESPREAD' is impossible by source (do_cross_jump only merges/deletes, never duplicates - jump.c:2536-2583); 'sel2-lifetime + post-early-exit sequencing' was Wall-1-scoped (moot); 'USE/CLOBBER suffix-break' is refined into frontier F2 with the emitter sites now located.

- [s1] [fable-blitz 2026-07-07] do-while(0) (sanctioned 2026-07-06, POSTDATES m7/m8): direct note effects are inert on every comparison surface of this wall by source reading (prev_nonnote_insn at 2389, stream2 note-skip at 2392-2393, label-unification note-skip at 3187-3192) - any wrap effect on cross-jump would be INDIRECT (changed RTL emission of the wrapped arm / sched2 adjacency). Unmeasured on this function; cheap to sweep; low prior.

- [s1] [fable-blitz 2026-07-07] Executor metric warning: the masked sandbox CANNOT see this wall (m1 scored masked 0 while the SHA1 diverged) - every probe verdict needs the full-build SHA1 gate or a raw objdump byte-diff of the function window; sandbox-0 is necessary-not-sufficient and `retire` is the only proof.

- [s2] F1 discriminator (measured 2026-07-17, committed.i.sched2/.jump2 regenerated this session): the -1 pair does not merge because sched2 hoists case-10's set s0<--1 (insn 374) away from its jump — the pre-jump insn is sb D_800A3350 (insn 371) — so find_cross_jump iter1 gets 0 counted matches and the label bonus never applies (jump.c:2532 requires last1!=0). Final bytes are restored by reorg pulling the hoisted li into the delay slot (committed.s: sb; j .L244; delay li $16,-1). The protection is honest sched2 SLACK, not a byte-free insn.

- [s2] The slack mechanism is NOT transplantable to the 13 pair by byte-count: target fixes case-9/11 at exactly 2 text insns [j; delay li13] behind its jtbl label and the ==3 arm at [beq; delay li15; j; delay li13]; every candidate slack insn would add bytes. In any byte-matching compile both pre-jump insns are the identical set s0<-13, iter1 matches, and case-9/11's own jtbl CODE_LABEL gives the stream-1 label bonus (--minimum -> 0) -> merge GUARANTEED.

- [s2] Structural axis on this wall is CLOSED (rejected/structural-13pair-axis-closed.c): arm re-association r1, sense swap r3 (both -> merged + branch-sense flipped, 122 diff lines), s16 narrowing r2 (658 diff lines), block-local const split r4 (folded, merged). Combined with s2's wrap-family negative (NOTEs skipped everywhere) and s1's m7 label-unification negative, the only remaining iter1-breaking RTL class is USE/CLOBBER, whose only C-reachable emitters (union constructor expr.c:2996, struct-return calls.c:1784) are the manufacture family the Judge ruled unsanctioned (decisions.md 2026-07-17 17:09).

- [s2] Sandbox remains blind on this wall (masked 0 while SHA1 diverges, s1 evidence); all probes this session judged by label-normalized full-function cc1 asm diff vs the committed baseline (m0, 363 insns; merged control shape 361).

- [s3] Structural sweep COMPLETE (s3_probe.py, r5-r9, 2026-07-17): r5 cross-case goto-share (case 9/11 goto into the ==3 arm's set13 — GCC does NOT duplicate; jtbl retargeted), r6 source-order case reorder, r7 goto-thunk indirection (jump1 tensions it away), r8 split-init accumulation `sel=0xC; sel+=1;` (the SANCTIONED 2026-06-13 family — cse folds 12+1 to set13 before jump2) — all four produce the IDENTICAL merged 361-insn shape (same 86 label-normalized diff lines as honest-0xD). r9 hoisted-common-set (sel=0xD above the switch, case 9/11 bare goto) is the only last1==0 spelling: the block vanishes entirely (jtbl entries point straight at sel_dispatch) and layout diverges 619 diff lines — byte-count kills the zero-insn side exactly as F1 killed the slack side. Artifacts: tmp/grind/motion_SetMotion/s3/.

- [s3] Closure theorem now sealed from BOTH sides by measurement: a byte-matching compile forces both 13-sites to be exactly [.. ; set13; j] with case-9/11's jtbl CODE_LABEL adjacent -> iter1 counts the set13 match, label bonus fires, merge GUARANTEED (s2 r1-r4 + s3 r5-r8); any spelling that avoids the merge either adds/moves real insns (F1 slack transplant, r9 hoist) or needs a byte-free non-NOTE insn between set13 and j — an RTL class reachable from C only via the union-constructor/struct-return CLOBBER emitters the Judge ruled unsanctioned (decisions.md 2026-07-17 17:09). No honest structural spelling remains unmeasured.

- [s3] Metric note: `sandbox motion_SetMotion --disable all` on the committed form prints score 1 (the single li 12-vs-13 word, rule dropped) — the engine's honest distance; the ledger's floor=10 figure is the legacy queue-weighted number. Both are blind to the merge wall (any honest-0xD form scores 0 there while SHA1 diverges); full-build SHA1 / raw byte-diff remain the only valid verdict gates.

- [s3b] Adversarial re-test of the closure theorem (s3b_probe.py, 2026-07-18) with four spelling classes NOT in the r1-r9/m bank — all KILLED: r10 duplicate case blocks (case 9 and case 11 as separate honest set13 blocks) and r11 cond-expr arm (`sel = (v0!=10) ? 0xD : 0xF;`) both canonicalize to the IDENTICAL merged 361-insn shape (same 86 label-normalized diff lines as the honest-0xD control — cross-jump collapses the 9/11 duplicates first, survivor still merges with the arm); r12 u32 sel is doubly dead (149 diff lines: unsigned vacuates the `sel >= 0` dispatch guard, new [j; move a0,s0] tail, plus the merge); r13 pre-switch if-extraction of 9/11 (removing the jtbl CODE_LABEL = the label-bonus source from the block) pays 2 extra beq/li compare pairs + rewritten jtbl entries (369 vs 363 insns, 126 diff lines) — the label bonus cannot be dodged without real bytes. Closure theorem survives its fourth independent attack; baseline sandbox honest distance re-confirmed at 1 this session. Artifacts: tmp/grind/motion_SetMotion/s3/s3b_*.s + s3b_probe.py.

- [s3] r10 (case 9 and case 11 as two separate honest set13 C blocks) and r11 (sel = (v0!=10) ? 0xD : 0xF) both canonicalize to the IDENTICAL merged 361-insn shape as honest-0xD - the closure theorem's canonicalization claim holds across source-level block fan-out and cond-expr spelling.

- [s3] r13 measured the label-bonus dodge directly: removing the jtbl CODE_LABEL from the 13 block (pre-switch if for cases 9/11) costs 2 compare pairs + jtbl rewrite (369 vs 363 insns) - same byte-count closure as F1-slack and r9-hoist.

- [s3] r12 shows u32 sel is structurally unusable here independent of the wall: unsigned makes the if (sel >= 0) dispatch guard vacuous and restructures the dispatch tail (149 diff lines).

- [s3] Baseline re-confirmed this session: sandbox --disable all on the committed form = honest distance 1 (single li 12-vs-13 word, 1 rule dropped); src restored clean after probes (git status empty).

- [s3] Closure theorem has now survived four independent adversarial rounds (s2 r1-r4, s3 r5-r9, s2 wrap-family, s3b r10-r13); no honest structural spelling remains unmeasured.

- [s4] [s4] Permuter scorer is metric-BLIND on this wall, measured: perm_a output-0-1 scored 0 (with --stack-diffs) yet has 6 real word diffs vs target (swapped beq/j destinations between the ==3 arm and case-9/11). Label renumbering in the diff scorer equates swapped branch targets. Any future permuter score-0 on motion_SetMotion REQUIRES raw instruction-word verification before being believed (tmp/grind/motion_SetMotion/s4/verify_find.sh is the harness).

- [s4] [s4] Calibration: committed.c through the standalone pipeline = exactly 1 real word diff vs target (li s0,12 vs 13 at the case-9/11 site) + 3 constant jtbl-reloc lw words (8c220048/78/a8 vs 8c220000) shared by every compile — the raw-diff noise floor for this workspace.

- [s4] [s4] 112,618-iteration honest-0xD random campaign (13.1h, the discarded prior attempt's perm_a, adopted+harvested+stopped this session): 12 finds, all classified by raw word diff. 100-200-score finds = the merged 361-insn-shape family (119-125 word diffs). ALL sub-100 finds break the merge by moving a REAL insn into a 13-site suffix at the cost of semantics: score-0 moves load_sel2 into the ==3 arm (jump-swap), score-60 moves D_800A3350=0 from case-10's =5 path into case 11, score-120 moves D_800A334C=0x5A into case 9. The blind search independently converged on the closure theorem's boundary: iter1-breaking needs a real insn; every real insn is paid in bytes, position, or semantics.

- [s4] [s4] First measured 'different JUMP_LABELs' unmerge (output-0-1): both li 13 survive at 402 insns because the arm's jump was cross-jump-redirected into the shared lbu block. Honest transplant impossible: both 13-sites must dispatch with sel2=-1 so neither may route through the lbu; and do_cross_jump places the redirect label BEFORE the matched insns, so the jump word only matches target if the matched insns are byte-free (the Judge-banned USE/CLOBBER family). The s1 conjecture 'original had two different CODE_LABELs' is now measured dead on honest terms.

- [s4] [s4] perm_b (directed PERM_GENERAL chassis) stalled at 2,049 iterations over 13h with best find 100 (merged family) — the directed cross-product exhausted early with nothing below the merge penalty; harvested+stopped.

- [s4] [s4] No orphaned permuter processes at session start (driver reaped the discarded attempt's PIDs 416/1520); both campaign_meta.json + campaign.log banked; harvest telemetry recorded to metrics/events.jsonl this session.

- [s4] [s4] Baseline re-confirmed: sandbox motion_SetMotion --disable all = honest distance 1 (402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU); src untouched, git clean.

- [s5] Permuter axis re-attacked from the OPPOSITE side of the merge boundary and measured dead there too: fresh campaign s5-r13-unmerged-seed (54,087 iterations / ~30 min / -j6 --stack-diffs) seeded from the s3b r13 pre-switch-if chassis (the only honest unmerged form measured; base 408 insns / 136 raw word diffs / perm-score 1125). Best find 660 at ~10 min, then 20 min dry -> fresh-seed stop. Best four finds raw-word-verified (harvest_c.sh): 76-91 word diffs, 403-406 insns — nowhere near the 1-word committed noise floor; all retain the chassis family.

- [s5] NEW micro-fact from calibration: target contains THREE 2410000d (li s0,13) words (==3 arm, case-9/11 jtbl block, case-10 0x400040 path) but the r13 chassis emits only TWO — with the jtbl label removed, cross-jump merges the extracted if-block's set13 with the ==3 arm's set13 instead (both non-jtbl [set13; j] blocks, same jump_chain). Removing the label bonus does NOT protect the pair; find_cross_jump just picks a different partner. The closure theorem gains a sixth independent confirmation: the merge fires in EVERY honest chassis measured, differing only in which pair it eats.

- [s5] Permuter closure is now two-sided: s4 (merged seeds: 112,618-iter random + 2,049-iter directed; only false-zero/semantic-theft sub-100 finds) + s5 (unmerged seed: 54,087 iters, plateau at 82 word diffs). No further permuter chassis class exists — every honest form is on one side of the merge boundary or the other, and both basins are measured dry. Remaining frontier is unchanged: the owner-policy/community-evidence research session + optional gccdbg confirmatory trace (Judge 2026-07-17 17:34 disposition).

- [s5] Baseline re-confirmed: sandbox motion_SetMotion --disable all = honest distance 1 (402/402, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU); campaign harvested+stopped (pid 402 dead, telemetry in metrics/events.jsonl); src untouched, git clean.

- [s5] Permuter closure is now two-sided: s4 merged-seed campaigns (112,618-iter random + 2,049-iter directed, only false-zero/semantic-theft sub-100 finds) + s5 unmerged-seed campaign (54,087 iters, plateau 82 word diffs). Every honest chassis sits on one side of the merge boundary; both basins measured dry.

- [s5] Sixth independent confirmation of the closure theorem: the 13-merge fires in EVERY honest chassis measured, differing only in which pair it eats (r13's extracted if-block merges with the ==3 arm when the jtbl label is gone).

- [s5] No OWNER-ESCALATION for motion_SetMotion is filed yet in docs/grind/decisions.md - the 2026-07-17 17:34 entry is a Judge ruling PRESCRIBING the escalation (research session first, then file in hirahira_w_frie format), not the escalation itself; owner-gated is therefore not yet available.

- [s5] Baseline re-confirmed this session: sandbox motion_SetMotion --disable all = honest distance 1 (402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU); campaign harvested+stopped (pid dead, telemetry banked); src untouched, git clean.

- [s6] gccdbg white-box trace LANDED (BB2_XJUMP_DEBUG knob in tools/gcc-2.7.2/jump.c, env-gated, verified codegen-inert; in-tree diagnostic cc1 rebuilt, frozen build/cc1 untouched, faithfulness verified per-TU: knob-unset diagnostic .s == build/cc1 .s on all three TUs; fresh binary copied to tmp/gccdbg/cc1). Full trace + uid maps: tmp/grind/motion_SetMotion/s6/analysis.md + {honest,committed,r13}.xjdbg.log.

- [s6] Honest-0xD TU: the 13-pair merge fires EXACTLY as modeled — e1=408 (case-9/11 jtbl block [label 403; set13 406; j 408]) chain-partner attempt vs e2=223 (arm): one counted MATCH (406/211 set13, min 2->1) then stream-1 LABEL-BONUS at the jtbl CODE_LABEL 403 (min->0) -> DO_CROSS_JUMP jump=408 newjpos=406 newlpos=211. The reverse attempt (e1=223) gets the same MATCH but breaks at jump_insn-207-vs-barrier with min=1 -> no; own-label attempts end last1=0. No other exit condition is reachable for the pair. Judge's belt-and-braces check: CONFIRMED.

- [s6] F1 confirmed IN-TRACE (not just from dumps): only ONE set(-1) (case-8 insn 291) ever appears in any find_cross_jump compare; case-10's sched2-hoisted set-1 never does — its block's compares lead with the sb (mem<-0, insn 371, same uid as s2's committed dump) -> PAT-MISMATCH -> 0 counted matches -> the -1 pair is protected both directions in every pass.

- [s6] r13-chassis REFINEMENT of the s5 micro-fact: the label bonus is NOT jtbl-specific — ANY heading CODE_LABEL (plain branch-target labels 236 and 757) grants it. In r13 BOTH non-arm 13-blocks merge away in cascade (jump=242: 240->761 via bonus-236; then jump=763: 761->211 via bonus-757), leaving ONE RTL set13 (final li-13 word count is reorg delay-slot copying). Label-bonus dodging is structurally impossible: every honest chassis gives 13-blocks heading labels.

- [s6] Committed-form control traced: e1=408 vs e2=223 = PAT-MISMATCH set(reg<-12) vs set(reg<-13) at iteration 1 -> last1=0 -> the label bonus is NEVER REACHED (jump.c:2469 mismatch-break precedes the stream-1 label walk). Both directions, all passes; zero 12/13-site DO_CROSS_JUMP events. The regfix rule is precisely a first-iteration pattern-mismatch manufacturer; the only byte-free honest analog remains the Judge-banned USE/CLOBBER family. Closure theorem: seventh (first white-box) independent confirmation.

- [s6] Baseline re-confirmed: sandbox motion_SetMotion --disable all = honest distance 1 (402/402, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU); src untouched, git clean (metrics/events.jsonl append only). Trace-tool note: the XJDBG set(reg<-N) tag prints the const-int SET_SRC of ANY single_set incl. MEM dests (committed uid 371 sb prints as set(reg<-0)).

- [s6] The 13-pair merge has exactly ONE reachable win path: chain-partner attempt from the jtbl-block side, one counted set13 match + stream-1 CODE_LABEL bonus (jump.c label-bonus at the block's heading label); own-label and arm-side attempts provably fail in-trace.

- [s6] The label bonus is not jtbl-specific: in the r13 chassis both non-arm 13-blocks merge via plain branch-target heading labels (cascade 240->761->211), so label-bonus dodging is impossible in any honest chassis.

- [s6] The committed regfix rule's protection mechanism is a first-iteration pattern mismatch (set12 vs set13) that fires BEFORE the label walk — the only byte-free honest analog of that mismatch class remains the Judge-banned USE/CLOBBER manufacture family.

- [s6] F1 (sched2-slack protection of the -1 pair) confirmed live: case-10's set-1 never appears in any find_cross_jump compare; its block leads with the sb -> 0 counted matches both directions.

- [s6] Diagnostic-infrastructure: BB2_XJUMP_DEBUG knob now in tools/gcc-2.7.2/jump.c and compiled into the in-tree cc1 + tmp/gccdbg/cc1 (Jul 18 build); frozen build/cc1 (May 18) untouched; per-TU faithfulness check banked in trace.sh.

- [s6] Baseline re-confirmed: sandbox motion_SetMotion --disable all = honest distance 1 (402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU); src untouched, git clean.

- [s7] s7 union-TU byte-identity: the n1 union-CLOBBER form compiles byte-identical to the committed+rule TU except the single honest li 13 word (vs_committed.diff: .file line + li 0xc->0xd) — the CLOBBER emits zero bytes and reorg fills the delay slot identically.

- [s7] s7 in-trace mechanism (ninth closure-theorem confirmation, first of the candidate family itself): the CLOBBER protects the 13-pair by stream desynchronization — both find_cross_jump directions break at the GET_CODE insn-code check (INSN vs JUMP_INSN) at iteration 2 with min=1 before the label walk; the label bonus is unreachable; zero DO_CROSS_JUMP events touch the 13-sites while the Wall-1 sel2-tail merge still fires.

- [s7] Mechanism refinement: the union-CLOBBER break is NOT a SET-vs-CLOBBER pattern compare at jump.c:2469 — the CLOBBER is never pattern-compared; the extra byte-free insn desyncs the streams so the opposing stream is already at a jump_insn. Same exit family, different check.

- [s7] Community survey (sotn-decomp master@2472557, rood-reverse main@0ff937b, esa master@914e9db, shallow clones in s7/repos): zero cast-to-union expressions, zero single-member unions, zero 'clobber' strings anywhere; all unions are genuine multi-member data unions. The union-CLOBBER family has NO community precedent.

- [s7] OWNER-ESCALATION filed: docs/grind/decisions.md '2026-07-18 — motion_SetMotion — OWNER-ESCALATION' (hirahira_w_frie format, both options, community-evidence absence stated plainly, s6/s7 trace artifacts referenced). Filed per the 2026-07-17 17:34 Judge disposition; do not self-resolve.

- [s7] Baseline re-confirmed: sandbox motion_SetMotion --disable all = honest distance 1 (402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU); src untouched, git clean except decisions.md (the escalation) + metrics append.

- [s8] docs/grind/decisions.md final entry = '2026-07-18 — motion_SetMotion — OWNER-ESCALATION (filed by grind s7 per the 2026-07-17 17:34 Judge disposition; awaiting owner ruling — do not self-resolve)'; no ruling entry follows it (verified 2026-07-18 this session).

- [s8] The 17:34 Judge ruling in the same file certifies the exhaustion precondition first-hand: F1 discharged, r1-r9 banked KILLED, closure theorem sealed both sides, 'No honest lever remains unmeasured.'

- [s8] Rederive is covered by the dead structural axis: the closure theorem quantifies over ALL byte-matching compiles, not the current source spelling, so a fresh m2c/sibling chassis cannot evade the merge without paying bytes (s5's unmerged-seed campaign plateaued at 82 raw word diffs).

- [s8] memory/grind/motion_SetMotion/candidate.c exists (byte-correct floor-10 form); no src/ edits made this session; regfix.txt:2190 and the committed form untouched per the 17:34 disposition.

- [s9] docs/grind/decisions.md line 761: '2026-07-18 - motion_SetMotion - OWNER-ESCALATION (filed by grind s7 ... awaiting owner ruling - do not self-resolve)' is the FINAL entry in the file as of 2026-07-18 s9; no owner ruling follows it (file tail inspected this session).

- [s9] The escalation presents exactly two mutually exclusive owner options: (a) sanction the union-constructor CLOBBER family (banked n1 form rejected/judge-fail-0717-1708.c re-proposed through the normal candidate path), or (b) refuse the family and sign an owner disposition for regfix.txt:2190 per no-park-permanently.

- [s9] Judge ruling 2026-07-17 17:34 certifies honest-lever exhaustion; the closure theorem carries nine independent confirmations (s2 structural r1-r4, s3 r5-r9, s3b r10-r13, s4 two permuter campaigns 112k+2k iters, s5 unmerged-seed campaign 54k iters, s6 white-box gccdbg trace of honest/committed/r13, s7 white-box trace of the n1 union form + zero community evidence in sotn-decomp/rood-reverse/esa masters).

- [s9] Ledger bank intact this session: memory/grind/motion_SetMotion/candidate.c (floor-10 form, 44,155 bytes) plus 9 rejected forms including the byte-proven n1 union form (judge-fail-0717-1708.c, s7 vs_committed.diff = single li word).

- [s9] src/code6cac_c_mid.c, regfix.txt:2190, and the committed byte-correct form left untouched on main per the Judge constraint; no engine mutation commands run.

- [s10] SYNTHESIS (2026-07-18, full write-up tmp/grind/motion_SetMotion/s10/synthesis.md): the 10-session grind reduces to a three-basin partition covering ALL C shapes — (1) merged basin (every honest-0xD spelling canonicalizes to the 361-insn merged shape; wrong bytes), (2) unmerged-with-cost basin (merge broken by real insns/labels/layout, >=2 insns or 76+ word diffs; wrong bytes), (3) byte-free-breaker basin (USE/CLOBBER manufacture; byte-proven via the n1 form but Judge-unsanctioned). No fourth basin exists; the closure theorem quantifies over all byte-matching compiles, so rederive/permuter/synthesis outputs cannot leave the partition. Nine independent confirmations banked (s2 r1-r4, s3 r5-r9, s3b r10-r13, F1 slack-resolution, s4 dual campaigns 114k iters, s5 unmerged campaign 54k iters, s6 white-box trace x3 TUs, s7 n1 white-box trace, s7 zero-community-evidence survey).

- [s10] Frontier RESET to the two owner acts in the filed 2026-07-18 OWNER-ESCALATION (docs/grind/decisions.md final entry, verified unruled this session): (a) sanction the union-constructor CLOBBER family -> re-propose rejected/judge-fail-0717-1708.c through the normal candidate path (driver re-proves bytes, default-FAIL Judge rules); (b) refuse the family -> owner signs a disposition for regfix.txt:2190 per no-park-permanently. No grind modality can advance either option; every sanctioned axis is measured dead.

- [s10] Baseline re-confirmed: sandbox motion_SetMotion --disable all = honest distance 1 (402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU); ledger bank verified intact (candidate.c 44,155 bytes + 9 rejected forms); src untouched, no engine mutation commands run.

- [s10] Three-basin partition (s10 synthesis): (1) merged basin - every honest-0xD spelling canonicalizes to the 361-insn merged shape; (2) unmerged-with-cost basin - merge broken only by real insns/labels/layout at >=2 insns or 76+ word diffs; (3) byte-free-breaker basin - USE/CLOBBER manufacture, byte-proven (n1) but Judge-unsanctioned. No fourth basin; the closure theorem quantifies over all byte-matching compiles, so rederive/permuter/synthesis outputs cannot leave the partition.

- [s10] docs/grind/decisions.md verified this session: the '2026-07-18 - motion_SetMotion - OWNER-ESCALATION' entry (filed by s7, both options, community-evidence absence stated plainly) is the FINAL entry with no owner ruling following it.

- [s10] Closure theorem carries nine independent confirmations: s2 r1-r4, s3 r5-r9, s3b r10-r13, F1 sched2-slack resolution (non-transplantable), s4 merged-seed campaigns (112,618 + 2,049 iters, only false-zero/semantic-theft finds), s5 unmerged-seed campaign (54,087 iters, plateau 76-91 word diffs), s6 white-box gccdbg traces (honest/committed/r13), s7 white-box trace of the n1 union form, s7 zero-community-evidence survey (sotn-decomp/rood-reverse/esa masters).

- [s10] Baseline re-confirmed this session: sandbox motion_SetMotion --disable all = honest distance 1 (402/402 insns, 1 rule dropped = regfix.txt:2190 semantic-lie subst 12->13, 34 cheat-asm stripped elsewhere in TU).

- [s10] Ledger bank verified intact: candidate.c (floor-10 byte-correct form) + 9 rejected forms including the byte-proven n1 union form (judge-fail-0717-1708.c); src/code6cac_c_mid.c and regfix.txt:2190 untouched on main; no engine mutation commands run.

- [s11] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion is the FINAL entry in docs/grind/decisions.md (line 761 of 776) and is explicitly marked 'awaiting owner ruling — do not self-resolve'; no owner ruling follows it (verified this session by full heading scan of the file)

- [s11] Structural axis dead on the ledger: r1-r9 (s2/s3) + dup-case/ternary/u32-sel/pre-switch-if (s3b) + r13 unmerged-chassis all KILLED; the 17:34 Judge ruling certifies exhaustion complete with F1 discharged

- [s11] Both frontier items are owner acts (sanction the union-constructor CLOBBER family via option (a), or refuse it and sign a rule disposition for regfix.txt:2190 via option (b)); neither is reachable by any grind modality per the s10 three-basin synthesis

- [s11] Judge constraints bind against committing ANY USE/CLOBBER-manufacture spelling, which is the only remaining byte-free C-reachable mechanism — so no structural respelling can be both honest and byte-matching

- [s12] docs/grind/decisions.md:761 OWNER-ESCALATION for motion_SetMotion (filed by grind s7, 2026-07-18) verified present and UNRULED as of this session (2026-07-18): it is the last entry in the file (EOF at line 777), marked 'awaiting owner ruling - do not self-resolve'

- [s12] The escalation presents both owner options: (a) sanction the union-constructor CLOBBER family (closes via banked n1 form rejected/judge-fail-0717-1708.c through the normal candidate path) or (b) refuse the family and sign an owner disposition for regfix.txt:2190 per no-park-permanently

- [s12] Mandated structural axis is Judge-certified dead: 17:34 ruling certifies exhaustion complete (r1-r9 KILLED, r10-r13 in s3/s3b, F1 slack non-transplantable with zero byte-room at the 13-sites); brief forbids re-measuring dead axes when the filed escalation exists

- [s12] No src/ edits made this session; candidate.c and the 9-form rejected/ bank are unchanged and current; committed byte-correct form + regfix.txt rule stay untouched on main per the 17:34 disposition

- [s13] docs/grind/decisions.md ends at line 776; the 2026-07-18 motion_SetMotion OWNER-ESCALATION block (lines 761-776) is the final entry with no owner ruling following it — verified this session (s13)

- [s13] The escalation presents both owner options: (a) sanction the union-constructor CLOBBER family with per-use dossier (banked n1 form is byte-proven and trace-proven), (b) refuse the family and sign an owner disposition for regfix.txt:2190 per no-park-permanently

- [s13] Mandated permuter modality is dead by the ledger: s4 (112k iters, merged chassis — score-0 was a scorer false match) and s5 (54k iters, unmerged r13 chassis — 82-word-diff plateau; merge re-forms with a different partner) close both basins; the s6 white-box trace certifies the closure theorem that makes any further permuter search converge only on unsanctioned USE/CLOBBER spellings

- [s13] Ledger artifacts intact: memory/grind/motion_SetMotion/{candidate.c,evidence.md,hypotheses.md,state.json} plus 9 rejected forms banked

- [s14] docs/grind/decisions.md ends at line 776; the 2026-07-18 motion_SetMotion OWNER-ESCALATION block (lines 761-776, filed by s7) is the final entry with no owner ruling following it - verified this session (s14)

- [s14] The escalation presents the two mutually exclusive owner options: (a) sanction the union-constructor CLOBBER family with per-use dossier (banked n1 form rejected/judge-fail-0717-1708.c is byte-proven and white-box trace-proven), (b) refuse the family and sign an owner disposition for regfix.txt:2190 per no-park-permanently

- [s14] Mandated permuter axis is dead by the ledger without re-measurement: s4 merged-seed campaigns (112,618-iter random - the score-0 was a scorer false match with 6 real word diffs; 2,049-iter directed - best find 100) plus s5 unmerged-r13-seed campaign (54,087 iters, plateau 82 raw word diffs, merge re-forms with a different partner) close both basins; s6 BB2_XJUMP_DEBUG trace certifies any further permuter search can only converge on the Judge-banned USE/CLOBBER spellings

- [s14] Baseline re-confirmed this session: sandbox motion_SetMotion --disable all = honest distance 1 (402/402 insns, rules_dropped 1 = regfix.txt:2190 semantic-lie subst 12->13, cheat_asm_stripped 34 elsewhere in TU)

- [s14] Ledger bank intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 byte-correct form) + 9 rejected forms; no src/ edits this session; regfix.txt:2190 and the committed form untouched on main per the 17:34 Judge disposition

- [s15] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion is the FINAL entry in docs/grind/decisions.md (lines 761-776); a Select-String sweep for OWNER RULING entries shows none dated after it, so it remains unruled as of 2026-07-18 (session 15).

- [s15] The escalation's exhaustion section (decisions.md:767) is Judge-certified at 17:34 and extended by the s6/s7 white-box traces: structural r1-r13 dead, F1 slack-transplant dead, label-family dead, three permuter campaigns (112,618 + 2,049 + 54,087 iters) dead, both forensics traces banked with artifacts under tmp/grind/motion_SetMotion/s6/ and s7/.

- [s15] Binding Judge constraints forbid committing any USE/CLOBBER-manufacture spelling (the only remaining C-reachable mechanism), so no probe available to this session could change the floor without the owner ruling first.

- [s15] memory/grind/motion_SetMotion/candidate.c remains the banked best form (floor 10, byte-identical with the one rule applied); left untouched per the escalation's standing disposition that main stays as-is pending the ruling.

- [s16] docs/grind/decisions.md ends at line 777; the 2026-07-18 motion_SetMotion OWNER-ESCALATION block (lines 761-776, filed by s7) is the FINAL entry with no owner ruling following it - verified fresh this session (s16)

- [s16] The escalation presents the two mutually exclusive owner options: (a) sanction the union-constructor CLOBBER family with per-use dossier (banked n1 form rejected/judge-fail-0717-1708.c is byte-proven and white-box trace-proven), (b) refuse the family and sign an owner disposition for regfix.txt:2190 per no-park-permanently

- [s16] Mandated forensics axis is dead on the ledger with pass-level artifacts: s6 BB2_XJUMP_DEBUG traces (honest/committed/r13 TUs - merge path, rule mechanism, and F1 slack all named at jump.c/sched2 level) and s7 trace of the n1 union form (CLOBBER stream-desync break, both directions); the closure theorem carries nine independent confirmations and binding Judge constraints forbid the only remaining C-reachable mechanism

- [s16] Ledger bank intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 byte-correct form) + 9 rejected forms; no src/ edits this session; regfix.txt:2190 and the committed form untouched on main per the 17:34 Judge disposition; working tree clean except metrics append
