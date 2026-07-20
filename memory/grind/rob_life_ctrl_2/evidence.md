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
