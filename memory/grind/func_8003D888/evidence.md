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

## s2 (structural, 2026-09-01) — floor stays 13; the seating mechanism is now fully named

Chassis note: the s1 working-tree body was NOT committed — HEAD still carries the legacy
register-pin/`__asm__` cheat chassis for func_8003D888. `candidate.c` re-applied cleanly and
re-measures **13** (37/37 insns), so every s1 spelling conclusion is chassis-confirmed.

### Target role assignment (re-read from asm/funcs/func_8003D888.s, not inherited)
`$a2`=s (both arms), `$a1`=avail → ptr → word (ONE carrier, three values), `$a3`=count
(copy of param `n`, decremented in place `subu a3,a3,a1`), `$v1`=r, `$a0`=const 1.
Both entry copies are real insns: `addu a2,a0,zero` and `addu a3,a1,zero`.

### Measured spellings this session (all `sandbox --disable all`)
| form | score | insns | seats (s / carrier / count) |
|---|---|---|---|
| candidate.c (s1: avail separate, param reused for ptr/word, fresh `cnt`) | **13** | 37 | $a2 / $a1 / $t0 (avail $a3) |
| A = merged carrier `t` (avail→ptr→word) + param `n` as in-place count | 22 | 36 | $a3 / $a2 / $a1 |
| E = param `n` carries avail→ptr→word, fresh `cnt` copy is the count | 22 | 36 | $a3 / $a2 / $a1 |
| B = A but ptr/word is a block-local `p` (avail stays global) | 20 | 36 | $t0 / $a1(local-alloc) / $a2, avail $a3 |
| A2/A3 = A with the if-arm statements re-associated (`r <<= n` late; `m2` late) | 22 | 36 | identical to A |

A/E reproduce the target's ENTIRE insn stream (including the in-place `subu` and the
`li a0,1` in the branch delay slot); their only defects are the 3-cycle seat rotation and the
resulting loss of the count's entry copy (36 vs 37 insns).

### The gate, named at instruction level (FINDREGDBG on the instrumented cc1, variant A)
    FINDREGDBG func=func_8003D888 pseudo=74 conflicts: 2 3 4 29
    FINDREGDBG  someone_prefers: 3 5          <-- $a1 excluded
    FINDREGDBG  pass0_used: 0 1 2 3 4 5 16..23 26..31
    FINDREGDBG  own_copy_prefs: (empty)   own_full_prefs: (empty)
`find_reg` (global.c) pass 0 ORs in `regs_someone_prefers` and the complement of
`regs_used_so_far`; the carrier allocno (74 in A) therefore skips $a1 and takes $a2 — pass 0
succeeds, so the pass-1 fallback (which ignores `someone_prefers`) is never reached.
$a1 is in `someone_prefers[carrier]` because the COUNT allocno holds an $a1 copy-preference,
and the count holds it in every spelling measured:
  - A (count = the param pseudo): the entry copy `(set 73 (reg a1))` gives it directly.
  - E (count = a fresh local copied from the param): cse propagates the hard reg into the copy
    (`(set 74 (reg a1))`), which additionally KILLS the carrier's own entry copy as dead code —
    greg for E shows `74 preferences: 5` and NO preference line for 73. Confirmed in the dump.
`prune_preferences` (global.c:877-931) can rescue the carrier two ways only:
  (a) purge the count's pref — needs hard $a1 live inside the count's range; the only window
      where the count is live and the carrier is dead is the else-arm tail
      (`li v0,1; sllv v0,v0,a3`), where local-alloc already has $v0/$v1 free and no extra insn
      exists to occupy $a1. Variant B forces a hard-$a1 occupant (block-local `p`) but it lands
      INSIDE the carrier's range too, so all three globals inherit the $a1 conflict and spill
      out to $t0 (score 20).
  (b) the "we also prefer it" carve-out (global.c:918-928, `allocno_size[j] <= allocno_size[i]`
      ⇒ temp &= ~own_full_prefs): needs the carrier to hold its OWN $a1 full-preference.
      `expand_preferences` (global.c:824-871) only merges preferences between allocnos that do
      NOT conflict, and carrier/count are simultaneously live by construction, so the merge can
      never fire. The carrier can only get an $a1 preference from a surviving entry copy of the
      param — i.e. the carrier must BE the param variable AND the param's incoming value must
      have a use that cse cannot rewrite to hard $a1 (in E it has exactly one: the count copy).

### Second gate: allocation ORDER (this is what H6 really is)
Even with the carrier in $a1, target needs s→$a2 before count→$a3, i.e.
`allocno_compare` priority(s) > priority(count). Measured (lreg): s = 9 refs / 26 insns
(pri 3*9/26 = 1.038), count = 9 refs / 24 insns (pri 3*9/24 = 1.125) ⇒ the count is ordered
FIRST and takes $a2. Target therefore requires the count's live length ≥ 26 (a tie hands it to
the lower allocno number, 72 = s) or its ref count down to 8. The difference is visible in the
target's if-arm schedule: `sllv v1,v1,a3` (r <<= count) sits LATE (after `lw a1,0(a1)`), where
ours emits it immediately after the `and`, so the target's count lives ~3 insns longer.
**H6 is upstream of seating, not downstream of it** — it feeds allocno priority.

- [s2] Statement re-association inside the if-arm is codegen-NEUTRAL here: A2 (`r <<= n` moved
  after the s[2] store) and A3 (`m2` computed after the word load) both emit byte-identical code
  to A (22/36). sched1 rebuilds the block from the dependence DAG + INSN_PRIORITY, so source
  order inside a basic block is not a lever for the live-length gate; the lever must change the
  DEPENDENCE GRAPH, not the text order.
- [s2] H5 (s1) is KILLED: the .sched dump shows block 0's scheduling region is insns {12,15,16}
  only — the two entry copies (insns 4 and 6) sit above NOTE_INSN_FUNCTION_BEG and sched1 never
  moves them. The `move` that appears in the load-delay slot in the 13/20 builds is placed by
  sched2/the delay-slot filler AFTER register allocation and therefore cannot influence seats.
  The target's `nop` at 8003D894 is an assembler-inserted load-delay nop, not a scheduling
  artifact — so "reproduce the unfilled load-delay slot" was never a reachable lever.

- [s2] Harness note for future sessions: `src/code6cac_c2.c` contains BOTH an `extern s32
  func_8003D888(...)` prototype (line ~969, above the caller func_8003D7B4) and the definition
  (~line 996). A body-splicing script that searches for `'s32 func_8003D888('` matches the
  PROTOTYPE first and silently deletes func_8003D7B4 and its neighbours; anchor on
  `'\ns32 func_8003D888('`. The sandbox still scores 13 on the mutilated TU, so the corruption
  is invisible to the gradient — verify with `git diff --stat src/` before banking a form.

- [s2] HEAD still carries the legacy register-pin/__asm__ cheat chassis for func_8003D888 (s1's natural C was never committed); memory/grind/func_8003D888/candidate.c re-applies cleanly and re-measures 13 (37/37 insns) on the current chassis.

- [s2] Target roles re-read from asm: $a2=s, $a1=avail->ptr->word (one carrier, three values), $a3=count (copy of param n, decremented in place subu a3,a3,a1), $v1=r, $a0=const 1; both entry copies are real insns.

- [s2] Variants A and E reproduce the target's ENTIRE insn stream including the in-place subu and li a0,1 in the branch delay slot; they score 22 only because of a 3-cycle seat rotation (s/carrier/count = $a3/$a2/$a1 vs target $a2/$a1/$a3), which also dissolves the count's entry copy (36 vs 37 insns).

- [s2] FINDREGDBG (instrumented cc1 at tools/gcc-2.7.2/cc1, BB2_FINDREG_DEBUG=74, variant A): conflicts 2 3 4 29 / someone_prefers 3 5 / own_copy_prefs and own_full_prefs both EMPTY -- the carrier has no preference of its own, so the prune_preferences 'we also prefer it' carve-out cannot fire.

- [s2] expand_preferences (global.c:824-871) merges preferences only between NON-conflicting allocnos; carrier and count are simultaneously live by construction, so the carrier can never inherit an $a1 preference from the count.

- [s2] The only window where the count is live and the carrier is dead is the else-arm tail (li v0,1; sllv v0,v0,a3); occupying hard $a1 there would purge the count's preference, but local-alloc has $v0/$v1 free and no spare insn exists. Variant B forces a hard-$a1 occupant (block-local ptr/word) but its range lands inside the carrier's range too, so all three globals inherit the $a1 conflict and s spills to $t0 (score 20).

- [s2] Harness hazard banked: src/code6cac_c2.c has an extern prototype for func_8003D888 ABOVE its caller func_8003D7B4; a splice script anchored on 's32 func_8003D888(' matches the prototype and silently deletes the caller. The sandbox still prints 13 on the mutilated TU, so always git diff --stat src/ before trusting a measurement.
