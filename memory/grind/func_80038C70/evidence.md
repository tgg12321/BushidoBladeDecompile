> **ALIAS NOTE (2026-08-24):** this ledger's function is `func_80038C70` (formerly `motion_SetMotion` — decisions.md rulings and interior prose may use either name; any `memory/grind/motion_SetMotion/` path is this dir).

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

- [s17] 2026-07-18 OWNER-ESCALATION for motion_SetMotion verified on file at docs/grind/decisions.md:761-776 and UNRULED as of 2026-07-18 s17 — it is the final entry; the file ends with its References line and no owner ruling follows.

- [s17] Every sanctioned axis is measured dead on the ledger: structural (s2/s3/s3b r1-r13, five-way closure confirmation), permuter (s4 false-zero/semantic-theft classification + s5 two-sided unmerged-chassis plateau), forensics (s6 end-to-end gccdbg trace certifying the closure theorem; s7 union-form trace + zero community evidence), rederive (s8/s9: dead by construction — the theorem quantifies over all byte-matching C shapes), synthesis (s10 three-basin partition: no cross-axis composition escapes).

- [s17] Binding Judge constraints forbid the only byte-proven closing form (union-constructor CLOBBER family, banked as rejected/judge-fail-0717-1708.c) pending the owner's option (a)/(b) ruling; regfix.txt:2190 and the committed form stay untouched on main meanwhile.

- [s17] candidate.c remains banked at memory/grind/motion_SetMotion/candidate.c (floor 10 chassis); no src edits made this session, per the parked state.

- [s18] docs/grind/decisions.md:761-776 - the 2026-07-18 motion_SetMotion OWNER-ESCALATION is the FINAL entry in the file (776 lines total); no owner ruling follows it as of 2026-07-18 session 18

- [s18] The two binding Judge constraints forbid committing any USE/CLOBBER-manufacture spelling (union constructor, cast-to-union, single-member struct, struct-return, dead union local) - the only byte-free breaker the closure theorem leaves open

- [s18] Ledger intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, Wall-1-solved floor-10 form) plus 9 banked rejected forms; frontier is exactly the two owner acts (sanction the CLOBBER family and re-propose judge-fail-0717-1708.c, or refuse and sign a disposition for regfix.txt:2190)

- [s18] Sessions 8-17 across all five modalities (rederive, synthesis, structural, permuter, forensics) each independently verified the same gate; every remaining sanctioned axis is measured dead on the ledger

- [s19] docs/grind/decisions.md is 776 lines; the 2026-07-18 OWNER-ESCALATION for motion_SetMotion occupies lines 761-776 and is the FINAL entry — no owner ruling follows it as of s19 (2026-07-18)

- [s19] Ledger coherence re-confirmed: 25 hypothesis entries s2-s18; all five ladder axes (structural, permuter, forensics, rederive, synthesis) measured dead with the closure theorem carrying eight independent confirmations including two BB2_XJUMP_DEBUG white-box pass traces (s6 honest/committed/r13, s7 union n1 form)

- [s19] s10 three-basin partition re-verified as the synthesis-axis seal: axis kills are quantified over jump2-time RTL invariants of ANY byte-matching compile, not over source spellings, so cross-axis compositions (F1+r13, permuter-find+structural-respelling, split-init+reshaped-arm) all canonicalize inside the enumerated basins

- [s19] Banked artifacts verified on disk: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 form) and 9 rejected/ forms including the byte-proven, trace-proven n1 union form (judge-fail-0717-1708.c) referenced by escalation option (a)

- [s20] OWNER-ESCALATION for motion_SetMotion verified on file at docs/grind/decisions.md:761-776 (filed 2026-07-18 by grind s7 per the 17:34 Judge disposition), and it is the FINAL entry in the 776-line file - no owner ruling follows; the escalation is unruled as of s20

- [s20] Mandated structural axis measured dead on the ledger: r1-r13 structural sweeps banked (rejected/s3-structural-sweep-*.c, s3b-*.c, structural-13pair-axis-closed.c), closure theorem has eight independent confirmations per the escalation text, and the 17:34 Judge ruling certifies exhaustion complete

- [s20] Ledger intact this session: memory/grind/motion_SetMotion/candidate.c (44155 bytes, floor-10 form) and 9 banked rejected forms all present; no src/ edits made, committed form and regfix.txt:2190 untouched per the 17:34 disposition item (3)

- [s21] OWNER-ESCALATION for motion_SetMotion verified on file at docs/grind/decisions.md:761-776 (filed 2026-07-18 by grind s7 per the 17:34 Judge disposition), and verified UNRULED: it is the final entry in the file (ends line 777), with no owner ruling after it

- [s21] The mandated structural modality is measured dead on the ledger: r1-r9 (s2/s3) and r10-r13 (s3b) all KILLED; rejected bank holds structural-13pair-axis-closed.c, s3-structural-sweep-goto-reorder-thunk-splitinit-hoist-dead.c, s3b-dupcase-ternary-u32-preswitch-if-dead.c

- [s21] Judge constraints bind: no USE/CLOBBER-manufacture spelling of any kind may be committed; regfix.txt:2190 and the committed form stay untouched on main pending the owner's ruling

- [s21] Ledger candidate memory/grind/motion_SetMotion/candidate.c preserved as the best form (floor 10, honest distance 1 = the single li 13 word); no src edits made this session per the do-not-self-resolve disposition

- [s21] Sessions s8-s20 across all five modalities (rederive, synthesis, structural, permuter, forensics) each independently verified the same owner-gated state; s10 three-basin partition seals cross-axis composition

- [s22] 2026-07-18 OWNER-ESCALATION for motion_SetMotion is the final entry in docs/grind/decisions.md (lines 761-776); no ruling text follows it as of session 22 (2026-07-18)

- [s22] Permuter axis dead two-sided per ledger: three campaigns totaling 168,754 iterations produced only false-zero (s4-permuter-false-zero-swapped-jump-targets.c) and semantic-theft (s4-permuter-semantic-theft-store-into-case911.c) finds, all below the merge penalty

- [s22] Judge constraints ban committing any USE/CLOBBER-manufacture spelling (union constructor, cast-to-union, single-member struct, struct-return, dead union local) pending the owner ruling, which is the only C-reachable mechanism left per the closure theorem

- [s22] Both frontier items are owner acts (sanction option (a) via re-proposal of the banked n1 form, or refuse via option (b) rule disposition); neither is reachable by a grind session

- [s23] The 2026-07-18 OWNER-ESCALATION for motion_SetMotion is the FINAL entry in docs/grind/decisions.md (lines 761-776 of 776) and no owner ruling follows it - verified this session by reading the file tail.

- [s23] The escalation presents both options: (a) sanction the union-constructor CLOBBER family (closes via banked n1 form rejected/judge-fail-0717-1708.c through the normal candidate path), (b) refuse the family (owner signs a disposition for regfix.txt:2190 per no-park-permanently). Community survey evidence: NONE FOUND, stated plainly in the entry.

- [s23] Permuter axis dead two-sided on the ledger: s5 fresh-seed plateau (rejected/s5-permuter-unmerged-r13-seed-plateau.c), s4 false-zero (swapped jump targets) and semantic-theft rejections, re-verified by permuter-modality sessions s13, s14, s22.

- [s23] Closure theorem (Judge-certified, F1 discharged): any byte-matching compile forces both 13-sites to [set13; j] with the jtbl label adjacent, guaranteeing jump2's counted-match + label-bonus merge; the only C-reachable iter1-breaker is the unsanctioned USE/CLOBBER class per the binding Judge constraints.

- [s23] memory/grind/motion_SetMotion/candidate.c preserved unchanged (byte-proven floor-10 form); no src/ edits made this session.

- [s24] docs/grind/decisions.md:761-776 — 2026-07-18 OWNER-ESCALATION for motion_SetMotion is the final entry in the file (776 lines total) and carries no owner ruling; re-verified this session (s24).

- [s24] The escalation's exhaustion section certifies the closure theorem with eight independent confirmations including the s6/s7 white-box forensics traces — the mandated modality's deliverable (exact pass + decision) already exists in the ledger.

- [s24] Ledger frontier is exactly the two owner acts: (a) sanction the union-constructor CLOBBER family (banked byte-proven n1 form re-proposed through the normal candidate path) or (b) refuse and sign a disposition for regfix.txt:2190 per no-park-permanently.

- [s24] memory/grind/motion_SetMotion/candidate.c present (floor-10 form, Wall-1 duplicated-store priority lift landed); rejected/ bank intact at 9 forms; no src/ edits made this session.

- [s25] docs/grind/decisions.md:761-776 is the final entry in the file (verified this session): the motion_SetMotion OWNER-ESCALATION filed 2026-07-18 by grind s7, awaiting owner ruling, do-not-self-resolve

- [s25] No OWNER RULING entry follows the escalation; the last five entry headers end at :761

- [s25] Closure theorem (Judge-certified 17:34, eight confirmations): any byte-matching compile forces both 13-sites to [set13; j] with a heading CODE_LABEL adjacent, guaranteeing the find_cross_jump merge; the only byte-free C-reachable breaker is the USE/CLOBBER class, unsanctioned and constraint-blocked in every respelling

- [s25] Forensics artifacts already banked: s6 BB2_XJUMP_DEBUG traces (honest merge fires exactly once via counted set13 match + label bonus; committed rule's set12-vs-set13 mismatch breaks iteration 1) and s7 union-form end-to-end trace (CLOBBER insn 407 between CODE_LABEL 405 and set13 409; both-direction GET_CODE break with min=1; zero DO_CROSS_JUMP events on any 13-site uid)

- [s25] Ledger candidate memory/grind/motion_SetMotion/candidate.c and the 13-form rejected/ bank verified present; floor unchanged at 10 (honest sandbox distance 1 insn = the single li 13 word covered by regfix.txt:2190)

- [s25] Sessions s11-s24 across all five modalities (structural, permuter, forensics, rederive, synthesis) each independently verified the same gate; s25 forensics matches s15/s16/s24 findings exactly

- [s26] [s26] docs/grind/decisions.md: the 2026-07-18 motion_SetMotion OWNER-ESCALATION (lines 761-776, filed by grind s7) is the FINAL entry; file tail read this session ends with the escalation's References line - no owner ruling follows

- [s26] [s26] Mandated rederive axis dead without re-measurement: the closure theorem quantifies over all byte-matching compiles (s10 three-basin partition), so fresh m2c/decomp.me-corpus/Kengo-transplant chassis cannot produce a shape outside the merged basin (wrong bytes), the unmerged-with-cost basin (>=2 insns / 76+ word diffs), or the Judge-banned USE/CLOBBER byte-free-breaker basin

- [s26] [s26] Ledger bank verified intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 byte-correct form) plus 9 rejected forms including the byte-proven n1 union form judge-fail-0717-1708.c (44,478 bytes)

- [s26] [s26] No src/ edits made this session; regfix.txt:2190 and the committed byte-correct form remain untouched on main per the 2026-07-17 17:34 Judge disposition item (3); working tree clean except metrics append

- [s26] [s26] Sessions s11-s25 across all five ladder modalities independently verified this same gate; s26 (rederive) matches s17/s18 findings exactly

- [s27] docs/grind/decisions.md:761-776 OWNER-ESCALATION (filed by s7) re-verified this session as the FINAL entry; no owner ruling follows (file body read through EOF at 776)

- [s27] Fresh sandbox motion_SetMotion --disable all (s27): score 1, target_insns 402, build_insns 402, rules_dropped 1, cheat_asm_stripped 34 - identical to the banked s14 measurement; the honest floor is unchanged

- [s27] Bank intact: memory/grind/motion_SetMotion/candidate.c = 44,155 bytes; all 9 rejected/ forms present including the byte-proven trace-proven n1 union form judge-fail-0717-1708.c (44,478 bytes) required by escalation option (a)

- [s27] Closure theorem standing at nine independent confirmations (r1-r13 structural sweeps, F1 slack-transplant negative, m7/r13 label-family negatives, 168,754 permuter iterations two-sided, s6 honest/committed/r13 gccdbg traces, s7 union-form end-to-end trace, zero community evidence in sotn/rood-reverse/esa masters); 17:34 Judge ruling certifies no honest lever remains unmeasured

- [s27] src/code6cac_c_mid.c and regfix.txt:2190 untouched on main per the 17:34 disposition; hypotheses.md updated with two s27 entries

- [s28] docs/grind/decisions.md read 755-EOF this session: the 2026-07-18 OWNER-ESCALATION block (lines 761-776) is the final entry, still marked 'awaiting owner ruling — do not self-resolve'; no OWNER RULING follows it

- [s28] Fresh sandbox measurement s28: motion_SetMotion --disable all = score 1, target/build 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU — byte-for-byte identical to the banked s14/s27 measurements

- [s28] Bank intact: candidate.c 44,155 bytes; 9 rejected/ forms present including judge-fail-0717-1708.c (44,478 bytes, the byte-proven trace-proven n1 union-CLOBBER form referenced by escalation option (a))

- [s28] Synthesis axis sealed: s10 partition argument (re-verified s19, s28) — all axis kills are jump2-time RTL-invariant statements, so every source-level composition must still present both 13-sites as [set13; j] with adjacent heading CODE_LABEL, guaranteeing the merge; only byte-free breaker is the Judge-banned USE/CLOBBER family

- [s28] s28 verification entry appended to memory/grind/motion_SetMotion/hypotheses.md

- [s29] docs/grind/decisions.md ends with the 2026-07-18 OWNER-ESCALATION References line (verified via tail read this session); no OWNER RULING entry follows — the escalation filed by grind s7 remains unruled.

- [s29] Fresh gradient this session: sandbox motion_SetMotion --disable all = score 1, target 402 / build 402 insns, 1 rule dropped (regfix.txt:2190), 34 cheat-asm stripped elsewhere in TU — byte-identical to the banked s14/s27/s28 measurements.

- [s29] Structural axis dead by Judge-certified exhaustion: r1-r13 sweeps (s2/s3/s3b) all KILLED, F1 slack-transplant non-transplantable, m7/r13 label-family negatives banked, s6/s7 BB2_XJUMP_DEBUG traces seal the closure theorem two-sided; 17:34 Judge ruling certifies 'no honest lever remains unmeasured'.

- [s29] Ledger bank intact: memory/grind/motion_SetMotion/candidate.c = 44,155 bytes; all 9 rejected/ forms present including judge-fail-0717-1708.c (44,478 bytes, the byte-proven trace-proven n1 union form for escalation option (a)).

- [s29] src/code6cac_c_mid.c and regfix.txt:2190 untouched on main this session per the 17:34 disposition; s29 verification entry appended to hypotheses.md.

- [s30] s30: docs/grind/decisions.md is 776 lines; the 2026-07-18 OWNER-ESCALATION for motion_SetMotion at lines 761-776 is the final entry (ends with its References line), still unruled — no OWNER RULING follows it.

- [s30] s30: sandbox motion_SetMotion --disable all = score 1, target 402 / build 402 insns, 1 rule dropped (regfix.txt:2190), 34 cheat-asm stripped elsewhere in TU — byte-identical to the banked s14/s27/s28/s29 measurements.

- [s30] s30: bank intact — candidate.c 44,155 B; 9 rejected forms present incl. judge-fail-0717-1708.c 44,478 B (the byte-proven, trace-proven n1 union form referenced by escalation option (a)).

- [s31] docs/grind/decisions.md read 740-EOF in s31: file ends at the escalation's References line (line 776); the 2026-07-18 OWNER-ESCALATION for motion_SetMotion at lines 761-776 is the final entry with no owner ruling after it

- [s31] Fresh gradient s31: sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU - byte-identical to the banked s14/s27/s28/s29/s30 measurements

- [s31] Bank verified intact s31: candidate.c = 44,155 bytes; all 9 rejected/ forms present including the byte-proven n1 form judge-fail-0717-1708.c = 44,478 bytes

- [s31] Permuter axis closure re-verified from the ledger: s4 perm_a 112,618 + perm_b 2,049 iterations (merged seeds; zero honest finds; scorer floor occupied by false matches) and s5 perm_c 54,087 iterations (unmerged r13 seed; plateau 76-91 word diffs; merge re-forms with a different partner) - both sides of the merge boundary measured

- [s31] src/code6cac_c_mid.c and regfix.txt:2190 untouched on main per the 17:34 Judge disposition; no campaign launched, no edits made this session

- [s32] docs/grind/decisions.md:761-776 — OWNER-ESCALATION for motion_SetMotion filed 2026-07-18 per the Judge's 2026-07-17 17:34 disposition; it is the last entry in the file as of this session (verified by tail read), therefore unruled; the entry itself forbids self-resolution.

- [s32] Permuter axis measured dead two-sided (hypotheses.md [s4]/[s5], re-read this session): merged-seed campaigns perm_a (112,618 iter) and perm_b (2,049 iter directed) yielded only false-zero (swapped jump targets) and semantic-theft finds; unmerged-seed campaign perm_c (54,087 iter, r13 chassis) descended by repairing branch offsets, never restoring the third li13 word — the chassis is structurally one li13 short because the merge picks a different partner when the jtbl label is removed.

- [s32] s6 gccdbg trace (hypotheses.md [s6], CONFIRMED): the 13-pair merge fires via 1 counted set13 match + label bonus; label-bonus dodging is impossible in any honest chassis (r13's blocks cascade-merge via their own heading labels); the only byte-free breaker is the unsanctioned USE/CLOBBER manufacture class — so no permuter basin can contain an honest byte-neutral unmerged form.

- [s32] Floor unchanged at 10 (honest sandbox distance 1 = the single li word governed by regfix.txt:2190); no src/ edits were made this session — the committed byte-correct form and the rule stay untouched on main per the escalation's standing disposition.

- [s33] docs/grind/decisions.md re-verified this session (s33, forensics): 776 lines total; the 2026-07-18 motion_SetMotion OWNER-ESCALATION (lines 761-776, filed by s7) is the FINAL heading with no owner ruling following it.

- [s33] Mandated forensics axis remains dead with pass-level artifacts already banked: s6 BB2_XJUMP_DEBUG traces (honest/committed/r13 TUs) and s7 n1 union-form trace name the exact pass (jump2 find_cross_jump, counted set13 match + stream-1 CODE_LABEL bonus; committed rule = iteration-1 set12-vs-set13 pattern mismatch; union CLOBBER = stream-desync GET_CODE break) - the modality's deliverable already exists in the ledger.

- [s33] Ledger bank verified intact: candidate.c 44,155 bytes + 9 rejected forms; no src/ edits made; regfix.txt:2190 and the committed byte-correct form untouched on main per the 17:34 Judge disposition.

- [s33] docs/grind/decisions.md re-verified s33: 776 lines; the 2026-07-18 motion_SetMotion OWNER-ESCALATION (lines 761-776, filed by grind s7 per the 17:34 Judge disposition) is the FINAL heading; no owner ruling follows

- [s33] Forensics axis dead with artifacts: s6 traces certify the closure theorem white-box (merge fires via exactly one path: counted set13 match + stream-1 label bonus; own-label and arm-side attempts provably fail) and s7 traces the unsanctioned n1 union form (byte-identical except the single li 13 word; CLOBBER desyncs streams so both directions break at the GET_CODE insn-code check with min=1)

- [s33] All five ladder axes measured dead on the ledger: structural (r1-r13), permuter (168,754 iterations two-sided), forensics (s6/s7), rederive (dead by construction under the closure theorem), synthesis (s10 three-basin partition)

- [s33] Binding Judge constraints forbid committing any USE/CLOBBER-manufacture spelling - the only C-reachable byte-free iter1-breaker - pending the owner's option (a)/(b) ruling

- [s33] Ledger bank intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 byte-correct Wall-1-solved form) + 9 rejected forms including the byte-proven, trace-proven n1 form (judge-fail-0717-1708.c); no src/ edits this session; regfix.txt:2190 and the committed form untouched on main

- [s34] [s34] docs/grind/decisions.md ends at line 777; the 2026-07-18 motion_SetMotion OWNER-ESCALATION (lines 761-776, filed by grind s7) is the FINAL entry with no owner ruling following it — re-verified fresh this session by reading the file tail (Read through 799 returned EOF at 777)

- [s34] [s34] Mandated forensics axis is dead on the ledger with pass-level artifacts: s6 BB2_XJUMP_DEBUG traces (honest form merges via one counted set13 match + jtbl CODE_LABEL stream-1 bonus; committed rule protects via iteration-1 set12-vs-set13 pattern mismatch before the label walk; F1 sched2-slack protection of the -1 pair confirmed in-trace) and s7 trace of the n1 union form (byte-free CLOBBER desyncs both find_cross_jump streams — GET_CODE INSN-vs-JUMP_INSN break at iteration 2, label bonus unreachable)

- [s34] [s34] Binding Judge constraints forbid committing any USE/CLOBBER-manufacture spelling — the only C-reachable byte-free iter1-breaker per the closure theorem — pending the owner's option (a)/(b) ruling

- [s34] [s34] Ledger bank intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 byte-correct Wall-1-solved form) plus 9 rejected forms including the byte-proven, trace-proven n1 union form (judge-fail-0717-1708.c); no src/ edits made; regfix.txt:2190 and the committed form untouched on main per the 17:34 Judge disposition

- [s35] sandbox motion_SetMotion --disable all (s35) = score 1, target 402 / build 402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU - identical to every banked measurement since s14

- [s35] docs/grind/decisions.md read through EOF (777): OWNER-ESCALATION block at 761-776 is the final entry, unruled, filed by grind s7 in hirahira_w_frie format with both owner options and community evidence stated plainly as NONE FOUND

- [s35] Ledger bank intact: candidate.c 44,155 bytes; 9 rejected forms on disk including judge-fail-0717-1708.c (44,478 bytes, the byte-proven trace-proven n1 union form referenced by escalation option (a))

- [s35] Closure theorem standing: nine independent confirmations (r1-r13 structural sweeps, F1 slack-transplant negative, m7/r13 label-family negatives, 168,754 permuter iterations two-sided, s6/s7 white-box BB2_XJUMP_DEBUG traces, zero community evidence in sotn-decomp/rood-reverse/esa masters)

- [s36] s36: OWNER-ESCALATION block at docs/grind/decisions.md:761-776 re-verified as the final entry in the file (read 740-EOF); no owner ruling follows; both options (a) sanction union-CLOBBER family via n1 re-proposal, (b) refuse family + owner rule-disposition for regfix.txt:2190) intact with community evidence stated NONE FOUND

- [s36] s36: fresh gradient sandbox motion_SetMotion --disable all = score 1, 402/402 insns, 1 rule dropped, 34 cheat-asm stripped elsewhere in TU - unchanged from every banked measurement since s14

- [s36] s36: bank intact - candidate.c 44,155 bytes, 9 rejected/ forms present including judge-fail-0717-1708.c (44,478 bytes, the byte-proven trace-proven n1 union form escalation option (a) depends on); src/code6cac_c_mid.c and regfix.txt:2190 untouched on main per the 17:34 disposition

- [s36] Closure theorem remains sealed with nine independent confirmations (r1-r13 structural sweeps, F1 slack-transplant negative, m7/r13 label-family negatives, 168k+ permuter iterations two-sided, s6 BB2_XJUMP_DEBUG pass trace, s7 union-form end-to-end trace, zero community evidence in sotn/rood-reverse/esa masters)

- [s37] [s37] docs/grind/decisions.md: the 2026-07-18 motion_SetMotion OWNER-ESCALATION (lines 761-776, filed by grind s7) is the FINAL entry in the file with no owner ruling following it - re-verified this session by reading the file tail

- [s37] [s37] Mandated synthesis axis is sealed by the s10 three-basin partition: (1) merged basin - every honest-0xD spelling canonicalizes to the 361-insn merged shape; (2) unmerged-with-cost basin - merge broken only by real insns/labels/layout at >=2 insns or 76+ word diffs; (3) byte-free-breaker basin - USE/CLOBBER manufacture, byte-proven (n1 form) but forbidden by binding Judge constraints; no fourth basin exists and cross-axis compositions cannot leave the partition

- [s37] [s37] Ledger bank verified intact on disk: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 byte-correct Wall-1-solved form) plus 9 rejected forms including the byte-proven, trace-proven n1 union form (judge-fail-0717-1708.c) referenced by escalation option (a)

- [s37] [s37] No src/ edits made this session; src/code6cac_c_mid.c, regfix.txt:2190, and the committed byte-correct form remain untouched on main per the 2026-07-17 17:34 Judge disposition item (3); working tree clean except metrics append

- [s38] docs/grind/decisions.md tail verified this session (structural modality): the 2026-07-18 motion_SetMotion OWNER-ESCALATION (filed by grind s7) is the FINAL entry - the file ends with its References line and no owner ruling follows; escalation remains unruled as of 2026-07-18 s38.

- [s38] Mandated structural axis dead on the ledger without re-measurement: r1-r13 all KILLED (s2/s3/s3b), 17:34 Judge ruling certifies exhaustion complete, s10 three-basin partition quantifies over all byte-matching compiles; binding Judge constraints forbid the only byte-free breaker (USE/CLOBBER manufacture).

- [s38] Ledger bank intact: candidate.c 44,155 bytes (floor-10 form) + 9 rejected forms; src/ and regfix.txt clean, no edits made; committed form and regfix.txt:2190 untouched on main per the 17:34 disposition.

- [s38] docs/grind/decisions.md tail verified this session (s38): the file ends with the 2026-07-18 motion_SetMotion OWNER-ESCALATION References line; no owner ruling follows - the escalation is unruled.

- [s38] Ledger bank verified intact this session: memory/grind/motion_SetMotion/candidate.c present at 44,155 bytes (floor-10 byte-correct Wall-1-solved form) and rejected/ holds 9 banked forms including the byte-proven, trace-proven n1 union form (judge-fail-0717-1708.c).

- [s38] git status clean on src/ and regfix.txt: no edits made this session; the committed byte-correct form and regfix.txt:2190 stay untouched on main per the 17:34 Judge disposition item (3).

- [s38] Structural axis dead per ledger without re-measurement: r1-r13 KILLED (s2/s3/s3b), F1 sched2-slack non-transplantable (zero byte-room at the 13-sites), label-bonus undodgeable in any honest chassis (s6 r13 trace); the only remaining C-reachable byte-free breaker is the Judge-banned USE/CLOBBER manufacture family.

- [s39] OWNER-ESCALATION for motion_SetMotion verified on file at docs/grind/decisions.md:761-776 (dated 2026-07-18, filed by grind s7 per the 2026-07-17 17:34 Judge disposition, marked 'awaiting owner ruling — do not self-resolve'); it is the FINAL entry in the file — no owner ruling follows it as of s39 (2026-07-18).

- [s39] Structural axis dead on the ledger: hypotheses.md banks r1-r13 all KILLED across s2/s3/s3b (re-association, branch-sense swap, s16 and unsigned narrowing, const split, goto-share, case reorder, jump-thunk, split-init, hoist, duplicate-case, cond-expr, pre-switch if-extraction) — every spelling canonicalizes to the merged shape before jump2 or pays real bytes.

- [s39] Judge constraints bind: no USE/CLOBBER-manufacture spelling of any kind may be committed; regfix.txt:2190 and the committed byte-correct form stay untouched on main pending the owner ruling; both frontier items are owner acts (sanction family (a) vs refuse family (b)), not grindable work.

- [s39] Floor unchanged at 10 (honest pure-C distance 1 = the single li 13 word protected by the last rule); candidate.c retained in memory/grind/motion_SetMotion/ as the best banked form.

- [s40] docs/grind/decisions.md:761-776 (2026-07-18 OWNER-ESCALATION, filed by s7 per the 17:34 Judge disposition) is the FINAL entry in the 777-line file with no owner ruling following it — re-verified fresh this session (s40)

- [s40] Permuter axis dead two-sided: s4 merged-seed campaigns (112,618-iter random, lone score-0 a scorer false match with 6 real word diffs; 2,049-iter directed, best find 100) + s5 unmerged-r13-seed campaign (54,087 iters, plateau 82 raw word diffs, merge re-forms with a different partner)

- [s40] Closure theorem carries nine independent confirmations (structural r1-r13, F1 sched2-slack non-transplantable, three permuter campaigns, s6 white-box traces of honest/committed/r13 TUs, s7 white-box trace of the n1 union form, s7 zero-community-evidence survey of sotn-decomp/rood-reverse/esa masters)

- [s40] Binding Judge constraints forbid committing any USE/CLOBBER-manufacture spelling — the only C-reachable byte-free mechanism left — pending the owner's option (a)/(b) ruling

- [s40] Ledger bank verified intact this session: memory/grind/motion_SetMotion/candidate.c (floor-10 byte-correct form) + 9 rejected forms including the byte-proven, trace-proven n1 union form (judge-fail-0717-1708.c); src/code6cac_c_mid.c and regfix.txt:2190 untouched on main per the 17:34 disposition; no campaigns launched, no engine mutation commands run

- [s41] docs/grind/decisions.md:761-776 contains the 2026-07-18 OWNER-ESCALATION for motion_SetMotion (filed s7 per the 17:34 Judge disposition); it is the final entry in the file with no owner ruling after it — verified by direct read this session

- [s41] Permuter axis dead per ledger: campaigns of 112,618 + 2,049 + 54,087 iterations from both sides of the merge boundary produced only false-zero and semantic-theft finds below the merge penalty (banked in rejected/)

- [s41] Closure theorem sealed with eight independent confirmations (structural r1-r13, F1 slack-transplant negative, label-family negatives m7/r13, three permuter campaigns, s6 pass-level BB2_XJUMP_DEBUG trace): any byte-matching compile forces both 13-sites to [set13; j] adjacent to the jtbl label, merge guaranteed, only byte-free breaker is the Judge-constrained USE/CLOBBER class

- [s41] candidate.c banked at memory/grind/motion_SetMotion/candidate.c (floor-10 form, Wall-1 solved); 9 rejected forms banked including judge-fail-0717-1708.c (the byte-proven n1 union form awaiting option (a))

- [s42] OWNER-ESCALATION for motion_SetMotion filed 2026-07-18 at docs/grind/decisions.md:761-776 (hirahira_w_frie format, both options, community evidence honestly NONE) remains the final entry for the function as of s42 - no owner ruling appended; Select-String over decisions.md confirms no later motion_SetMotion or OWNER RULING entry.

- [s42] Forensics closure is internal to the escalation: s6 BB2_XJUMP_DEBUG trace (merge fires exactly once via counted set13 match + label bonus; committed rule's set12-vs-set13 mismatch breaks iteration 1) and s7 union-form end-to-end trace (clobber insn 407 between CODE_LABEL 405 and set13 409; both-direction iteration-2 INSN-vs-JUMP_INSN GET_CODE break, min=1, label walk never reached; reorg fills [j; delay li 13] exactly).

- [s42] Exhaustion is Judge-certified (2026-07-17 17:34) with eight independent confirmations: r1-r13 structural sweeps, F1 slack-transplant negative, m7/r13 label-family negatives, three permuter campaigns (112,618 + 2,049 + 54,087 iterations, only false-zero/semantic-theft below the merge penalty), and the s6/s7 traces.

- [s42] Sessions s27-s41 (all five modalities) each independently re-verified this same owner-gated state; s33/s34 were the forensics-modality verifications.

- [s43] docs/grind/decisions.md is 776 lines; the 2026-07-18 motion_SetMotion OWNER-ESCALATION (filed by grind s7 per the 17:34 Judge disposition) occupies lines 761-776 and is the FINAL entry with no owner ruling after it - verified by reading the file tail this session (s43)

- [s43] The escalation presents the two mutually exclusive owner options: (a) sanction the union-constructor CLOBBER family (closes via banked byte-proven, trace-proven n1 form rejected/judge-fail-0717-1708.c through the normal candidate path), (b) refuse the family and sign an owner disposition for regfix.txt:2190 per no-park-permanently

- [s43] Mandated forensics axis is dead on the ledger with pass-level artifacts: s6 white-box traces certify the closure theorem (merge fires via counted set13 match + label bonus; the rule is a first-iteration pattern-mismatch manufacturer; F1 slack non-transplantable) and s7 traced the candidate family itself (CLOBBER stream-desync, zero DO_CROSS_JUMP events at 13-sites, byte-identical except the single li 13 word)

- [s43] Ledger bank intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, floor-10 byte-correct form) + 9 rejected forms; no src/ edits made this session; regfix.txt:2190 and the committed form untouched on main per the 17:34 Judge disposition item (3)

- [s44] docs/grind/decisions.md OWNER-ESCALATION for motion_SetMotion (lines 761-776, filed by grind s7) verified present and UNRULED this session (s44); the only entry following it is the 2026-07-18 20:17 D_80082050 PASS - unrelated function, not a ruling.

- [s44] Mandated rederive axis is dead by construction on the ledger: the closure theorem quantifies over all byte-matching compiles (s8), sealed by the s10 three-basin partition and nine confirmations (s2 r1-r4, s3 r5-r9, s3b r10-r13, F1 slack non-transplantable, s4 dual campaigns 114k iters, s5 unmerged campaign 54k iters, s6 white-box traces x3 TUs, s7 n1 union trace, s7 zero-community-evidence survey); rederive sessions s35/s36 previously confirmed.

- [s44] Ledger bank intact: memory/grind/motion_SetMotion/candidate.c (44,155 bytes, byte-proven floor-10 Wall-1-solved form) + 9 rejected forms including the byte-proven, trace-proven n1 union form (judge-fail-0717-1708.c) referenced by escalation option (a).

- [s44] No src/ edits made; regfix.txt:2190 and the committed byte-correct form untouched on main per the 2026-07-17 17:34 Judge disposition item (3); working tree clean except the metrics/events.jsonl append.

## [s45] escalation modality — chassis re-measurement + source-level enumeration proof of the cross-jump wall

- [s45] CHASSIS RE-MEASURED under the current toolchain (post cc1 fork-crash fix adopted 2026-08-24,
  decisions.md:10340). `sandbox func_80038C70 --disable all` on the committed HEAD form:
  score=1, target_insns=402, build_insns=402, rules_dropped=1, cheat_asm_stripped=7.
  The digest's "ledger floor 10" is STALE/mis-tracked bookkeeping — the honest floor of the
  committed form is 1 (the single `addiu $s0,$zero,0xC` immediate that regfix.txt:1095
  rewrites to 13), and 402/402 insn parity holds. Every s14/s27..s44 measurement of "1" is
  reconfirmed on the new chassis; the cc1 fix changed nothing here.

- [s45] THE HONEST FORM RE-MEASURED on the new chassis. Edited src/code6cac_c_mid.c
  `case 9: case 11: sel = 0xC;` -> `sel = 0xD;` (the semantic truth the rule fakes) and re-ran the
  sandbox: score=2, target_insns=402, build_insns=400. The honest 0xD form is TWO INSNS SHORT,
  not one: jump2's cross-jump deletes BOTH the `addiu $s0,$zero,0xD` and its `j sel_dispatch`
  from one arm. (Prior sessions recorded "1 word short"; the correct figure on this chassis is
  a 2-insn deletion, and the committed form's score-1 is the immediate only.) Source restored;
  form banked at rejected/s45-honest-0xD-crossjump-merge-400insns.c.

- [s45] THE TWO MERGE-CANDIDATE ARMS, located exactly (jump2 RTL dump, artifact
  tmp/grind/func_80038C70/s45/jump2_fn.txt, extracted from the .jump2 dump produced this
  session by tools/grinder/dump.ps1):
    * arm A (if-chain, `if (v0 != 10) sel = 0xD;`): insn 211 `(set (reg/v:SI 16 s0) (const_int 13))`
      + jump_insn 223 `(set (pc) (label_ref 444))` {jump}  -> target bytes 0x80038DA8 `j .L80038EDC`
      with delay slot 0x80038DAC `addiu $s0,$zero,0xD`.
    * arm B (switch case 9/11): insn at dump line 892 `(set (reg/v:SI 16 s0) (const_int 12))`
      + jump_insn at line 897 `(set (pc) (label_ref 444))` {jump}  -> target bytes 0x80038EC8
      `j .L80038EDC` with delay slot 0x80038ECC `addiu $s0,$zero,0xD`.
  Label 444 == sel_dispatch == .L80038EDC (0x80038EDC). Twelve jumps in this function target it.

- [s45] THE F1 DISCRIMINATOR QUESTION IS ANSWERED, AND THE LEDGER'S PRIOR ANSWER WAS WRONG.
  The 2026-07-17 17:34 Judge instruction was: find the real discriminator protecting the -1/0xF
  pair and respell it honestly at the 13 pair. The ledger recorded the answer as "sched2
  slack-hoisting". That attribution cannot be right (sched2 runs AFTER jump2 and cannot protect
  anything from a jump2 deletion). The real discriminator, read directly out of
  tools/gcc-2.7.2/jump.c:1996: the whole cross-jump attempt is gated by
      `if (cross_jump && simplejump_p (insn))`
  The 0xF pair's if-chain arm terminates in jump_insn 207, a `{branch_equality}` CONDITIONAL
  branch (`(if_then_else (eq (reg s1) (reg v0)) (label_ref 444) (pc))`, dump lines 445-451) whose
  delay slot carries `(set s0 (const_int 15))` (insn 1166). A conditional branch is not
  `simplejump_p`, so cross-jumping is NEVER ATTEMPTED from it — that, not sched2, is what keeps
  the two 0xF producers distinct in the target bytes.

- [s45] WHY THAT DISCRIMINATOR CANNOT BE TRANSPLANTED TO THE 0xD PAIR (byte-level, not
  spelling-level). Transplanting it requires one of the two 0xD arms to reach sel_dispatch via a
  conditional branch rather than an unconditional jump. Both arms' terminators are FIXED BY THE
  TARGET BYTES as unconditional jumps: 0x80038DA8 and 0x80038EC8 are both `0803E3B7`
  (`j .L80038EDC`). The one fall-through slot immediately preceding .L80038EDC is already
  occupied by the default arm (0x80038ED8 `addu $s0,$zero,$zero`). So neither the
  conditional-branch shape nor the fall-through shape is available at the 0xD pair: the
  protective shape is excluded by the target itself, not by our failure to find a spelling.

- [s45] COMPLETE ENUMERATION OF THE REMAINING BREAKERS, from find_cross_jump's own source
  (tools/gcc-2.7.2/jump.c:2403ff, read first-hand this session). The backward walk is:
      i1 = prev_nonnote_insn (i1);
      i2 = PREV_INSN (i2); while (i2 && (NOTE || CODE_LABEL)) i2 = PREV_INSN (i2);
      if (GET_CODE (i1) == CODE_LABEL) { --minimum; break; }        /* LABEL-BONUS: helps merging */
      if (i2 == 0 || GET_CODE (i1) != GET_CODE (i2)) break;         /* insn-code mismatch */
      ... rtx_renumbered_equal_p (p1, p2) ...                       /* pattern mismatch */
  With both terminators identical `{jump}` insns to label 444, the walk's FIRST step compares the
  two `addiu $s0,$zero,0xD` setters. For the merge to fail, the streams must desynchronize at
  exactly that step. The exhaustive option set is:
    (1) different insn CODE at that position — requires interposing a non-NOTE RTL object in
        exactly one stream that emits ZERO bytes. In GCC 2.7.2 that set is exactly {USE, CLOBBER}
        (BARRIER cannot sit between a set and its following jump; a CODE_LABEL there is
        impossible, and a CODE_LABEL in stream 1 hits LABEL-BONUS, which HELPS the merge).
        This is precisely family F5, the union-constructor CLOBBER — REFUSED by owner ruling
        2026-07-19 (decisions.md:789).
    (2) different PATTERN at that position while emitting the same bytes — requires one arm's
        `addiu $s0,$zero,13` to come from an RTL other than `movsi_internal2 (set (reg:SI 16)
        (const_int 13))`. The only candidate is an add-form `(plus (reg X) (const_int 13))`, which
        assembles to `addiu $16,$X,13`; to keep the target bytes X must be $zero, and any C
        expression GCC can prove is zero-plus-13 is const-folded back to `(const_int 13)` before
        jump2 (cse/combine), while any expression it cannot prove allocates a real register and
        changes the register field. The quadrant is empty.
    (3) make one arm not `simplejump_p` or make it fall through — excluded by the target bytes
        above.
  This closes the 2026-07-17 F1 frontier with a SOURCE-LEVEL ENUMERATION rather than an
  exhaustion argument: F5 is not merely the last lever anyone found, it is the ONLY member of the
  breaker set that GCC 2.7.2's own code admits, and it is owner-refused.

- [s45] ENDGAME-LOCK GATE (a) — `python3 tools/scan_hand_coded.py --single func_80038C70`:
  tier=LOW, score=0/8 (427 insns; S1..S8 all unset). Canonical-asm grant path FAILS.
  Additionally the function is jtbl-coupled (its switch emits jtbl_80010BB4, referenced from
  asm/funcs/func_80038C70.s), which is why the 2026-08-19 asm-until-matched migration REFUSED
  it for INCLUDE_ASM — a whole-body asm form would orphan the C-generated jump table.

- [s45] ENDGAME-LOCK GATE (b) — SOTN-master precedent for the closing construct: ABSENT,
  re-verified this session. Grepping docs/reference/sotn-construct-index.md for
  cross-jump / cross_jump / clobber / union returns ZERO hits across all 1,056 indexed entries.
  This independently reconfirms the s7 survey (zero precedent in sotn-decomp / rood-reverse /
  esa) that the owner's 2026-07-19 refusal rested on. Gate (b) FAILS.

- [s45] OWNER-ACT CONFLICT IDENTIFIED (this is the decidable question the packet raises).
  Two owner acts now govern this one function and they point opposite ways:
    * 2026-07-19 (decisions.md:789-812) — OWNER RULING, escalation option (b): the F5 family is
      REFUSED; func_80038C70 is dispositioned as an owner-signed PERMANENT EXCEPTION, keeping the
      byte-correct committed form plus its regfix rule on main as INCOMPLETE-owner-accepted,
      PARKED out of the active queue, with "No further grinding."
    * 2026-08-24 (decisions.md:10358) — OWNER CAMPAIGN rules-to-zero: the final 89 rules "every
      one retiring at that function's COMPLETED-C", with func_80038C70 placed FIRST in the
      campaign lane (1 rule, lowest queue distance) and unparked into active.
  The campaign presupposes a reachable COMPLETED-C for every lane member. For this function that
  presupposition is false unless the 2026-07-19 refusal is revisited: the s45 enumeration shows
  COMPLETED-C requires F5 and nothing else.

- [s45] Chassis re-measured on the current toolchain (post cc1 fork-crash fix, decisions.md:10340): committed form scores 1 with target 402 / build 402 insns; the honest `sel = 0xD` form scores 2 with build 400. The digest's 'ledger floor 10' is stale bookkeeping — the true honest floor is 1 for the committed rule-era form.

- [s45] The residual is exactly one nibble: regfix.txt:1095 `func_80038C70: subst "addiu\t$16,$zero,12" "addiu\t$16,$zero,13" @ 149`, faking `case 9: case 11: sel = 0xC;` into the target's 0xD. It is this function's only rule and one of the campaign's final 89.

- [s45] The two merge candidates are byte-identical tails: arm A (if-chain `if (v0 != 10) sel = 0xD;`) at 0x80038DA8 `j .L80038EDC` + 0x80038DAC `addiu $s0,$zero,0xD`, and arm B (switch case 9/11) at 0x80038EC8 / 0x80038ECC — the same two words. In jump2 RTL they are insn 211 + jump_insn 223 and the pair at dump lines 892/897, both `(set (pc) (label_ref 444))`.

- [s45] LEDGER CORRECTION: the F1 discriminator protecting the 0xF pair is NOT 'sched2 slack-hoisting' (sched2 runs after jump2 and cannot prevent a jump2 deletion). It is the caller gate `if (cross_jump && simplejump_p (insn))` at tools/gcc-2.7.2/jump.c:1996 — the 0xF pair's if-chain arm ends in a conditional {branch_equality} insn, so cross-jumping is never attempted from it.

- [s45] That protection is unavailable at the 0xD pair BY THE TARGET BYTES: 0x80038DA8 and 0x80038EC8 are both 0803E3B7 (unconditional `j`), and the one fall-through slot before .L80038EDC is occupied by the default arm at 0x80038ED8 (`addu $s0,$zero,$zero`).

- [s45] COMPLETE ENUMERATION from find_cross_jump's own source (jump.c:2403ff): with both terminators identical `{jump}`s to label 444, the merge fails only if the streams desynchronize at the first backward step (the two `addiu $s0,$zero,0xD` setters). The three quadrants are: zero-byte non-NOTE interposition = exactly {USE, CLOBBER} = family F5 (owner-refused); same-bytes-different-pattern = only `(plus (reg $zero) 13)`, which GCC const-folds back before jump2 (empty); non-simplejump/fall-through terminator (excluded by target bytes). F5 is the unique breaker GCC 2.7.2 admits.

- [s45] Endgame gate (a): `tools/scan_hand_coded.py --single func_80038C70` -> tier=LOW score=0/8, no indicator set. Also jtbl-coupled (emits jtbl_80010BB4), the reason asm-until-matched refused INCLUDE_ASM for it in 2026-08-19.

- [s45] Endgame gate (b): docs/reference/sotn-construct-index.md (1,056 entries) has ZERO hits for cross-jump / cross_jump / clobber / union — no SOTN-master precedent for the closing construct, independently reconfirming the s7 survey that the owner's 2026-07-19 refusal rested on.

- [s45] OWNER-ACT CONFLICT: decisions.md:789 (2026-07-19) refuses F5 and dispositions this function as an owner-signed permanent exception, parked, 'No further grinding'; decisions.md:10358 (2026-08-24) launches rules-to-zero with this function FIRST in the lane and COMPLETED-C as the stated goal. The campaign presupposes a reachable COMPLETED-C, which the s45 enumeration shows is false unless the F5 refusal is revisited.

- [s45] Decision packet filed this session at docs/grind/decisions.md:10376 with the single decidable question, evidence pointers, and four consequence branches (A exempt / B1 re-sanction F5 / B2 owner-override canonical-asm / B3 fidelity deviation, plus C reframe-as-campaign-success). Grind-lane recommendation: (A) or (C) — the evidence behind the 2026-07-19 refusal is unchanged and was reconfirmed this session.

- [s45] src/ left clean: the honest-0xD probe edit was reverted immediately after measurement (git status shows only docs/grind/decisions.md, the two ledger files, the new rejected form, and metrics/events.jsonl).

## [s46] synthesis modality - merged attack; two ledger corrections; closure re-proved from block capacity

- [s46] CHASSIS RE-MEASURED (this session): `sandbox func_80038C70 --disable all` on committed HEAD =
  score 1, target 402 / build 402, rules_dropped 1 (regfix.txt:1095 subst "addiu\t$16,$zero,12" ->
  "...,13" @ 149), cheat_asm_stripped 7. The honest form (`case 9: case 11: sel = 0xD;`) = score 2,
  target 402 / build 400. Both figures reproduce s45 exactly; src was restored immediately and
  `git status --porcelain` shows only the metrics append. The brief's "ledger floor 1" is correct;
  the historical "floor 10" is stale queue-weighted bookkeeping.

- [s46] NEW MEASUREMENT (nobody had disassembled the merged output in 46 sessions):
  `objdump -d tmp/sandbox/func_80038C70/code6cac_c_mid.o` on the honest-0xD build
  (artifact tmp/grind/func_80038C70/s46/honest.dis) shows arm B's block DOES NOT survive as
  `j; nop` - it is DELETED ENTIRELY. The layout runs `e7c: j d90 / e80: nop` (case-10 goto
  load_sel2) straight into `e84: j e90 / e88: li s0,15` (case 12) and then `e8c: move s0,zero`
  (default). do_cross_jump deletes arm B's set13 and redirects its jump to a new label ahead of
  arm A's set13; arm B's block is then a bare `j` whose only predecessors are ADDR_VEC entries, so
  the jump-to-jump canonicalization retargets the jtbl entries at arm A's block (d64) and deletes
  the block as unreachable. Net -2 insns = the 400-vs-402 figure. CONSEQUENCE: the "let the merge
  fire and let reorg steal the target insn back into the delay slot" idea is dead - after the merge
  there is no jump at arm B for reorg to fill.

- [s46] LEDGER CORRECTION 1 - s45's falsification of the sched2 attribution is ITSELF FALSE.
  s45 argued "sched2 runs AFTER jump2 and cannot protect anything from a jump2 deletion".
  tools/gcc-2.7.2/toplev.c: `schedule_insns` (sched2) is at line 3117; the only cross-jump-enabled
  `jump_optimize (insns, 1, 1, 0)` is at line 3142. sched2 runs BEFORE jump2 - s1 read this
  correctly in 2026-07-07 and s45 overturned it on a wrong premise.

- [s46] LEDGER CORRECTION 2 - s45's "F5 is the UNIQUE member of the breaker set" is FALSIFIED by a
  counterexample inside this same function. s45's replacement discriminator (caller gate
  `if (cross_jump && simplejump_p (insn))`, jump.c:1996) is true but explains the 0xF PAIR, not the
  -1 pair that the 2026-07-17 17:34 Judge instruction was about. The -1 pair's two producers are
  BOTH unconditional simplejumps to label 444 - 0x80038E10 (case 8, jtbl block) and 0x80038E9C
  (case-10 inner path) - so by s45's enumeration they must merge. They do not, in our build or in
  the shipped binary. jump2 RTL (s45/jump2_fn.txt, re-read this session) shows the mechanism:
  case-8's block is [code_label 288 "case8_sel"; insn 291 set s0,-1; jump_insn 293 -> 444], while
  the case-10 block is [insn 358 s1=0; insn 374 set s0,-1; insn 361/363/366/368 (v0=5, sb
  D_800A3207, v0=90, sb D_800A334C); insn 371 sb D_800A3350=0; jump_insn 376 -> 444]. Iteration 1
  compares insn 291 (set -1) against insn 371 (sb mem,0): PATTERN MISMATCH, last1 == 0, no merge in
  either direction. reorg then pulls insn 374 down past four insns into the jump's delay slot,
  restoring target bytes `sb zero,..; j .L80038EDC; delay addiu $s0,-1`.

- [s46] THE OMITTED QUADRANT, NAMED: there IS an honest, byte-neutral, non-F5 cross-jump breaker,
  and this function already ships one. It is NOT s45's "zero-byte interposition" (which is exactly
  {USE, CLOBBER} = F5); it is REAL-INSN INTERPOSITION that costs zero net bytes because the
  interposed insn is already in the block and the displaced `set` lands in the jump's delay slot.
  It requires the arm's jump2-time basic block to hold >= 2 real insns (block slack).

- [s46] BLOCK-CENSUS OF BOTH 0xD ARMS at jump2 (s45/jump2_fn.txt, read first-hand this session):
  arm A = [jump_insn 207 {branch_equality} -> 444; NOTE 209; NOTE 217; insn 211 (set s0 13);
  jump_insn 223 -> 444; barrier 224] - ONE real insn; `insn 1166` (set s0,15) sits in the
  PRE-branch block, before insn 206/jump_insn 207. arm B = [jump_insn 400 -> 1207; barrier 401;
  code_label 403; insn 406 (set s0 12); jump_insn 408 -> 444; barrier 409] - ONE real insn, and
  nothing can fall into it (a BARRIER precedes its label). Each arm's target footprint is exactly
  2 words (0x80038DA8/DAC and 0x80038EC8/ECC).

- [s46] BLOCK-CAPACITY CLOSURE (replaces the family-uniqueness closure): a block ending in an
  unconditional jump with a 2-word footprint can carry at most 2 real insns - one fills its own
  jump's delay slot, and a second could only survive by being donated upward into the PRECEDING
  conditional branch's delay slot. That donation route is closed from reorg.c's own source, not by
  assumption: `fill_slots_from_thread` (tools/gcc-2.7.2/reorg.c:3399) DOES scan the whole thread
  and can lift a non-first insn out of it, but the win test at reorg.c:3466 requires
  `! insn_sets_resource_p (trial, &opposite_needed, 1)` - at arm A the opposite (taken) thread
  needs `$s0 == 0xF`, so NO $s0-writing insn from arm A's block is donatable, and the beq's delay
  slot is necessarily filled from the pre-branch block with `set s0,15` (which is exactly what the
  dump shows). Arm B has no preceding branch to donate to at all. Therefore the insn adjacent to
  each 0xD arm's jump is necessarily its own `set13`; find_cross_jump iteration 1 always counts the
  match, arm B's heading CODE_LABEL always supplies the `--minimum; break` bonus, and the merge is
  unavoidable in ANY byte-matching compile.

- [s46] CORRECTED BREAKER SET (supersedes the s45 enumeration): { F5 zero-byte USE/CLOBBER
  interposition - owner-REFUSED 2026-07-19 and re-refused 2026-08-24; real-insn interposition -
  honest, demonstrated at this function's -1 pair, requires >= 2 real insns of block slack }. The
  honest member is unavailable at BOTH 0xD arms because the target's own 2-word footprints deny the
  slack. The verdict s45 reached is upheld; its stated reason was wrong and is now replaced by a
  premise-free byte-count argument.

- [s46] OWNER DIRECTIVE (2026-08-24 auto-reject class) ACKNOWLEDGED AND EXECUTED for the first time
  in this ledger: the 2026-07-19 permanent-exception clause is VOIDED, F5 stays refused, the
  function keeps grinding under standing policy, and NO standards-lowering packet may be filed. No
  escalation was filed this session; the s45 packet at docs/grind/decisions.md:10376 is treated as
  resolved by that ruling. Solver modality remains untried and is now frontier item 1.

- [s46] src/code6cac_c_mid.c restored byte-for-byte after the probe (backup at
  tmp/grind/func_80038C70/s46/orig_backup.c); regfix.txt untouched; no engine mutation commands run.
  memory/grind/func_80038C70/candidate.c refreshed from the restored HEAD form (the byte-correct
  floor-1 chassis). Full write-up: tmp/grind/func_80038C70/s46/synthesis.md.

- [s46] Chassis re-measured this session: sandbox func_80038C70 --disable all on committed HEAD = score 1, target 402 / build 402, rules_dropped 1 (regfix.txt:1095 subst 'addiu\t$16,$zero,12' -> '...,13' @ 149), cheat_asm_stripped 7. Honest 'sel = 0xD' form = score 2, target 402 / build 400. Both reproduce s45 exactly.

- [s46] NEW: the merged output's bytes were disassembled for the first time in 46 sessions (tmp/grind/func_80038C70/s46/honest.dis). Arm B's block is deleted outright, not left as 'j; nop' — the jtbl entries for cases 9/11 are retargeted at arm A's block (d64). This is the full explanation of the -2 insn delta and it kills the 'let the merge fire, let reorg repair it' family.

- [s46] LEDGER CORRECTION 1: s45's premise 'sched2 runs AFTER jump2' is false. tools/gcc-2.7.2/toplev.c:3117 is sched2; toplev.c:3142 is the only cross-jump-enabled jump_optimize. sched2 runs BEFORE jump2, as s1 recorded on 2026-07-07.

- [s46] LEDGER CORRECTION 2: s45's 'F5 is the unique member of the breaker set' is falsified by this function's own -1 pair — two unconditional simplejumps to the same label 444 that do NOT merge, protected by a REAL-INSN interposition (case-10's set s0,-1 is scheduled five insns above its jump; the pre-jump insn is 'sb D_800A3350,0'; reorg later pulls the set into the delay slot). s45's replacement discriminator (jump.c:1996 simplejump_p gate) is true but describes the 0xF pair, not the -1 pair.

- [s46] CORRECTED BREAKER SET: { F5 zero-byte USE/CLOBBER interposition (owner-REFUSED 2026-07-19, re-refused 2026-08-24); real-insn interposition (honest, shipped at this function's -1 pair, requires >= 2 real insns of block slack) }.

- [s46] CORRECTED CLOSURE (block capacity, premise-free): each 0xD arm's jump2 basic block holds exactly ONE real insn (arm A: insn 211; arm B: insn 406), and each has a 2-word target footprint. A second real insn is byte-free only via donation into a preceding conditional branch's delay slot; reorg.c:3399 permits lifting a non-first thread insn, but reorg.c:3466 rejects any insn setting a resource needed at the opposite thread — arm A's taken thread needs $s0 == 0xF, so no $s0-writing insn is donatable and that slot is filled from the pre-branch block with set s0,15. Arm B is preceded by a BARRIER and has no donation target. Therefore the merge is unavoidable in any byte-matching compile.

- [s46] OWNER DIRECTIVE 2026-08-24 (auto-reject class) executed for the first time in this ledger: the 2026-07-19 permanent-exception clause is VOIDED, F5 stays refused, the function keeps grinding under standing policy, and no standards-lowering packet may be filed. No escalation filed this session; the s45 packet at docs/grind/decisions.md:10376 is treated as resolved by that ruling.

- [s46] Scope: src/code6cac_c_mid.c restored byte-for-byte after the probe (backup tmp/grind/func_80038C70/s46/orig_backup.c); regfix.txt untouched; no engine mutation commands run; git status shows only the three ledger files plus the metrics append. candidate.c refreshed from the restored HEAD form (byte-correct floor-1 chassis).
