# SELF-VET — func_80045878  (s13, 2026-08-31, structural modality)

STATUS: this vet accompanies a **ruling-request**, NOT a candidate-ready
submission.  The form it vets (`memory/grind/func_80045878/candidate.c`)
measures `sandbox func_80045878 --disable all` = **score 0, build_insns 108 ==
target_insns 108, rules_dropped 0** on today's tree, but one of its three
constructs is deliberately NOT claimed under a sanctioned family, and the form
as a whole contradicts the letter of the Judge's standing 2026-08-30 constraint
for this function.  s13 is asking, not submitting.

CONSTRUCTS: (1) `s16 *p = 0;` — dead constant initialisation of a fresh
pointer local whose stored value is never read; (2) `p = s1;` — fresh pointer
local aliasing the existing local `s1`, then used as the base of the seven
tail stores; (3) `s32 t; t = a0 + 3; p[11] = t;` — fresh, once-written,
once-read named intermediate.

## T1 semantic purpose
(1) NONE.  The stored 0 is never read; deleting the initialiser leaves the
function's behaviour byte-identical at the C level.  Its whole effect is on
codegen.  FAILS T1 on its own terms — which is why it is claimed under a
sanctioned family (dead-store) rather than defended as ordinary C.
(2) NONE beyond `s1`.  Writing the seven tail stores through `p` is
semantically identical to writing them through `s1`.  A reader would ask "why
is this here?".  FAILS T1.
(3) NONE beyond the inline expression.  `p[11] = a0 + 3;` is semantically
identical.  FAILS T1; claimed under the named-intermediate family.

## T2 human-programmer
(1) A human might plausibly write `s16 *p = 0;` as a defensive NULL
initialisation — but not in a function where `p` is unconditionally assigned
before its first use.  Treat T2 as FAILED and rely on the family claim.
(2) A human writing a long run of stores through one object DOES commonly
introduce a short local alias, and the ORIGINAL author demonstrably had some
construct here: target itself emits `addu $v0, $s1, $zero` at the top of the
tail and addresses all seven stores through `$v0`.  So this construct is
present in the original's bytes.  It is nonetheless a redundant alias at the
C level; T2 is charitable at best, not a pass.
(3) Naming `a0 + 3` before storing it is ordinary style, but it is here for
codegen reasons.  T2 FAILED; family claim carries it.

## T3 GCC-internals justification
YES for all three, explicitly.  The mechanism is:
`reg_scan` (regclass.c) fixes `regno_first_uid[p]` over the RTL as it stands
BEFORE cse; cse.c:836-864 `make_regs_eqv` therefore finds
`uid_cuid[regno_first_uid[p]] < cse_basic_block_start` at the tail block, makes
`p` (not `s1`) the qty's canonical register, and the base copy insn survives
instead of being propagated away; flow.c `life_analysis` then DELETES the dead
store, so `reg_basic_block[p]` stays block-local (block 13) and `p` is
allocated by local_alloc rather than global_alloc; local-alloc.c:1641
`qty_compare` ranks the block-13 quantities by
`floor_log2(n_refs) * n_refs * size / (death - birth)` — `p` (7 refs / 10
insns) outranks the tail scratches (2 refs each) and, since mips.h defines no
REG_ALLOC_ORDER, `find_free_reg` scans ascending and gives `p` $v0 and the
scratches $v1.  Construct (3) exists because without it cse builds a HImode
common-subexpression temp for the three `a0` half-word stores (4 refs / 5
insns, priority 16000) which OUTRANKS `p` (14000) and steals $v0 — measured:
109 insns / score 10.  This is a T3 "cheat signal" by the letter of the
checklist; it is disclosed in full rather than hidden, and it is exactly the
`mechanism:` clause the FAKE template demands.  Dump evidence, not inference:
`tmp/grind/func_80045878/dumps/text1a_c.lreg` shows `Register 79 used 7 times
across 10 insns in block 13` for the dead-init form (block-local) versus
`Register 76 used 10 times across 12 insns` (no block tag = multi-block) for
the previous carrier form.

## T4 permuter/search provenance
NO permuter, no auto-search.  Every form this session was hand-written from a
compiler-source reading of cse.c/flow.c/local-alloc.c and measured with
`sandbox --disable all`.  The construct was PREDICTED by the mechanism and then
confirmed, not found by a search that detectors happened to miss.

## T5 family check
(1) Matches `.claude/rules/dead-store-fake-exception.md` exactly (see
SANCTIONED-FAMILY-CLAIMS).  Not a register pin, not asm, not a scheduling
barrier, not volatile coercion, not a frame-coercion array.
(3) Matches the named-intermediate family with all six 2026-08-17 prongs.
(2) **Does NOT match any sanctioned family.**  It is not the pointer-alias
family (`.claude/rules/pointer-alias-fake-exception.md` scopes a pointer local
to a GLOBAL; `s1` is a local).  It is not the named-intermediate family (prong
(2) of the 2026-08-17 clarification routes "pure no-op copies" to the
dead-store family, and prong (3) requires byte-neutrality via the compiler
folding the copy — here the copy is deliberately NOT folded).  It is not
variable reuse (`p` is invented, and bound 2 of
`.claude/rules/staged-value-reused-variable.md` excludes inventing a local to
borrow).  This is why the session's outcome is `ruling-request`.
Additionally, taken together, `p` is a fresh local written TWICE (dead 0, then
the real base), which is the literal shape the Judge's 2026-08-30 constraint
for this function bans, and the form necessarily drops the `v0` tail-base
reuse that the same constraint told the next session to keep.

## T6 naming-announces-intent
`p` and `t` are ordinary short local names in the style of the surrounding
decompiled file (`v0`, `s1`, `s3`, `s0`).  Neither is `pad`, `dummy`, `unused`,
`spill`, `slack` or similar.  Both have real uses: `p` is dereferenced seven
times, `t` is read once.  PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead store to a local (construct 1)
  SCOPE: "dead store to a local: `dest = val1;` where `dest` is never read"
  PRECEDENT: `.claude/rules/dead-store-fake-exception.md:35`

  FAMILY: named-intermediate declaration order (construct 3)
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written, once-read — multi-write carriers are NOT this entry (the `y1` FAIL, decisions.md:1833, stands); (2) real value — the intermediate holds a computation that appears in the target's own bytes and only relocates where the value is named; pure no-op copies stay with the dead-store family and its prerequisites; (3) byte-neutral — `build_insns == target_insns`, the compiler folds the copy; (4) fresh local, not a borrow — [[staged-value-reused-variable]] keeps its own bounds; (5) destination not live-pre-initialized (the `x/tx` FAIL, decisions.md:4251, stands); (6) standard prerequisites: dump-proven named mechanism, documented lever exhaustion, `/* FAKE: ... */` annotation, layer-1 + layer-2 review."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:208`

  FAMILY: none claimed for construct 2 (`p = s1;`) — see T5.

ANNOTATION-CONFORMANCE:
  /* FAKE: dead store to the local `p`, mechanism: regclass.c reg_scan fixes
     regno_first_uid before cse.c:836-864 make_regs_eqv runs, so `p` stays
     canonical and the tail base copy survives, while flow.c life_analysis
     deletes this store and leaves reg_basic_block[p] block-local so
     local-alloc.c:1641 qty_compare seats it in $v0 ahead of the tail
     scratches, lever-exhaustion: memory/grind/func_80045878/hypotheses.md
     (s1-s13, 46 banked rejected forms) */
  /* FAKE: named intermediate for a real value that target materialises as
     `addiu $v1, $s2, 0x3`, mechanism: keeps the a0+3 truncation out of the
     HImode cse temp that otherwise outranks `p` in local-alloc.c:1641
     qty_compare (measured: without it the base loses $v0, 109 insns /
     score 10), lever-exhaustion: memory/grind/func_80045878/hypotheses.md */
  Both lines carry what + mechanism + lever-exhaustion and are present verbatim
  in memory/grind/func_80045878/candidate.c.  Construct 2 carries no annotation
  because no family is claimed for it.
