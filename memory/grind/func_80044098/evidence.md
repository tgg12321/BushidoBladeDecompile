# Evidence bank — func_80044098

## s1 (recon, 2026-07-27) — exact allocno numbers (instrumented cc1, BB2_ALLOC_DEBUG)
- Baseline re-proven: canonical C, sandbox --disable all = 13, 26/26 insns, pin-free candidate in src.
- The 13 diffs are EXACTLY the pointer/counter swap; a6 ($a2=6) and the -1 bound ($a1=5) already match.
- Measured priorities (tmp/gccdbg/cc1, formula global.c:612 `floor_log2(refs)*refs/livelen*10000*size`):
  - counter (pseudo 75): nrefs=14 livelen=16 pri=26250 → allocated FIRST → takes $v1(3)
  - pointer (pseudo 74): nrefs=12 livelen=17 pri=21176 → second → takes $a0(4)
  - -1 const (87): 3/14/2142 → $a1; a6 (76): 3/15/2000 → $a2 (both already correct)
- Ref decomposition (loop refs ×2 depth weight): counter = 8 outside + 3×2 in-loop; pointer = 4 outside + 4×2 in-loop.
- Flip conditions (byte-fixed): pointer needs ≥15 refs @ len 17 (pri 26470) vs full counter; or counter split to its 11-ref minimum (pri ≈24400, see guard-fold constraint below) + pointer ≥14 refs (24705).
- GUARD-FOLD CONSTRAINT (measured kill, s1): if the pre-decrement value is live in a SEPARATE pseudo, combine folds `cnt = a4-1; if (cnt != -1)` → `beq a4,$0` with the dec in the delay slot (score 17, 25 insns, + $sp frame adjust). Generalizes: counter must be ONE pseudo from the 0x7FFF mask through the loop (min 11 refs = mask 2 + sw 1 + dec 2 + guard 1 + loop 6... [mask set counts 1: set,sw,dec set+use,guard,loop 3×2]). Also retro-explains v5's "beqz exit". Counter-side splitting alone can NEVER drop below the pointer's 21176.
- Sibling precedent func_80044100 (matched, same file, same walker loop): pointer lands in $v1 there because the param is still live at the pointer load (2nd table index) → pointer can't take $a0. Our function's param dies at the pointer load insn, so no such conflict exists.
- Fork's build/cc1 does NOT contain ALLOCDBG; the instrumented binary is tmp/gccdbg/cc1 (works with BB2_ALLOC_DEBUG=1).
- m2c shape = candidate shape (no structural info missing).

- WIP rejected_form: reorder a6 before a4 (14, unchanged)

- WIP rejected_form: v1[-1] indexing + --a4 in while-cond (14, unchanged)

- WIP rejected_form: explicit base local copy (base elided, 14 unchanged)

- WIP rejected_form: split header into hdr var for the 0x8000 test (WORSE: 20, hdr->a1, beqz exit, +stack adjust)

- == imported from memory/wip notes.md ==
# func_80044098 — WIP (text1a_c reloc/fixup over a pointer table entry)

## TL;DR
Honest pin-free floor = **13** (sandbox --disable all), == HEAD's honest
distance. HEAD "matches" ONLY via two `register asm()` pins
(`v1 asm("v1")`, `a4 asm("a0")`) — cheats — so the function is INCOMPLETE.
candidate.c is the faithful pin-free body (26=26 insns, 13 diffs).

## The 13 diffs are ONE clean v1<->a0 register rename
Target: pointer (D_80103608[a0] result, walks the array) -> **v1**;
counter/header `a4` -> **a0** (reuses the dying s16 param register).
Our pin-free build: pointer -> **a0**, counter -> **v1** (swapped).

Mechanism: the param `a0` is live until `addu at,at,a0` computes the table
address, then dies. Target loads the pointer into a FRESH low reg v1 (=3,
preferred) and reuses a0 for the LATER header read `lw a0,-4(v1)`. Our GCC
instead reuses the just-dead a0 for the pointer load destination, pushing the
counter to v1. Per [[register-alloc-pure-c]] Step 0: target uses the LOWER reg
(v1=3) for the pointer => OURS is the anomaly; the counter `a4` is
out-prioritising the pointer for v1 (near-tied loop-weighted refs; the pointer
is born one statement earlier so has the slightly longer live range => lower
priority => loses v1). Same tied-priority-rename class as func_80037A20.

## rejected_forms (measured-negative, do NOT re-derive)
- reorder a6 before a4 (v2): 14 diffs, swap unchanged.
- v1[-1] indexing + `--a4` in while-cond (v3): 14, unchanged.
- explicit `base` local, walk a copy (v4): base optimised away, 14 unchanged.
- split header into a separate `hdr` var for the 0x8000 test (v5): WORSE (20;
  hdr -> a1, loop-exit test changes to beqz, +stack adjust).

## Avenues for next session (change MODALITY)
- decomp-permuter from candidate.c (the documented modality for a single tied
  register rename) — NB [[permuter-closability-evaluated]] flags the remaining
  cheat-carrying pool as largely permuter-resistant; this clean single-swap is
  a better-than-average candidate, worth a directed run.
- cc1 -da greg dump to confirm the allocno-priority tie and find a live-range
  lever that raises the pointer's priority for v1 (e.g. shorten the pointer's
  range or lengthen/shrink the counter's) WITHOUT a dead store.

## Floor
- HEAD honest distance: 13 (carries 2 register-asm pins)
- candidate.c honest distance: 13 (zero cheats) — floor NOT lowered below HEAD.


- [s1] Baseline re-proven this session: canonical verdict C; sandbox --disable all = 13 with 26/26 insns on the pin-free candidate applied to src/text1a_c.c

- [s1] Instrumented cc1 (tmp/gccdbg/cc1, BB2_ALLOC_DEBUG=1) exact numbers: counter pseudo 75 nrefs=14 livelen=16 pri=26250 -> $v1; pointer pseudo 74 nrefs=12 livelen=17 pri=21176 -> $a0; -1 const 87 3/14/2142 -> $a1 (matches target); a6 76 3/15/2000 -> $a2 (matches target). The 13 diffs are exactly the pointer/counter register swap.

- [s1] Ref decomposition: counter 8 outside + 3 in-loop (x2 depth weight); pointer 4 outside + 4 in-loop. Flip thresholds: pointer needs >=15 refs at len 17 (pri 26470) vs the intact counter, or >=14 refs (24705) against a load/test-split counter at its 11-ref minimum.

- [s1] Guard-fold constraint (measured): any form leaving the pre-decrement value live in a separate pseudo gets the loop guard folded to beq <pre>,$0 by combine - wrong bytes. Bounds the whole counter-side search space.

- [s1] Sibling func_80044100 (matched, same file, identical walker loop) gets pointer-in-v1 because its param is still live at the pointer load (second table index) - a conflict-based flip our function cannot reproduce (param dies at the pointer load insn).

- [s1] tools/gcc-2.7.2/build/cc1 lacks the ALLOCDBG instrumentation; tmp/gccdbg/cc1 has it - recorded in ledger so future sessions do not rediscover.

- [s1] m2c reconstruction of the target matches the candidate shape exactly - no structural lead there.

## s2 (structural, 2026-07-27) — FLOOR 13 -> 3; peel+hdr+holder form banked
- [s2] frontier-2 CONFIRMED from source: combine.c:52-57 states reg_live_length is never updated and reg_n_refs is "not adjusted ... when a register is no longer required"; the only adjustments (combine.c:2306-2337) zero a DELETED insn's own dest when its last set dies. Flow-time counts on surviving pseudos are frozen through global alloc. Empirical proof: banned mini_pb's cancellation pair counted 16 refs after combine folded it away.
- [s2] frontier-1 (same-path byte-neutral ref lift) KILLED: cse+cse2 run BEFORE flow and the whole function is one fall-through EBB (else-arms are empty exits), so every same-path decoration folds pre-count. 8/8 spellings (cast-addressing, split-addr temp, reload-after-store, mem-RMW, decl swap, loop ptr temp, while-form, lvalue) left pointer at 12 refs / 21176; 6 were byte-identical-and-inert, 2 byte-diverging (rejected/same-path-decorations-cse-collapsed.c).
- [s2] Live-length axis KILLED analytically: counter is born after and dies with the pointer (no asymmetric lengthening site); uniform lengthening keeps pri ratio 1.166*(17+k)/(16+k) > 1 for all k.
- [s2] hdr split (load/test in own var) is byte-IDENTICAL to base (v5's "worse 20" does not reproduce in this spelling) but is an ANTI-lever alone: counter 11 refs at livelen 12 -> pri 27500 (s1's ~24400 assumed livelen stayed 16 — corrected).
- [s2] PEEL mechanism found: hand-peeled first iteration = real-statement duplication; flow counts the peel (+4 pointer refs outside loop), cross-jump re-merges it into the loop. Peel alone: counter wins (17@22=30909 vs 16@23=27826). Peel + hdr split: FLIP — pointer 27826 -> $v1, counter 23333 -> $a0, hdr shares $a0; sandbox = 6 (25/26; guard/loop -1 consts unified because cse follows the while entry jump, LABEL_NUSES==1; a6 gains a 4th ref and steals $a1).
- [s2] m2 = -1 const-holder set INSIDE the arm fixes the small-reg constellation: m2 4 refs @ livelen 16 -> pri 5000 > a6 3809 -> m2 $a1, a6 $a2, guard keeps its own li -> $v0. ALL five regs match target. sandbox = 3 (29/26). Holder set BEFORE the guard fails two ways: cse steals it for the guard compare (one li), and livelen 28 -> pri 3571 < a6 (pO measured).
- [s2] Residual 3 diffs = peel stub [addiu a0,-1; lw v0,0(v1); j mid-loop] + li a1 in the j's delay slot. Mechanism: backward list scheduler (sched1) places the no-in-block-consumer li between the peel body and the entry jump; cross-jump's backward suffix match (subu/sw/inc merged, .L8 at subu) stops at the li; sched2 reorders the leftover [lw,addu] stub to [addu,lw]. INVARIANT under: 12 peel-statement orders, 120 declaration orders, register storage class, block-scope decl-with-init, all 4 m2 positions.
- [s2] In-block m2 consumer (peel a4+=m2) combine-folds (li kept, dependence severed pre-sched1) -> stub persists with different prefix; cross-block m2 consumer (loop a4+=m2) does NOT fold (combine is intra-BB) -> addu $4,$4,$5 wrong bytes. do-while(0) around the peel double-weights the COUNTER's peel refs too (35555 > 34782, flip reverts) and kills the merge. All in rejected/.
- [s2] Sched dump artifacts: tmp/grind/func_80044098/s2/p5_SVA_m0.i.sched (peel block = insns 49-62; the "launching X before Y" stall-avoidance swap picks the block's first insn).

- [s2] combine.c:52-57 + 2306-2337: reg_n_refs/live_length frozen post-flow except zeroing fully-dead deleted dests — frontier-2 settled from source

- [s2] Same-path decoration space is empty by pass order (cse pre-flow + single fall-through EBB): frontier-1 as spelled is dead, measured 8/8

- [s2] Live-length axis dead analytically: counter born after / dies with pointer, no asymmetric site; uniform lengthening preserves pri ratio > 1

- [s2] hdr split is byte-identical to base (v5 'worse 20' does not reproduce) but ALONE is an anti-lever: counter 11 refs @ livelen 12 -> pri 27500 (corrects s1's ~24400 estimate)

- [s2] Peel+hdr-split flip measured: pointer 16@23=27826 -> $v1, counter 14@18=23333 -> $a0, hdr shares $a0; sandbox 6

- [s2] pU (peel+hdr+in-arm m2 holder): sandbox 3, 29/26 insns, ALL target registers correct (v1/a0/v0/a1/a2)

- [s2] Residual mechanism: sched1 places li m2,-1 between peel body and entry jump; cross-jump merges [subu,sw,inc] and stops at the li; sched2 reorders leftover to [addu,lw]; invariant under 136 structural variants

- [s2] sandbox scores this session: base 13 (re-proven), pK 6, pU 3 — all pin-free, zero rules

## s3 (structural, 2026-07-27) — stub mechanism SOLVED; pY banked (3 @ 26/26); both holder families proven 3-locked
- [s3] STUB ROOT CAUSE (frontier-1 settled from source + dump): sched.c adjust_priority (~line 2552) — REG_DEAD notes are always stripped before it runs, so every launched insn hits n_deaths==0; if birthing_insn_p (SET, dest REG, dest live, reg_n_sets[dest]==1, pre-reload) its priority is raised to max_priority (= LAUNCH_PRIORITY 0x7f000001 when launched by the block-end jump, which links every insn). Verified in p5_SVA_m0.i.sched: insn 49 (li m2,-1) shows 0x7f000001 at T-2 -> placed at block tail. Multi-set regs (counter dec, pointer inc) show priority 1, unboosted. The 136-variant invariance is BY DESIGN: single-set constants sink to block tail regardless of source order. THE knob is reg_n_sets[dest] != 1.
- [s3] Dead sets can NEVER bump reg_n_sets: flow.c final pass turns a dead insn into NOTE_INSN_DELETED and `goto flushed` BEFORE mark_set_regs runs (flow.c ~1491-2079). Measured twice byte-identical to pU baseline: pV (trailing `m2 = 0;` after loop) and pX (m2 as unused 2nd parameter — dead entry copy). Both uncounted, boost persists. Corollary: the only post-flow deleter is combine, and any combine-deletable second set is the Judge-banned re-set chain — so NO byte-neutral second set exists for a FRESH holder.
- [s3] pY (holder borrowed into the dead hdr local, `hdr = -1`): boost OFF (2 sets) -> li schedules first (LUID) -> cross-jump re-merges the ENTIRE peel -> sandbox 3 @ 26/26. Structure/schedule/li-placement all exactly target (li lands after guard beq+nop, before loop label — target position). Residual = ONE register: merged hdr pseudo (load+test+mask ∪ holder) lives through the loop -> conflicts loop-temp($v0) + counter($a0) -> lands $a1 -> lw dest + 2 andi srcs differ.
- [s3] p3 (holder borrowed into an explicit test local `tst`): MEASURED 4, worse. Borrowing lengthens tst's range -> pri ~3571 < hdr 5714 -> hdr allocates first, and with no short-lived test temp conflicting it out of $v0, hdr drifts into $v0 (loop temp is in-loop, no conflict with hdr). In pU/pY the implicit test temp (2refs@2len pri 10000) takes $v0 FIRST and its conflict pushes hdr to $a0 — any borrow host must preserve that ordering. rejected/tst-borrow-hdr-drifts-v0.c.
- [s3] THEOREM (borrow-host enumeration complete): a multi-set holder's non-holder segment must emit target insns writing some register R; target writes $a1 exactly once (the li itself), so R ∈ {a0 (hdr/param/counter), v0 (test/guard/loop-temp)} — and BOTH a0 (counter) and v0 (loop temp) conflict with any loop-spanning holder segment; GCC 2.7.2 has no live-range splitting, so the pseudo cannot take two regs. Hosts: hdr -> 3 diffs (pY, measured), tst -> 4 (p3, measured), param a0 -> entry copy materializes as extra move (analytic), a6/v1/a4/loop-temp -> live-conflict or per-iteration redefinition (analytic). FRESH holder -> boost -> 3-insn stub (pU, s2). Both known holder families bottom at exactly 3.
- [s3] pK-track analytic kill (both prerequisites dead): (a) jump1 runs BEFORE cse and normalizes every goto/label respelling (threading + unreachable deletion + jump-to-next), so no spelling bumps the while-test label's LABEL_NUSES pre-cse; (b) even if unification broke, loop.c inserts the materialized const at the peel-block tail before the entry jump -> fresh single-set pseudo -> boosted to tail -> cross-jump match stops at it, ≤ pU's 3.
- [s3] sandbox scores this session: pU re-proven 3 (29/26), pV 3-identical, pX 3-identical, pY 3 (26/26), p3 4. Final src = pY.
- [s3] Judge note: pY's `hdr = -1` is a HYBRID of two sanctioned families — constant-holder (named-local-fake-exception) hosted in a reused dead local (staged-value-reused-variable, whose Origin section names this exact birthing_insn_p/reg_n_sets mechanism). Not literally covered by either rule text alone; flag for classification when a 0 lands from this base.

- [s3] sched.c adjust_priority/birthing_insn_p is the stub's root cause: single-set live-dest constants are boosted to LAUNCH_PRIORITY and sink to block tail by design; the only source-reachable knob is reg_n_sets[dest] != 1

- [s3] flow.c final pass deletes dead insns (NOTE + goto flushed) BEFORE mark_set_regs counts them: dead second sets (pV trailing store, pX dead param entry copy) are byte-inert, measured twice

- [s3] pY (hdr-borrow holder) = sandbox 3 @ 26/26 with exact target structure incl. li a1,-1 placement; residual is one register (hdr -> $a1 vs target a0 on lw + 2 andi srcs)

- [s3] p3 (tst-borrow) = 4: borrow host must keep the implicit short test temp's pri-10000 $v0-grab intact or hdr drifts into $v0

- [s3] THEOREM banked: target writes $a1 exactly once, and both $a0 (counter) and $v0 (loop temp) conflict with any loop-spanning holder segment; GCC 2.7.2 cannot split live ranges, so multi-set holders can never byte-match all segments and fresh holders are always boosted — both known holder families bottom at exactly 3

- [s3] pK-track killed analytically on both prerequisites: jump1 normalizes all goto/label respellings pre-cse (no LABEL_NUSES bump exists), and loop.c would insert the materialized const at the peel-block tail where the boost re-sinks it

- [s3] sandbox this session: pU 3 (29/26) re-proven, pY 3 (26/26) final in src, p3 4

## s4 (permuter, 2026-07-27) — permuter axis measured DEAD; pY re-proven 3 in src; endgame lock confirmed
- [s4] Session start per the binding Judge constraint: src reverted from the pinned HEAD form to pY; sandbox --disable all re-proven 3 @ 26/26, zero rules, zero pins. candidate.c restored to pY (the banned pair form it briefly held is archived as rejected/judge-fail-0727-1613.c).
- [s4] Three concurrent full-window campaigns (tools/permuter_campaign.py, -j 8 each, 32-core host), all harvested + stopped in-session:
  - base-random-classify (canonical do-while chassis, no stop-on-zero): 25,262 iters / 26 min. ONLY 0-attractor is the Judge-banned cancellation-pair class — output-0-1 (prior run, 47 s) and output-0-2 (this run, 504 s, `v1++; v1--; v1++;`) are independent re-finds of the same banned re-set chain. Best non-banned find: score-10 false flip (below).
  - pU-fresh-holder-random (29/26 chassis): 24,074 iters / 26 min, 38 finds, best 15 — the basin CONVERGES onto the known borrow attractor: output-15-1 = `m2 = hdr;` pre-guard staging (pY-family respell: 2nd set on m2 defeats the birthing_insn_p boost, same floor 3), output-25-1 = `m2 = hdr & 0x8000; if (m2)` (p3 tst-borrow family). Both inside the s3 3-lock theorem. Nothing below 15.
  - pY-reseed-full-window (score-3 seed): 24,290 iters / 26 min, 1 "find" = score-15 with an UNINITIALIZED-read mutation (junk). Combined with prior s4a run: ~56k iterations on the pY basin, zero improving finds.
- [s4] Score-10 find DECODED (the only novel topology all session): mask folded into the store (`*(v1-1) = a4 & 0x7FFF` with a4 left unmasked) + dw0 + a6 split-init. It is a SEMANTICS-DIVERGENT false flip — the counter runs unmasked (0x8000 extra iterations), the mask temp lands $v0, and the residual 2 diffs (andi/sw dest $v0 vs target $a0) can only close by masking the counter itself, which restores counter refs and reverts the flip. Full decomposition banked in rejected/perm-false-flip-mask-fold-v0-temp.c.
- [s4] Component probes (workspace compile at offset 0, stripped-dump diff lines, base=26=13 diffs): a6 split-init alone = 26 (INERT); a6 split + dw0 (mask intact) = 26 (INERT); semantics-correct re-mask `a4 = (a4 & 0x7FFF) - 1` = 17 @ 25 insns = the s1 guard-fold kill exactly as predicted. No semantics-correct spelling of the mask-fold topology exists.
- [s4] CONCLUSION: with ~105k cumulative permuter iterations across 4 basins (base ×2, pU, pY ×2), the search finds only (a) the banned pair class, (b) the two 3-locked holder families, (c) semantics-broken junk. Frontier-3 (a base-form pointer-ref lift that is neither decoration nor peel) produced exactly one candidate topology and it is semantics-divergent with a proven-dead correct spelling. Every sanctioned axis is now measured dead → endgame lock per the 2026-07-27 standing auto-ruling; OWNER-ESCALATION filed in docs/grind/decisions.md this session.

- [s4] Session start per the binding Judge constraint: src/text1a_c.c reverted from the pinned s0-HEAD form to pY; sandbox --disable all = 3 @ 26/26, 0 rules, 0 pins, re-proven this session; candidate.c restored to pY (banned pair form remains archived as rejected/judge-fail-0727-1613.c)

- [s4] Three concurrent full fresh-seed-window campaigns via tools/permuter_campaign.py (-j 8 each), all harvested + stopped in-session; telemetry in metrics/events.jsonl (permuter-launch/wait/harvest events)

- [s4] base basin: 25,262 iters, 8 new finds; only 0s are the banned cancellation-pair class (output-0-2 = v1++;v1--;v1++ re-set chain, an independent re-find of the ruled-UNSANCTIONED class)

- [s4] score-10 base find decoded as a FALSE FLIP: mask folded into the store with a4 left unmasked drops 2 counter refs, flipping allocation, residual = andi/sw dest $v0 vs $a0; closing it requires mask-into-counter which restores counter refs and reverts the flip — topology cannot reach 0; banked as rejected/perm-false-flip-mask-fold-v0-temp.c

- [s4] pU basin: 24,074 iters, 38 finds, best 15 = m2=hdr pre-guard staging (a respell of the pY borrow family: 2nd set defeats the birthing_insn_p boost, same 3-lock); score-25 = tst-borrow (p3 family); nothing outside the s3 theorem

- [s4] pY basin: 24,290 iters this reseed + 32k prior = ~56k cumulative, zero improving finds

- [s4] Cumulative ~105k permuter iterations across 4 basins find only: (a) the banned pair class, (b) the two proven-3-locked holder families, (c) semantics-broken junk — the permuter modality (last live grindable axis per the s3 frontier) is measured dead

- [s4] OWNER-ESCALATION entry filed at docs/grind/decisions.md:1789 naming func_80044098, citing the 2026-07-27 17:57 Judge ruling ('Both gates fail -> the standing auto-ruling applies') and requesting the standing-auto-ruling disposition: OWNER-ACCEPTED INCOMPLETE, terminal park, re-attempt eligible
