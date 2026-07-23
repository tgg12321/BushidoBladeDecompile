# Hypothesis ledger — func_800692C0

## Live frontier (from s1 recon; ordered)

1. **greg-dump read of the sum/bitpos allocno tie** (mechanism: local-alloc.c
   qty priority = n_refs/live_length; only these 2 allocnos disagree with target).
   Probe: `cc1 <flags> -da base.i`, read ";; Register dispositions" + qty ordering
   for the sum/bitpos pseudos; find the exact n_refs / live_length delta driving
   ours to assign sum→$t1 instead of $t2. Map = destination.
2. **Live-range restructure to flip the tie** (mechanism: sum is return value =>
   longest live range => lower priority => higher reg in ours; target gives sum
   the higher reg $t2, so the goal is to KEEP sum lower-priority than bitpos OR
   flip which one dies first). Probe: introduce an explicit return temp that ends
   sum's live range before bitpos's final `+=0x10` (e.g. accumulate then
   `ret=sum; return ret;` placed to shorten/lengthen the right range); measure
   each. UNTRIED — prior work permuted decl/init ORDER only, never live-range span.
3. **Permuter with hand-trimmed single-fn target** (mechanism: prior import pulled
   sibling func_8004A348 inline-asm => maspsx "too many values to unpack"). Probe:
   build target.o from asm/funcs/func_800692C0.s + prelude.inc at offset 0 (per
   difficult-is-not-impossible §3), seed from candidate.c, random + directed PERM,
   --stop-on-zero. Note the two root causes are COUPLED (li-slot + t1/t2 tie) —
   permuter is the tool for a coupled sched+RA fixpoint.

## [s1] The honest floor for func_800692C0 is 9 with the pin-free candidate.c form.
- mechanism: cheat-invisible sandbox strips the register-asm pins + 1 regfix reorder rule (355 cheat-asm bytes, 1 rule dropped) and scores the pure-C form.
- probe: canonical func_800692C0 (verdict=C, distance=9) + sandbox func_800692C0 --disable all (score=9, 67/67 insns) THIS session
- result: canonical distance=9, sandbox score=9
- verdict: CONFIRMED

## [s1] The 9 diffs reduce to exactly two coupled root causes, and only two allocnos (sum, bitpos) disagree with target.
- mechanism: Register-name diff isolation: target sum=$t2($10)/bitpos=$t1($9), ours reversed; every other allocno (i=$t3, a3_off=$t5, p=$t4, const1=$t6) and the whole diamond body are byte-identical. Second cause: LICM-hoisted `li t6,1` placed at preheader slot 4 (target) vs slot 8 (ours).
- probe: objdump of tmp/sandbox/func_800692C0/text1b.o instruction-aligned against asm/funcs/func_800692C0.s (artifact ours_disasm.txt)
- result: confirmed: sum/bitpos swap accounts for the 5 loop-body diffs + 1 prologue init; li-t6 displacement accounts for the prologue-slot diffs
- verdict: CONFIRMED

## [s1] No sibling/duplicate function provides a transplantable matched analog.
- mechanism: find_duplicates leads list has no entry for this function.
- probe: grep 800692C0 tmp/duplicates_leads.txt
- result: 0 hits
- verdict: CONFIRMED

## [s2] Clean structural init/live-range reordering CANNOT flip the sum/bitpos RA tie.
- mechanism: allocno_compare pri=flog2(nrefs)*nrefs*size/live_length is deterministic; nrefs are fixed by the byte-identical post-jump2 loop body (sum=10, bitpos=9). Achievable livelens are at their structural extremes (sum def-first/returned-last=62 max; bitpos carried init-last=56 min). pri(sum)=4838 > pri(bitpos)=4821 always.
- probe: 10 preheader init orderings + recompute(i<<4, a3_off<<3) + shared-goto accumulate, all measured via tmp/gccdbg/cc1 BB2_ALLOC_DEBUG (standalone sa.c).
- result: no ordering reaches sum ll>=63 or bitpos ll<=55; recompute breaks carried structure (66 insns, score 14); shared-accum drops sum to $t5 (score 16).
- verdict: KILLED (clean structural axis). The RA flip itself IS reachable (worth ~4 pts, 9->5) but every achievable flip either adds an insn (`one` opaque var: 68, cheat) or breaks the carried increment (recompute: 66).

## [s2] Live frontier (refined; ordered)
1. **duplicated-statement-into-arms byte-neutral ref-lift** (SANCTIONED family, needs ruling). Lift bitpos nrefs 9->10 by duplicating a bitpos-reading statement into both sval arms so jump2 re-merges it byte-neutrally; bitpos ll 56 < sum 62 -> pri 5357 > sum -> flip WITHOUT extra insn. This is a last-resort FAKE-annotated sanctioned exception ([[duplicated-statement-into-arms]] = "proven byte-free ref-lift for global-RA priority walls") -> emit ruling-request before adopting; do NOT self-approve. Open Q: is there a bitpos-read that duplicates+merges given the two arms shift by different consts (1 vs 2)?
2. **permuter, clean single-fn target at offset 0** (frontier #3 unchanged). Now seed from candidate.c with the EXACT allocno target in hand (flip sum<->bitpos, keep 67 insns). The coupled li-slot(const1) + t1/t2 tie is the fixpoint.
3. **li-slot(const1) scheduling alone** — const1(li t6,1) sits at target slot 4, ours slot 6-8. Purely scheduler placement; ~3 of the 9 diffs. No clean source lever found this session (all orderings place it late); likely only movable together with the RA fix.

## [s2] Clean structural reordering (init order, increment position, live-range span) can flip the sum/bitpos allocation tie to target.
- mechanism: global.c allocno_compare pri=flog2(nrefs)*nrefs*size/live_length, ties by allocno# (sum#<bitpos# -> sum wins); alloc order == register order. nrefs fixed by the byte-identical post-jump2 loop body (sum=10, bitpos=9). To flip needs sum livelen>=63 OR bitpos livelen<=55.
- probe: tmp/gccdbg/cc1 BB2_ALLOC_DEBUG on standalone sa.c: 10 preheader init orderings + increment-position variants; read exact nrefs/livelen/pri per pseudo.
- result: Every ordering: sum livelen pinned 62, bitpos livelen floor 56 (const1 li always schedules after bitpos def). Gap always >=17; sum(pri4838) always beats bitpos(pri4736/4821). No clean flip.
- verdict: KILLED

## [s2] Recomputing bitpos fresh each iter (bitpos=a3_off<<3, or i<<4) shortens its live range to win $t1 without a cheat.
- mechanism: Fresh (non-carried) bitpos has short livelen -> high pri -> $t1; deriving from a3_off (low-pri) avoids stealing i's register (unlike i<<4).
- probe: Apply bitpos=a3_off<<3 to src, sandbox --disable all; also ALLOCDBG.
- result: RA flips correctly (bitpos->$t1, sum->$t2, return move $2,$10) but drops target's carried `addiu $t1,0x10` -> build_insns 66 vs 67, score 14. Target's bitpos IS carried. Recompute family dead for any source var.
- verdict: KILLED

## [s2] Source-level shared-goto accumulate (matching target's merged .L8006938C addu) gives the target register profile.
- mechanism: Target's single `addu $t2` is a jump2 cross-jump merge that runs AFTER allocation; alloc-time form is two accumulates.
- probe: shared goto-into-arm accumulate, ALLOCDBG + sandbox.
- result: Merge written in source drops sum nrefs 10->6, pri 4838->2033 -> sum lands $t5; also LICM-hoists the 2. score 16. Confirms target's alloc-time form == candidate (two accumulates).
- verdict: KILLED

## [s2] The sum/bitpos RA swap is itself unreachable in pure C (a genuine tie wall).
- mechanism: n/a — testing whether ANY form flips it.
- probe: `s32 one=1; sum += one<<bitpos;` (opaque var) + bitpos=0 last: injects a preheader li inside sum's live range, sum ll 62->63 pri 4761, bitpos ll 56 pri 4821.
- result: REFUTED: the flip IS reachable — RA matches target (return move $2,$10), sandbox 9->5. But it costs a redundant li (build_insns 68 vs 67), a coercion/cheat -> honest floor stays 9. Proves RA-flip is worth ~4pts; sole clean-flip obstacle is +1 sum-livelen at constant insn count.
- verdict: CONFIRMED

## [s3] Duplicated-statement-into-arms can lift bitpos nrefs 9->10 byte-neutrally to flip the RA tie (frontier #1).
- mechanism: SANCTIONED [[duplicated-statement-into-arms]] byte-free ref-lift; duplicate a REAL bitpos-reading statement into both sval arms, jump2 re-merges it -> +1 nref at alloc, +0 emitted insns -> bitpos pri (nrefs10/ll57 ~5262) > sum 4838 -> flip at 67 insns.
- probe: constructed `probe=4<<bitpos` duplicated into both arms; measured live (`return sum+probe`) and dead (`return sum`) via standalone tmp/gccdbg/cc1 ALLOCDBG + insn count (rejected/dup-bitpos-read-not-byte-neutral.c).
- result: (a) target's cross-jump-merged suffix .L8006938C reads bitpos ZERO times => no byte-neutral merge slot for any bitpos read; (b) live duplicate = 62 insns (+2, not byte-neutral); (c) dead duplicate = 60 insns but bitpos nrefs stays 9 (flow deletes dead store before ref-count, INERT). No real+byte-neutral+mergeable bitpos-reading statement exists.
- verdict: KILLED. Byte-neutrality precondition provably fails; NOT a ruling-request (mechanical, not policy).

## [s3] The floor-9 wall reduces to exactly 1 LUID of bitpos livelen, gated entirely by const1 (`li $t6,1`) scheduling.
- mechanism: bitpos nrefs=9 is fixed by the byte-identical post-jump2 loop body. pri(bitpos)=flog2(9)*9/ll=269952/ll. Flip (pri>4838) needs ll<=55. Achievable floor is 56 because const1 (LICM-hoisted `1`) schedules AFTER bitpos's def, lengthening bitpos's range by 1. Target schedules const1 before bitpos's def (preheader slot 4 vs 6) -> ll=55 -> flip.
- probe: pri arithmetic on measured base (ll57->4736, 56->4820, 55->4908) + s2's 10-ordering floor-56 measurement + target-asm slot analysis.
- result: gap is exactly 1 LUID; const1 is a compiler-hoisted invariant with no source statement, so source reordering cannot move it (all structural orderings s2+s3 dead). Pure scheduler-placement fixpoint.
- verdict: CONFIRMED (reframing). The remaining lever is the permuter driving the const1-vs-bitpos-def scheduling to place const1 early at constant 67 insns (frontier #2). Structural modality exhausted.

## [s3] duplicated-statement-into-arms can lift bitpos nrefs 9->10 byte-neutrally to flip the sum/bitpos RA tie at 67 insns (frontier #1).
- mechanism: SANCTIONED [[duplicated-statement-into-arms]]: duplicate a REAL bitpos-reading statement into both sval arms; jump2 re-merges -> +1 nref at alloc, +0 emitted insns -> bitpos pri(nrefs10/ll57 ~5262) > sum 4838 -> flip.
- probe: constructed probe=4<<bitpos duplicated into both arms; measured live (return sum+probe) and dead (return sum) via standalone tmp/gccdbg/cc1 ALLOCDBG + insn count.
- result: Target's cross-jump-merged suffix .L8006938C (addu sum; sh 0,($a3); sh 0,($a2)) reads bitpos ZERO times -> no byte-neutral merge slot. Live duplicate=62 insns (+2, not byte-neutral). Dead duplicate=60 insns but bitpos nrefs STAYS 9 (flow deletes dead store before ref-count, INERT). No real+byte-neutral+mergeable bitpos read exists; NOT a ruling-request (mechanical, not policy).
- verdict: KILLED

## [s3] The floor-9 wall reduces to exactly 1 LUID of bitpos livelen, gated entirely by const1 (li $t6,1) scheduling position.
- mechanism: bitpos nrefs=9 fixed by byte-identical post-jump2 loop body; pri(bitpos)=269952/ll. Flip (pri>sum 4838) needs ll<=55; floor is 56 because const1 (LICM-hoisted 1) schedules AFTER bitpos's def. Target schedules const1 at preheader slot 4 (before bitpos init slot 6) -> ll=55 -> flip (sum=$t2, bitpos=$t1, return move $2,$10).
- probe: pri arithmetic on measured base (ll57->4736, 56->4820, 55->4908) + s2 floor-56 (10 orderings) + target-asm slot analysis.
- result: Gap is exactly 1 LUID; const1 is a compiler-hoisted invariant with no source statement, so source reordering cannot move it (all structural orderings s2+s3 dead). Pure scheduler-placement fixpoint (const1 vs bitpos-def).
- verdict: CONFIRMED
