# SELF-VET - func_8003DE14

STATUS: PRE-RULING. This session returned `ruling-request`, NOT `candidate-ready`.
Two of the three constructs below are vetted to submission standard; the third
(`h`) has no covering family and is the subject of the ruling question. Do NOT
submit this body as `candidate-ready` until the `h` block below carries a real
SCOPE + PRECEDENT (or `h` is replaced by a form that does).

CONSTRUCTS: `gm` fresh named intermediate for the green mask (`gm = g_src & 0x3E0;`
then read once in the store); `h` fresh local staging two real values at two sites
(`h = target_color; *dst++ = h;` and `while (j < rect[2] * (h = rect[3]))`);
the s21 chain extender `((s32)dst_buf + j) - j` on the LoadImage argument.
(The natural-order OR chain `(pixel & 0x8000) | r_src | gm | (px & 0x7C00)` is
ordinary C and is NOT counted as a construct - see T5 below.)

## T1 semantic purpose
- `gm`: names a real sub-expression (the green channel's 0x3E0 mask) that the
  target itself computes and stores (`andi $v1,$v1,0x3E0` / `or $v0,$v0,$v1`,
  asm/funcs/func_8003DE14.s:132,137). Removing the name does not change the value
  but does change the bytes (17/173), so its effect is codegen, not semantics -
  which is why it is FAKE-annotated rather than presented as ordinary C.
- `h`: both writes stage a real value consumed on the spot (the stored colour in
  the fast arm; the multiplicand in the latch test). Zero dead code; the values
  are semantically required, the sharing of the carrier is not.
- j chain extender: folds to zero bytes; no semantic purpose. FAKE by construction.
- OR chain: full semantic purpose - it is the pixel the function stores.

## T2 human-programmer
- `gm`: yes, a reader sees a named mask. Mildly redundant, not alien.
- `h`: a reader would ask why one local carries two unrelated values, and in
  particular why the latch reads `rect[3]` through an assignment. This is the
  construct that fails T2, and it is why this session asks rather than submits.
- j chain extender: fails T2 outright (a sanctioned FAKE, not ordinary C).
- OR chain: yes - sign, red, green, blue is the function's own channel order.

## T3 GCC-internals justification
All three constructs are justified by a named, dump-proven pass: global.c's
allocno priority ordering (prio = nrefs*40000/live_length; rows dumped via
BB2_ALLOC_DEBUG to tmp/grind/func_8003DE14/s36/alloc.log) for `gm`;
local-alloc.c:470-476 (pseudos with reg_basic_block < 0 are skipped, so global.c
seats them and the first scheduling pass orders the two `lh`s) for `h`; flow.c's
`reg_n_refs` counted before combine.c folds the detour, for the j extender.
Per the checklist this is a cheat SIGNAL, which is exactly why each one needs a
sanctioned family and a FAKE annotation, and why the unclassified one goes to a
ruling instead of being self-approved.

## T4 permuter/search provenance
`gm` was found by exhaustive enumeration (tools/spelling_enum.py, 104 spellings,
18 zeros) - a PROPOSAL. It is kept because the ALLOCDBG rows explain it
mechanistically (green 18/18 -> 18/16, red 24/21 -> 24/22, satisfying the
inequality s35 derived in closed form BEFORE this session ran) and because every
alternative carrier was measured and killed. The OR ORDER kept is the natural
channel order and measures byte-identical to the enumerator's parenthesised form,
so no enumeration-derived grouping is committed. `h` and the j extender predate
this session (s32, s21) and were mechanism-derived there.

## T5 family check
- `gm` - named-intermediate (claim below). All six prongs measured, not asserted.
- OR chain - or-tree-shape-shift section "What IS allowed": "A worker may freely
  choose ANY of the natural orderings (1) listed above." sign|red|green|blue is
  the natural channel order and matches the target's own emission order; the
  paren-free spelling measures byte-identical to the enumerator's grouping
  (tmp/grind/func_8003DE14/s36/fin/noparen.c, 0/173), so no non-natural grouping
  is committed and the 2026-08-20 carve-out is not invoked.
- j chain extender - combine-foldable chain-extender (F1), claim below.
- `h` - NO FAMILY COVERS IT. Excluded from staged-value-reused-variable by bound 2
  (invented carrier) and from named-intermediate by multi-write. The ruling question.

## T6 naming-announces-intent
Neither `gm` nor `h` is named pad/dummy/unused/spill/tmp; each names what it holds
(green mask; the staged colour/halfword). Every construct's value is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: named-intermediate declaration order (for `gm`)
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written - **relaxed from "once-written, once-read" by owner ruling 2026-08-31 ([[ordinary-c-judge-decidable]]; evidence: the SOTN-master PSX `new_var_temp` class, docs/reference/sotn-construct-index.md:649 - NOTE the caveat carried in that rule's section Ruling 1: the index carries declaration lines only, so this is precedent for the existence of fresh RA-purposed locals in SOTN PSX master, not a line-for-line shape match; the owner ruled with that caveat presented): a fresh local holding a real, consumed value may be read any number of times.**"
  PRECEDENT: .claude/rules/no-new-park-categories.md:204
  PRONGS (all measured this session): (1) once-written - `gm` is written exactly
  once; (2) real value - `andi $v1,$v1,0x3E0` + `or $v0,$v0,$v1` are in the
  target's bytes (asm/funcs/func_8003DE14.s:132,137); (3) byte-neutral -
  build_insns 173 == target_insns 173, score 0; (4) fresh, not a borrow - gp/rp/sum
  and g_src-in-place measured 17-43 (tmp/grind/func_8003DE14/s36/x); (5) destination
  not live-pre-initialized - `s32 gm;` is uninitialised until its single write;
  (6) dump-proven mechanism (global.c allocno priority, ALLOCDBG rows in
  tmp/grind/func_8003DE14/s36/alloc.log), lever exhaustion
  (memory/grind/func_8003DE14/hypotheses.md s29-s36), FAKE annotation present.

  FAMILY: combine-foldable chain-extender (F1) (for `((s32)dst_buf + j) - j`)
  SCOPE: "a LIVE store/computation routed through an algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted bytes - its only surviving effect is the extra `reg_n_refs` count flow.c records before the fold."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:51
  Extra prerequisite verified: the fold emits zero bytes - build_insns is 173 both
  with and without the extender (0/173 vs 7/173, same insn count, no new address
  materialization).

  FAMILY: (NONE - UNRESOLVED) for `h`
  The construct: one FRESH `s32 h` written at two sites, each write staging a real
  value consumed immediately (`h = target_color; *dst++ = h;` in the fast arm;
  `while (j < rect[2] * (h = rect[3]))` in the latch, where the assignment's value
  is the multiplicand). Zero dead code, zero unread writes.
  Why no family: staged-value-reused-variable bound 2 - "Inventing a new variable
  just to have something to borrow is NOT this rule"; named-intermediate -
  "Multi-WRITE carriers remain NOT this entry". No SCOPE sentence can honestly be
  quoted for it, so no claim is made here.

ANNOTATION-CONFORMANCE:
  /* FAKE: `gm` names the green channel's masked result so that g_src dies at the
   mask instead of at the store; mechanism: global.c allocno priority
   (prio = nrefs*40000/live_length, dumped via BB2_ALLOC_DEBUG); lever-exhaustion:
   memory/grind/func_8003DE14/hypotheses.md s29-s35 + s36 waves x */
   (present in candidate.c)
  /* FAKE: `h` is a single staging local used twice ...; mechanism: local-alloc.c
   :470-476 skips any pseudo with reg_basic_block < 0 ...; lever-exhaustion:
   memory/grind/func_8003DE14/hypotheses.md s24-s31 + s32 waves a-h */
   (present in candidate.c)
  /* FAKE: j chain extender on the dst_buf argument (s21); mechanism: combine.c
   folds the +j/-j pair away but flow.c reg_n_refs for j is counted before it;
   lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md s21-s30 */
   (present in candidate.c)
