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
