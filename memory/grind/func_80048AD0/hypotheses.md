# Hypothesis ledger — func_80048AD0

## s1 (recon, 2026-08-07)

- H1 "arg0-param-reuse counter": the param pseudo's prologue-copy {$a0}
  preference can seat a separate counter in $a0 while `sound` stays u8
  (andi-free call). Probe: `saved = arg0;` + `for (arg0 = 0; ...)`.
  Result: sandbox 20 (48 insns). **KILLED** — copy-prop keeps the param
  pseudo live across call #1 (calls_crossed>0) so global.c bars it from
  caller-saved $a0; it takes $s0 and `saved` costs a second callee-save.
  Corollary lesson (load-bearing for next session): the counter's pref
  source must be a pseudo BORN AFTER snd_LoadBgm.
- H2 "u8-sound-reuse counter": keep sound u8 for the andi-free call and
  eat the QImode counter cost. Result: sandbox 25 (55 insns). **KILLED** —
  every SImode consumer (5 scaled addresses, stored value, compare) forces
  a zero_extend. The reuse trick strictly needs an s32 counter.
- H3 (re-measure of prior m1) "idx-reuse counter, u8 sound": sandbox 2 —
  loop + delta/counter allocation fully match; only the index register
  differs (single pseudo gets $a0 for both roles; target wants index=$v0,
  counter=$a0 as separate pseudos). CONFIRMED as the ledger recorded.

## Live frontier (for s2)

1. Forensics: read global.c expand_preferences pass ORDER/direction (one
   pass? forward over insns?) to determine whether ANY dying-pseudo/SET
   chain can hand {$a0} to a fresh post-call counter pseudo — the counter's
   init `i = 0` has no dying pseudo, so the chain must run through a
   different init spelling that is still semantically real. If provably
   impossible for `(set i (const 0))`, that kills lever (a) wholesale and
   narrows path (ii) to delta-pref removal only.
2. Delta-pref removal (evidence.md lever (b)): find an arg spelling for
   snd_PlayBgm that still emits `addiu $a0,<delta_reg>,0x6E8` at the call
   but whose RTL first operand is not delta's pseudo at set_preference
   time. Note `p_end - base` rewrite is DEAD (emits subu, wrong opcode;
   base also dead pre-loop in target).
3. Permuter campaign seeded from candidate.c (score-1 base, weighted
   metric) sweeping counter-init/call-arg spellings — random+PERM directed;
   fresh-seed discipline per tools/permuter_campaign.py.

## [s1] The (B) s32-sound-reuse body applied WITHOUT declaration change (A) scores 1, not 19 — HEAD's pinned body understates the real floor
- mechanism: sound's snd_LoadBgm arg copy gives the reused counter the {$a0} preference (RTL-verified in prior WIP); the only cost of keeping the u8 prototype is one caller-side andi in the jal delay slot
- probe: applied tmp/cand (B) body with u8 decl kept; sandbox --disable all
- result: score 1, 47/47 insns; sole diff andi vs nop in the snd_LoadBgm delay slot
- verdict: CONFIRMED

## [s1] Reusing the arg0 PARAM as the counter (prologue-copy {$a0} pref) with u8 sound and an explicit saved-copy reaches counter=$a0 andi-free
- mechanism: set_preference records the {$a0} hard-reg copy pref on the param pseudo; a separate saved local was meant to take the $s0 role
- probe: saved = arg0; for (arg0 = 0; ...) variant; sandbox + objdump of the sandbox .o
- result: score 20 (48 insns): copy-prop keeps the param pseudo live across call #1, calls_crossed>0 bars it from $a0, lands $s0 plus a second callee-save for saved; side-finding: idx->$v0 half matched once idx was a separate pseudo
- verdict: KILLED

## [s1] Keeping sound u8 AND reusing it as the counter avoids the call-site andi at acceptable loop cost
- mechanism: u8 var passed to u8 param needs no truncation; question was the QImode counter's SImode-consumer cost
- probe: u8 sound reused as loop counter; sandbox
- result: score 25 (55 insns): zero_extend at all five scaled store addresses, the stored value, and the compare; reuse strictly requires an s32 counter
- verdict: KILLED
