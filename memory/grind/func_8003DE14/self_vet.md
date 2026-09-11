# SELF-VET - func_8003DE14 (session 38, STRUCTURAL)

STATUS: SUBMISSION. `sandbox func_8003DE14 --disable all` = score 0, build_insns
173 == target_insns 173, rules_dropped 0, with the body of
memory/grind/func_8003DE14/candidate.c applied to src/code6cac_c2.c THIS session.

This body carries NO `h` carrier and NO `total` staging borrow. The construct the
s36 Judge ruling FAILed (the fresh multi-written local `h`) is absent; so is the
`total` borrow that ruling granted as its replacement - s38 removed the need for a
latch carrier entirely.

CONSTRUCTS: (1) the inner loop bound's combine-foldable detour
`while (j < rect[2] * rect[3] + rect[2] - rect[2])`; (2) the s21 combine-foldable
detour `((s32)dst_buf + j) - j` on the LoadImage argument; (3) `gm`, a fresh
once-written named intermediate for the green channel's masked result.
(The natural-order OR chain `(pixel & 0x8000) | r_src | gm | (px & 0x7C00)` is
ordinary C in its natural channel order and is not counted as a construct - T5.)

## T1 semantic purpose
- bound detour: none. `+ rect[2] - rect[2]` is algebraically null; combine folds
  it back to `rect[2] * rect[3]` and emits nothing (173 insns with and without).
  Its only surviving effect is the reference count flow.c records before the fold.
  FAKE by construction, annotated as such.
- j extender: none, identically. FAKE by construction, annotated (banked s21).
- `gm`: names a real sub-expression the target itself computes and stores
  (`andi $v1,$v1,0x3E0` / `or $v0,$v0,$v1`, asm/funcs/func_8003DE14.s:132,137).
  Removing the name does not change the value but does change the bytes (17/173),
  so its effect is codegen, not semantics - hence FAKE-annotated.
- OR chain: full semantic purpose - it is the pixel the function stores.

## T2 human-programmer
- bound detour: NO. A reader asks "why + rect[2] - rect[2]?". It fails T2 openly,
  which is precisely why it is claimed under a sanctioned FAKE family and carries
  a mechanism annotation, rather than being presented as ordinary C.
- j extender: NO, same, and same disposition (sanctioned since s21).
- `gm`: yes - a reader sees a named mask; mildly redundant, not alien.
- OR chain: yes - sign, red, green, blue is the function's own channel order.

## T3 GCC-internals justification
Yes, and each mechanism is DUMPED, not guessed.
- bound detour: local-alloc.c:1669-1684 `qty_compare_1` ranks quantities by
  floor_log2(n_refs)*n_refs*size/(death-birth). The two halfword loads of the
  bound both die at the shared `mult`, so the earlier-born rect[2] load has the
  strictly larger denominator and, at the natural equal reference counts, sorts
  SECOND and is handed $v1 where the target has $v0. The detour's two extra reads
  CSE onto the same pseudo, so flow.c counts 12 weighted refs instead of 6 and the
  priority goes from 3 to 9 against the rect[3] load's 6. Measured:
  tmp/grind/func_8003DE14/s38/qty_win.log:623-624
  (`blk=11 ord=0 qty=0 reg1=147 birth=4 death=8 refs=12 got=2` and
  `ord=1 qty=1 reg1=150 birth=6 death=8 refs=6 got=3`) against the detour-free
  body's tmp/grind/func_8003DE14/s37/qty_g6.log (reg1=147 refs=6 -> got=3).
  The pass-ordering fact that makes a folded-away read count at all -
  `reg_n_refs` is computed in flow, which runs before combine - was banked in s37
  (hypotheses.md H37-2) and is re-confirmed by the insn count being unchanged.
- j extender: the same flow-before-combine ordering on `j`'s count (s21).
- `gm`: global.c allocno priority (prio = nrefs*40000/live_length), rows dumped
  via BB2_ALLOC_DEBUG to tmp/grind/func_8003DE14/s36/alloc.log.
Per the checklist a GCC-internals mechanism is a cheat SIGNAL - which is exactly
why each construct is claimed under a named sanctioned family below, carries a
FAKE annotation naming the pass, and cites its lever-exhaustion ledger.

## T4 permuter/search provenance
No permuter output is committed. The bound detour was NOT found by search: s37
derived its precondition in closed form as a CLASS kill ("a tie needs
floor_log2(r0)*r0 >= 24, i.e. a third in-block reference to the rect[2] value")
and s38 supplied that reference by the mechanism s37 had already banked
separately. The confirming sweep (8 spellings, tmp/grind/func_8003DE14/s38/v1/)
came AFTER the prediction, and the dumped qty priorities explain the winner
mechanistically. `gm` came from tools/spelling_enum.py in s36 but is kept on its
dumped ALLOCDBG rows and a closed-form inequality derived in s35 BEFORE that
enumeration ran.

## T5 family check
- bound detour - combine-foldable chain-extender (F1), claim below. It is the
  clause's exact shape: a LIVE computation (the loop bound) routed through an
  algebraically-equivalent detour, folded back by combine, zero emitted bytes,
  surviving effect = reg_n_refs only. It is NOT the forbidden or-tree-shape-shift
  (no associative/commutative reordering of an existing OR/AND tree is involved,
  and the multiply's operand order is unchanged), NOT a DImode chain, NOT a
  volatile coercion, NOT a dead local, NOT a frame pad, and it materializes no
  bytes (the clause's own extra prerequisite).
- j extender - same family, banked s21, unchanged.
- `gm` - named-intermediate, claim below; all six prongs measured.
- OR chain - or-tree-shape-shift section "What IS allowed": the natural channel
  ordering; the paren-free spelling measures byte-identical to the enumerator's
  grouping (tmp/grind/func_8003DE14/s36/fin/noparen.c, 0/173), so no non-natural
  grouping is committed.

## T6 naming-announces-intent
No new names are introduced by this session at all. `gm` names what it holds (the
green mask); no identifier in the body is pad/dummy/unused/spill/tmp/slack, and
every local's value is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: combine-foldable chain-extender (F1) (for the bound detour AND the s21
    LoadImage-argument detour - one family, two instances)
  SCOPE: "a LIVE store/computation routed through an algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted bytes - its only surviving effect is the extra `reg_n_refs` count flow.c records before the fold."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:51
  Extra prerequisite (the clause's own) VERIFIED for both instances: the fold
  emits zero bytes and materializes no address. build_insns is 173 for the body
  WITH the bound detour (score 0) and 173 for the identical body WITHOUT it
  (tmp/grind/func_8003DE14/s37/v2/g6.c, score 3); likewise 173 with and without
  the s21 extender (0/173 vs 7/173).
  Prerequisite 1 (lever-exhaustion): memory/grind/func_8003DE14/hypotheses.md
  s21-s37 - 37 sessions, ~206 rejected spellings banked under
  memory/grind/func_8003DE14/rejected/, including every operand order, every
  declaration order, for/while/do-while chassis rewrites, staged carriers at 26
  read sites, and the s37 CLASS kill (local-alloc.c:1669) proving no natural
  equal-refs spelling can seat the earlier load in $v0.
  Prerequisite 2 (named pass): local-alloc.c:1669-1684 qty_compare_1, with the
  dumped priorities above.
  Prerequisite 3 (annotation): present, quoted below.

  FAMILY: named-intermediate declaration order (for `gm`)
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written - **relaxed from "once-written, once-read" by owner ruling 2026-08-31 ([[ordinary-c-judge-decidable]]; evidence: the SOTN-master PSX `new_var_temp` class, docs/reference/sotn-construct-index.md:649 - NOTE the caveat carried in that rule's section Ruling 1: the index carries declaration lines only, so this is precedent for the existence of fresh RA-purposed locals in SOTN PSX master, not a line-for-line shape match; the owner ruled with that caveat presented): a fresh local holding a real, consumed value may be read any number of times.**"
  PRECEDENT: .claude/rules/no-new-park-categories.md:204
  PRONGS: (1) once-written - `gm` is written exactly once; (2) real value -
  `andi $v1,$v1,0x3E0` + `or $v0,$v0,$v1` are in the target's bytes
  (asm/funcs/func_8003DE14.s:132,137); (3) byte-neutral - 173 == 173, score 0;
  (4) fresh, not a borrow - gp/rp/sum and the g_src-in-place split measured 17-43
  (tmp/grind/func_8003DE14/s36/x); (5) destination not live-pre-initialized -
  `s32 gm;` is uninitialised until its single write; (6) dump-proven mechanism +
  lever exhaustion (hypotheses.md s29-s36) + FAKE annotation present.

ANNOTATION-CONFORMANCE:
  /* FAKE: the inner loop's bound is routed through the algebraically
   * equivalent detour `+ rect[2] - rect[2]`, which combine folds back to the
   * direct `rect[2] * rect[3]` with ZERO emitted bytes ... Mechanism:
   * local-alloc.c:1669-1684 `qty_compare_1` ... Lever-exhaustion:
   * memory/grind/func_8003DE14/hypotheses.md s24-s37 ... */
   (present in candidate.c on the latch statement; carries what + named GCC pass
   + lever-exhaustion pointer, plus the dumped qty rows)
  /* FAKE: j chain extender on the dst_buf argument (s21); mechanism: combine.c
   * folds the +j/-j pair away but flow.c's reg_n_refs for j is counted before it,
   * lifting j's allocno priority so the $t4/$t5 seat pair matches;
   * lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md s21-s30 */
   (present in candidate.c)
  /* FAKE: `gm` names the green channel's masked result so that g_src dies at the
   * mask instead of at the store; mechanism: global.c allocno priority
   * (prio = nrefs*40000/live_length, dumped via BB2_ALLOC_DEBUG) ...
   * lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md s29-s35 + s36
   * waves x */
   (present in candidate.c)

## s38b ADDENDUM (2026-09-11) - the necessity measurement layer-1 asked for

Layer-1 FAILed this body on 2026-09-11 05:42 on one ground only: "the s21
`((s32)dst_buf + j) - j` detour has NO necessity measurement on this s38
no-carrier chassis and the ledger's own nearest measurement (s37 u1) found it
inert". The body is UNCHANGED; what follows is that measurement.

MEASURED, one sweep, four bodies differing only in which FAKE construct is
present (tmp/grind/func_8003DE14/s38/v2/b1..b4.c, all 173 build insns,
rules_dropped 0):
  b1 = the committed body                                -> 0/173
  b2 = b1 with ONLY the s21 extender removed             -> 7/173
  b3 = b1 with ONLY `gm` removed                         -> 17/173
  b4 = both removed                                      -> 22/173
The s37 `u1` datum layer-1 relied on ("still 7 without it") was taken on the
h-CARRIER chassis, which this body does not use; that chassis no longer exists.

CLOSED FORM for the 7 (BB2_ALLOC_DEBUG dumps, not inference). b2's residual is a
pure $t4 <-> $t5 exchange between `j` and `complement`. global.c's
`allocno_compare` key is floor_log2(n_refs)*n_refs/live_length*10000:
  b2: j 11 refs/livelen 59 -> 5593 ; complement 11/54 -> 6111 -> complement takes $t4 (wrong)
  b1: j 15 refs/livelen 73 -> 6164 ; complement 11/54 -> 6111 -> j takes $t4 (target)
Artifacts: s38/qty_b2.log and s38/qty_b1.log, ord=15/16 lines. Margin 0.87%.

WHY NO ORDINARY-C FORM REACHES IT (56 bodies measured this session, banked under
rejected/s38b-*.c): `j`'s initialiser must be emitted before the inner loop's
guarding `blez`, because reorg.c:2963's backward delay-slot scan otherwise
reaches the `dst = dst_buf` init and hoists it out of the row-top block
(BB2_DBR_DEBUG trace s38/dbr_c2.log:2185-2191; that body scores 3 with the seats
CORRECT - it just moves the defect). With `j` initialised there, livelen(j) >
livelen(complement) for every spelling, so at equal refs `complement` always
wins; 12 refs is still short (3*12/59 = 6101 < 6111). Swept and failed: 23
declaration-order permutations (all inert), 8 declaration placements, 7
`complement` hoists/splits (23-29), 7 `complement` bookkeeping forms (7-11), 5
`duplicated-statement-into-arms` spellings of `j++` - the sanctioned family whose
own rule names a reg_n_refs lift as its effect - (13-25; the two that stay at 173
insns overshoot a floor_log2 step), and 6 in-latch `j` detours (3-12, so the two
detours cannot be merged into one construct).
