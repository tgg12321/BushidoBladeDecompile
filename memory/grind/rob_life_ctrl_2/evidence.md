# Evidence bank — rob_life_ctrl_2

- WIP rejected_form: {'form': 'HEAD body, rules removed (control)', 'score': 2, 'reason': 'DIFF (sha 99ee75aa) — proves the 2 rules are load-bearing.'}

- WIP rejected_form: {'form': 'split t mult and >>12 into 2 statements (t=...; t=t>>12; and t>>=12;)', 'score': 2, 'reason': "DIFF (sha 6d4e7ac0). Already in card's negative list; re-confirmed."}

- WIP rejected_form: {'form': 'collapse p into single `p=(v*(0x1000-s))>>12` statement', 'score': 2, 'reason': 'DIFF (sha a2f90405) — different bytes, does not fix the t-mflo.'}

- WIP rejected_form: {'form': 'register asm() pin on t', 'score': 2, 'reason': 'IGNORED by GCC: binds named pseudo 81 (already correct) not the unnamed mult-result pseudo 104. Also a cheat. (from prior card.)'}

- WIP rejected_form: {'form': 'reorder t after bounds check / local fc=0x1000-f / swap q-t order / decl-order swap', 'score': 2, 'reason': "All DIFF (from prior card's documented negative levers 1-5)."}

- == imported from memory/wip notes.md ==
# rob_life_ctrl_2 — WIP (blocked-lane triage 2026-06-14)

## TL;DR
HSV->RGB converter (text1a_c.c:0x80042504). Matches ONLY with 2 regfix subst
rules. 2-insn **register-allocation plateau** on the 5th mflo. NOT closed this
pass — the documented next modality (instrumented allocno-priority dump) was
not run here; all quick structural levers re-confirmed NEGATIVE.

## The exact gap (register allocation)
Source: `t = (v * (0x1000 - ((s * (0x1000 - f)) >> 12))) >> 12;`
- TARGET: t's mult result `mflo a1` ($5), then `sra a1,a1,0xc` in the beqz
  delay slot. a1 is the `li a1,4096` (0x1000) constant register — target
  reuses it for the mflo AFTER the constant's last `subu a1,...` minuend use.
- OUR build: `mflo $8` (t0), then `sra $5,$8,12`. GCC RA places the t-mult
  pseudo in $8 instead of reusing the dying a1.
- The 2 regfix rules rewrite exactly these 2 instructions (mflo dest + sra src).

The whole question is liveness: for GCC to reuse a1 for t's mflo, the 0x1000
constant pseudo must die exactly at t's subu. It currently doesn't (or the
priority tiebreaker picks $8 first).

## Resume steps (next session)
1. Run the INSTRUMENTED ALLOCNO DUMP — the named next step. Build tmp/gccdbg/cc1
   with BB2_ALLOC_DEBUG / BB2_PRIO_DEBUG per
   memory/project/register-alloc-deep-dive.md, on an isolated standalone of
   rob_life_ctrl_2. Read why the t-mult pseudo isn't given a1.
2. Test src forms that end the 0x1000 constant's live range exactly at t's
   subexpression (without splitting the constant into a second pseudo — q also
   uses 0x1000 and it must stay CSE'd to one pseudo).
3. If the dump shows the global.c:624 priority-tiebreaker wall (same class as
   cpu_side_move_dir_4 / marionation_Exec in register-alloc-deep-dive.md), this
   is a genuine pure-C wall -> escalate the canonical-asm-vs-keep-parked policy
   question. (Unlikely canonical: ordinary HSV->RGB arithmetic, not hand asm.)

## Ruled out (do NOT re-derive)
- HEAD body with rules removed -> DIFF (proves rules load-bearing).
- split t's `>>12` into 2 statements (`t=...; t=t>>12;` and `t>>=12;`) -> DIFF.
- collapse `p=(v*(0x1000-s))>>12` into one statement -> DIFF (different bytes).
- register asm() pin on t -> IGNORED (binds the wrong pseudo) AND a cheat.
- (prior card negatives) reorder t after bounds check; local fc=0x1000-f;
  swap q/t order; decl-order swap -> all DIFF.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (the lever ladder + the two known walls)
- `memory/project/register-alloc-deep-dive.md` (instrumented-dump recipe)


- [s1] HEAD 08f1aa37; sandbox floor=2 unchanged from ledger s0 wip-import

- [s1] canonical: C / distance 2 (pure-C target, not ASM-region/structural — pure-C match provably exists)

- [s1] regfix.txt:487-488 carries the 2 rob_life_ctrl_2 substs; both stripped by sandbox --disable all and score stays at 2 (rules are load-bearing, not vestigial)

- [s1] diff isolated to 2 adjacent insns at 0xC4/0xCC: t's mflo dest + its dependent sra src; all 94 other insns byte-match

- [s1] Kengo annotation confirms function name (my_rob/rob_life_ctrl_2, 96i, x2 size collision) — ordinary HSV->RGB arithmetic, NOT hand-written asm; canonical-asm route is not the answer here

- [s1] tmp/duplicates_leads.txt: NO near-clone lead for rob_life_ctrl_2 (single unique HSV->RGB in project; inverse mot_data_set at text1a_c.c:96 is RGB->HSV, different shape)

- [s1] Prior sessions exhausted structural levers: HEAD-body/rules-off (DIFF), split t mult and >>12 into 2 stmts (DIFF), collapse p into single stmt (DIFF), register asm() pin on t (IGNORED, cheat), reorder-t/local-fc/swap-q-t/decl-order-swap (all DIFF), plus 4 more rules-removed full-build variants (all DIFF, session s1 2026-06-14)

- [s1] The named next un-tried modality (ledger next_hypotheses[0], notes.md resume-step 1) is the INSTRUMENTED ALLOCNO DUMP (BB2_ALLOC_DEBUG/BB2_PRIO_DEBUG tmp/gccdbg/cc1 per memory/project/register-alloc-deep-dive.md) — has NOT been run

- [s2] s2 baseline confirmed: score=2, cheat_asm_stripped=78, rules_dropped=2 at HEAD 08e1a3f0 (post s1 ledger update commit 8e9d1138).

- [s2] s2 objdump byte-diff (probe1..probe4 all identical): the residual is ONLY at offset 0xC4 (`mflo t0` should be `mflo a1`) and 0xCC (`sra a1,t0,0xc` should be `sra a1,a1,0xc`). All 94 other insns byte-match, including the entire prologue/scheduling of the mult latencies (targets sras and sltiu are perfectly hoisted into mult-latency window, IDENTICAL to target).

- [s2] s2 negative axis measured: 4 distinct block-local-named-intermediate + decl-order structural forms ALL produce byte-identical output to HEAD. This confirms that GCC's RTL-level CSE/scheduler has already normalized the arithmetic DAG to its canonical form for this expression — additional C-level named intermediates and declaration-order shuffles are folded away before allocation and cannot affect the pseudo-104 tiebreaker.

- [s2] s2 mechanism eliminated: source-level extraction of `0x1000 - f` explicitly (probe2) reproduces target's target-0x32D8C `subu v0,a1,v0` position IDENTICALLY to HEAD's implicit CSE — the CSE was never the issue; RA tiebreaker at 0xC4 is downstream of it.

- [s2] s2 target vs ours structural review: target 0x32D6C..0x32DD0 vs ours 0x30..0xCC are byte-identical except at 0xC4/0xCC. Both builds schedule sltiu-before-mflo, both hoist the 3 post-mult sras into the mult-latency window, both make a1 (the 0x1000 constant pseudo) die at t's outer subu. The ONLY divergence is which hard reg GCC picks for the mult-result pseudo when both $a1 (freed) and $t0 (never used) are eligible.

- [s2] s2 conclusion: the residual is a pure global.c allocno-priority TIEBREAKER, not a C-source structural gap. Structural axis (frontier hypotheses 2 & 3) is exhausted for THIS diff shape. The remaining sanctioned modality is the forensics axis — the instrumented allocno-priority dump (frontier hypothesis 1) — which is what the s1 ledger and WIP notes named as the un-run resume avenue.
