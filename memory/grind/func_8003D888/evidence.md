# Evidence bank — func_8003D888

## s1 (recon, 2026-08-31)
- Function semantics: bitstream reader. State struct through `u32 *s`: s[0]=word pointer,
  s[1]=current word, s[2]=bits available. Returns next `n` bits. Caller func_8003D7B4 reads a
  4-bit length then an n-bit value (see decisions.md:483 PASS entry — semantic context).
- Committed body at session start was a legacy full register-pin + `__asm__` cheat chassis
  (sandbox stripped 25 cheat insns from it; floor read 32/37). Replaced in working tree with
  natural C this session.
- Floor gradient measured this session (sandbox func_8003D888 --disable all):
  - natural C, fresh locals (need/m1/m2/p separate): **30**
  - `n -= avail` param-reuse + late `shift = 32 - n` + separate `word` local: **26**
  - single reused `tmp` variable carrying avail→ptr→word (matches target $a1 role): **22**
    (build 36 insns vs target 37; missing insn = second arg-copy move `addu a3,a1`)
- Target register roles: s→$a2 (whole fn), n/need→$a3 (param decremented in place:
  `subu a3,a3,a1`), avail→ptr→word all in $a1, one/m2→$a0, m1/temps→$v0, r→$v1.
- At floor 22 the residual is (a) a 3-cycle seat rotation: ours tmp→$a2, s→$a3, n→$a1 vs
  target tmp→$a1, s→$a2, n→$a3; (b) two if-arm scheduling slots (`sllv r` placed after the
  `and` instead of after the word load; `li 32/subu` vs `lw word` order) which likely shift
  once seating is fixed.
- .greg dump (tmp/grind/func_8003D888/dumps/code6cac_c2.greg:8868): 4 global allocnos,
  allocation order 74(tmp) 73(n) 72(s) 75(r). **73 preferences: 5** — n's entry-copy
  preference for hard reg $a1. Mechanism: global.c find_reg skips regs in
  regs_someone_prefers on the first pass, so tmp (allocated first) skips $a1 → takes $a2;
  s skips $a1 → $a3; n then takes its preferred $a1 and its entry copy dissolves to a
  self-move (insn 6 `a1=a1`). 72(s) shows NO preference for $a0 (its copy pref was purged
  because 72 conflicts with hard reg 4 — $a0 hosts the `1` constant across n's range).
- Inference for target: n's $a1 copy-preference must be ABSENT (if it existed, tmp would
  have skipped $a1 the same way), and with no preference n allocates after s and takes next
  free $a3, giving exactly target seating tmp→$a1, s→$a2, n→$a3 with both entry moves real.
- Artifacts: tmp/grind/func_8003D888/s1/build_func.txt (current disasm),
  tmp/grind/func_8003D888/dumps/code6cac_c2.greg (allocation dump — NB generated against the
  floor-22 merged-tmp form, not the final candidate; rerun dump.ps1 before reading it against
  the floor-13 body).

### Later s1 measurements (param-reuse breakthrough)
- **Floor 13** (candidate.c, in src/ working tree, 37/37 insns): param `n` reused as the
  if-arm ptr/word carrier, fresh `cnt = n` carries the bit count. Seats: s→$a2 ✓, ptr/word→$a1 ✓,
  r→$v1 ✓, one/m2→$a0 ✓; wrong: avail→$a3 (target $a1), cnt→$t0 (target $a3); our subu
  recomputes `cnt = n - avail` (cse substitutes n for cnt) vs target in-place `subu a3,a3,a1`.
- prune_preferences (global.c:877-931) + set_preference (global.c:1661-1755) read in full:
  full-prefs propagate through copies; an allocno's own pref removes that reg from its
  avoid-set (lines 924-926); prefs are purged when the preferred hard reg conflicts (908-910).
- Purge confirmed empirically: fresh merged `w` local variant → local-alloc seats w in $a1
  inside the if-arm → n's $a1-pref purged, n moves to $a2 (score 20 for other reasons).
- Both score-20 variants share the killing defect: sched1 fills the avail-load delay slot with
  the n/cnt copy, so hard $a1 stays live past the avail load and avail acquires a permanent
  conflict with h5. Target instead shows `move; move; lw; nop; slt` — the load-delay slot
  UNFILLED with both copies above the lw. Reproducing that copy-above-lw schedule is the
  gating lever for the endgame seating (H5 in hypotheses.md).

- [s1] Committed body at session start was a legacy full register-pin + __asm__ cheat chassis (sandbox stripped 25 cheat insns); replaced with natural C in src/code6cac_c2.c (floor-13 body in tree, mirrored in candidate.c).

- [s1] Measured spelling gradient: fresh locals 30; n-=avail param reuse 26; single merged tmp (avail/ptr/word) 22; param-as-ptr/word + cnt copy 13 (37/37 insns).

- [s1] greg dump: 4 global allocnos, order 74(tmp) 73(n) 72(s) 75(r); '73 preferences: 5' is the seat-rotation driver; global.c:877-931,1661-1755 read and mechanism confirmed.

- [s1] Target entry sequence move/move/lw/nop/slt has an UNFILLED load-delay slot: sched1 must keep both entry copies above the avail lw; our build fills the slot with the second copy, creating the avail-vs-h5 conflict that blocks target seating.

- [s1] Residual at floor 13: avail->$a3 vs target $a1, cnt->$t0 vs target $a3, recomputed subu vs in-place, plus two if-arm schedule slots (sllv r placement, li 32/lw word order).
