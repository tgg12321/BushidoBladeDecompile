# Hypothesis ledger — func_8003D888

## s1 (recon, 2026-08-31) — floor 32 → 13

- H1 CONFIRMED: the function is a plain bitstream reader (state {word ptr, cur word, bits
  avail} through the u32* param); natural C compiles to the right CFG and 35-37/37 insns with
  zero coercion constructs. The legacy committed chassis (full register-pin + __asm__ body)
  was pure cheat scaffolding and is replaced.
- H2 CONFIRMED: target's $a1 carrying avail→ptr→word and the in-place `subu a3,a3,a1` mean the
  original REUSES the param `n` as the if-arm ptr/word carrier while a copy (`cnt`) carries the
  bit count. Measured gradient: fresh-locals 30 → param-as-need 26 → merged tmp 22 →
  param-as-ptr/word + cnt copy **13** (candidate.c, 37/37 insns, both arg-copy moves present).
- H3 CONFIRMED (mechanism, greg-dump-proven): our residual seat rotation is driven by
  global.c preference machinery. n's allocno carries "preferences: 5" from the entry copy;
  regs_someone_prefers makes every higher-priority allocno SKIP $a1 (find_reg first pass,
  global.c:1001), so whoever should share $a1 can't. The preference is purged
  (prune_preferences, global.c:908) only when hard $a1 is live inside n's range — achieved
  when local-alloc seats a block-local ptr/word carrier in $a1. Verified empirically: the
  fresh-`w` variant purged the pref (n moved off $a1).
- H4 KILLED (measured): source-order reordering (`n = s[0]` above `cnt -= avail`) to force the
  in-place subtract — scores 20. It does produce the in-place subu, but sched1 then fills the
  avail-load delay slot with the cnt entry-copy, keeping h5 live past the avail load → avail
  conflicts hard $a1 → seat rotation gets WORSE. See rejected/ptr-store-before-subtract-reorder.c.
- H5 OPEN (next session's frontier): the last structural fact to reproduce is target's entry
  sequence `move s; move n; lw avail; nop; slt` — an UNFILLED load-delay slot, i.e. sched1 kept
  both entry copies ABOVE the lw where ours sinks the second copy into the slot. Once that
  holds, per the conflict analysis in evidence.md s1 the seating cascades to target
  (avail→$a1 shares with ptr/word, cnt→$a3, s→$a2). Probe with the .sched dump (read the two
  insns' priorities / dependence classes at the tie) and/or tools/sched_solver; check whether
  the sched-rank-class-tie-wall rule (equal priority ⇒ rank_for_schedule dependence-class
  tie-break) applies to the {n-copy, avail-lw} pair.
- H6 OPEN: remaining if-arm slot diffs at floor 13 (`sllv r` after the `and` vs after the word
  load; `li 32`/`lw word` order) are downstream of seating; re-diff only after H5 resolves.

## [s1] func_8003D888 is a plain bitstream reader (state {word ptr, cur word, bits avail}) writable as natural C with no coercion constructs
- mechanism: semantics from asm + caller func_8003D7B4 (reads 4-bit length then n-bit value; decisions.md:483)
- probe: replaced the committed register-pin/__asm__ chassis with natural C; sandbox --disable all
- result: 35-37/37 insns, correct CFG; gradient 30/26/22/13 across spellings
- verdict: CONFIRMED

## [s1] the original reuses the PARAM n as the if-arm ptr/word carrier ($a1) with a fresh copy (cnt) carrying the bit count; need is decremented in place
- mechanism: target $a1 carries avail->ptr->word; entry copy addu a3,a1 is real; subu a3,a3,a1 is in-place; only the param's entry-copy preference legitimately holds $a1 for ptr/word
- probe: spelled cnt = n; n = s[0]; s[0] = n + 4; n = *(u32 *)n; sandbox
- result: score 13 (from 22), 37/37 insns, s->$a2 ptr/word->$a1 r->$v1 one/m2->$a0 all correct
- verdict: CONFIRMED

## [s1] our residual seat rotation is caused by n's allocno $a1 copy-preference making higher-priority allocnos skip $a1 (regs_someone_prefers), purged only when hard $a1 is live inside n's range
- mechanism: global.c find_reg:1001 first-pass IOR of regs_someone_prefers; prune_preferences:908 purge on hard-reg conflict; set_preference:1661; verified in .greg dump ('73 preferences: 5')
- probe: fresh merged w local variant: local-alloc seats w in $a1 in the if-arm
- result: n's pref purged (n moved off $a1) exactly as predicted; variant scored 20 for a separate scheduling reason
- verdict: CONFIRMED

## [s1] moving the ptr assignment above the subtract (to invalidate the cnt==n cse equivalence and force the in-place subu) lowers the floor
- mechanism: cse.c equivalence substitution extends n's live range into conflict with avail
- probe: reordered source (n = s[0] before cnt -= avail); sandbox
- result: 20 (worse than 13): in-place subu achieved but sched1 fills the avail-load delay slot with the cnt copy, h5 stays live past the load, avail permanently conflicts $a1
- verdict: KILLED
