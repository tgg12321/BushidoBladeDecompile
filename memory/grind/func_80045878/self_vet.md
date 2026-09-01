# SELF-VET — func_80045878  (s14, 2026-08-31, synthesis modality)

STATUS: this vet accompanies a **candidate-ready** submission, filed under the
owner directive attached to this function's queue item (owner ruling 2026-08-31,
`.claude/rules/ordinary-c-judge-decidable.md`, commit 73bee8f8): *"returned to
active — fresh layer-1 + default-FAIL Judge adjudication of the s13b candidate
against the amended rules (named-intermediate once-written relaxation +
store-level dead-store deadness). Nothing pre-decided; all prerequisites
verified per instance."*  s14 re-measured the s13b form on today's chassis,
re-ran the pass dumps against THIS exact form, and is submitting it for that
adjudication.  It is not asking a routing question — under Ruling 3 of the same
rule, non-membership is a clean Judge FAIL, never a packet.

MEASURED THIS SESSION (s14), with the form in place in `src/text1a_c.c`:
  - `sandbox func_80045878 --disable all` → **score 0, build_insns 108 ==
    target_insns 108, scorable true, rules_dropped 0, cheat_asm_stripped 66**
    (the 66 stripped are OTHER functions' cheat-asm in the same TU, not this
    function's — this function's body contains no asm of any kind).
  - `verify-oracle` → **`"ok": true, "build_matches": true`** — the full
    clean-driver build+link SHA1 still equals
    `62efab4f73f992798c43e8c730aa43baa10bb4fa` with the C body replacing the
    `INCLUDE_ASM` line.  Whole-EXE proof, not an isolated score.
  - `pwsh tools/grinder/dump.ps1 func_80045878` re-run against THIS form;
    `tmp/grind/func_80045878/dumps/text1a_c.lreg` line 15452 ff. shows
    `Register 79 used 7 times across 9 insns in block 13; GR_REGS or none;
    pointer.` (= `p`, BLOCK-LOCAL ⇒ local_alloc, not global_alloc) alongside
    `Register 80 used 2 times across 2 insns in block 13` and `Register 102
    used 2 times across 4 insns in block 13` (= the two tail scratches).  The
    7-refs-over-9-insns vs 2-refs quantities are exactly the `qty_compare`
    ordering the mechanism below predicts.

CONSTRUCTS: (1) `s0 = (s32) p;` — a dead store to the existing local `s0`, whose
stored value is never read on any path; (2) `s16 *p;` … `p = s1;` — a FRESH
local, written EXACTLY ONCE, read 7 times as the base of the tail stores;
(3) `s32 t; t = a0 + 3; p[11] = t;` — a fresh, once-written, once-read named
intermediate.

## T1 semantic purpose
(1) NONE. The stored value is never read: on the then-path `s0` is not read at
all, and on the else-path `s0` is re-assigned (`s0 = (s32) func_800455AC(s3);`)
before every read.  Deleting the statement leaves the function's observable
behaviour identical.  Explicitly claimed under the dead-store family rather
than defended as ordinary C.
(2) NONE beyond `s1`.  Writing the seven tail stores through `p` is
semantically identical to writing them through `s1`.  Claimed under the
named-intermediate family (as amended 2026-08-31).
(3) NONE beyond the inline expression — `p[11] = a0 + 3;` is semantically
identical.  Claimed under the named-intermediate family.
All three FAIL T1 on their own terms; that is why each carries a family claim,
a `/* FAKE */` annotation and the exhaustion ledger, per policy.

## T2 human-programmer
(1) No — a reader would ask why `s0` is written here.  T2 FAILED; the
dead-store family carries it.
(2) Partly.  A human writing a long run of stores through one object commonly
introduces a short local for the base, and the ORIGINAL author demonstrably had
such a base here: the target's own bytes contain `addu $v0, $s1, $zero` at the
top of the tail and address all seven stores through `$v0`.  The construct is
therefore present in the original binary, not invented to fake it.  It is
nonetheless redundant at the C level; treat T2 as charitable, not a pass.
(3) Naming `a0 + 3` before storing it is ordinary style; it is here for codegen
reasons.  T2 FAILED; family claim carries it.

## T3 GCC-internals justification
YES for all three, disclosed in full — the `mechanism:` clause the FAKE
template demands, dump-proven rather than inferred:
`reg_scan` (regclass.c) fixes `regno_first_uid[p]` over the RTL as it stands
BEFORE cse runs, so the mention of `p` in construct (1) precedes the tail block.
`cse.c:836-864 make_regs_eqv` therefore finds
`uid_cuid[regno_first_uid[p]] < cse_basic_block_start` at the tail, makes `p`
(not `s1`) the qty's canonical register, and the base copy insn SURVIVES
instead of being propagated away (without any early mention cse folds it: 107
insns / score 9).  `flow.c life_analysis` then DELETES the dead store of
construct (1), so `reg_basic_block[p]` stays block-local (block 13 in the .lreg
above) and `p` is allocated by local_alloc, which runs before global_alloc.
`local-alloc.c:1641 qty_compare` ranks block-13 quantities by
`floor_log2(n_refs) * n_refs * size / (death - birth)`; `p` (7 refs / 9 insns)
outranks the two tail scratches (2 refs each), and since mips.h defines no
REG_ALLOC_ORDER, `find_free_reg` scans ascending and gives `p` `$v0` and the
scratches `$v1` — exactly the target's seating.  Construct (3) exists because
without it cse builds a HImode common-subexpression temp for the three `a0`
half-word stores which outranks `p` in the same `qty_compare` and steals `$v0`
(measured: 109 insns / score 10).  Construct (1)'s value must be one that
creates no cse equivalence with a live pointer: carrying a REAL pointer there
(`p = (s16 *) v0;`) perturbs the tail (measured s13b: 109 insns / score 13,
banked as `rejected/s13b-deadinit-real-v0-value-109-score13.c`).
This is a T3 "cheat signal" by the letter of the checklist.  It is disclosed,
not hidden, and under ruling 3 of `.claude/rules/ordinary-c-judge-decidable.md`
("scheduling-motivated respelling is not a FAIL ground") the governing question
is family membership + prerequisites, which is what the next section answers.

## T4 permuter/search provenance
NO permuter, no auto-search, no detector-evasion.  Every form in this
function's ledger was hand-written from a source reading of cse.c / flow.c /
local-alloc.c and measured with `sandbox --disable all`; the s13/s13b forms
were PREDICTED by the mechanism above and then confirmed.  47 disproven forms
are banked in `memory/grind/func_80045878/rejected/`.

## T5 family check
(1) `s0 = (s32) p;` — dead-store family.  Under owner Ruling 2 (2026-08-31)
deadness is STORE-level, so a store whose value is never read is in scope even
though `s0` is later re-assigned and read.  That is exactly this shape.
(2) `p` — named-intermediate family as amended by owner Ruling 1 (2026-08-31),
prong-by-prong, stated honestly including the one clause that does not read
cleanly:
   - (1) once-written — **SATISFIED literally**: `p = s1;` is the ONLY write to
     `p` in the function.  Multi-write carriers remain banned (the `y1` FAIL,
     decisions.md:1833, and this function's own 2026-08-30 `c` FAIL); this form
     was constructed specifically to satisfy that clause, and the amended prong
     (1) allows the 7 reads.
   - (2) real value — SATISFIED: the value `p` holds is the object pointer, and
     the copy that materialises it appears in the TARGET's own bytes as
     `addu $v0, $s1, $zero`.  The prong's exclusion is "pure no-op copies"; this
     copy is not a no-op — it is an instruction the original compiler emitted
     and that the match requires.
   - (3) byte-neutral — SATISFIED in the prong's operative test:
     `build_insns == target_insns` (108 == 108), whole-EXE SHA1 unchanged.
     **DISCLOSURE:** the prong's trailing clause reads "the compiler folds the
     copy", and here the compiler does NOT fold it — the copy survives and is
     byte-identical to the target's.  I read the clause as a statement of how
     byte-neutrality is normally achieved rather than an independent
     requirement (a folded copy and a copy the target also emits are equally
     byte-neutral), but the Judge should rule on that reading explicitly; I am
     not claiming the clause is literally met.
   - (4) fresh local, not a borrow — SATISFIED: `p` is newly declared, borrows
     no existing variable, so `.claude/rules/staged-value-reused-variable.md`
     bound 2 does not apply.
   - (5) destination not live-pre-initialized — SATISFIED: `p` has no
     initialiser and no prior store (the `x/tx` FAIL, decisions.md:4251, was a
     live-pre-initialized destination; this is not).
   - (6) standard prerequisites — dump-proven named mechanism (above, .lreg
     re-run this session), documented lever exhaustion (hypotheses.md s1–s14,
     47 banked rejected forms), `/* FAKE: ... */` annotation present, layer-1 +
     layer-2 review is what this submission requests.
   **DISCLOSURE 2:** construct (1) reads `p` before `p` is assigned.  The read
   feeds only a store that GCC deletes, so no indeterminate value reaches any
   observable computation and no byte in the output depends on it — but it IS
   an indeterminate read at the C level and the Judge should weigh it.  The
   alternative spellings that avoid it were measured and are worse under the
   rules, not better: `s16 *p = 0;` (alt_deadzero_s13.c, score 0) gives `p` a
   defined value but makes `p` a TWO-write local, which prong (1) bans and
   which this function's own 2026-08-30 FAIL covers; `s16 *q; q = p;`
   (alt_singlewrite_freshq_s13b.c, score 0) has the identical indeterminate
   read plus one more no-semantic-purpose local, so the simplest-known-form
   test (ruling 3 item 4) selects the form submitted here.
(3) `t` — named-intermediate family, all six prongs met on the ORIGINAL
(pre-relaxation) reading: once-written, once-read, real value that the target
materialises as `addiu $v1, $s2, 0x3`, byte-neutral (108 == 108), fresh, not
live-pre-initialized, annotated.
NONE of the three is a register pin, hardcoded-`$N` asm, scheduling barrier,
volatile coercion, alias rename, frame-coercion array, `(void)` discard,
dead-goto, `if (1)` wrap, or any other entry in the forbidden-family catalog.
The function body contains zero `__asm__` of any kind.

## T6 naming-announces-intent
`p` and `t` are ordinary short local names in the style of the surrounding
decompiled file (`v0`, `s1`, `s3`, `s0`).  Neither is `pad`, `dummy`, `unused`,
`spill`, `slack`, `_buf`, `tail` or similar.  Both have real uses: `p` is
dereferenced seven times, `t` is read once.  `s0` is a pre-existing local with
its own real uses.  PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: dead store to a local (construct 1)
  SCOPE: "dead store to a local: `dest = val1;` where the STORED VALUE is never read — **deadness is STORE-level, not variable-level (owner ruling 2026-08-31, [[ordinary-c-judge-decidable]] Ruling 2):** the store is in scope even if `dest` is later re-assigned and read (defensive-init shape: `x = a; ... x = b;` with every read reached only by the second store)."
  PRECEDENT: `.claude/rules/dead-store-fake-exception.md:35`
  PRECEDENT: `.claude/rules/ordinary-c-judge-decidable.md:91`

  FAMILY: named-intermediate declaration order, as amended 2026-08-31 (constructs 2 and 3)
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written — **relaxed from \"once-written, once-read\" by owner ruling 2026-08-31 ([[ordinary-c-judge-decidable]]; evidence: the SOTN-master PSX `new_var_temp` class, docs/reference/sotn-construct-index.md:649 ...): a fresh local holding a real, consumed value may be read any number of times.**"
  PRECEDENT: `.claude/rules/no-new-park-categories.md:211`
  PRECEDENT: `.claude/rules/ordinary-c-judge-decidable.md:64`
  PRECEDENT: `docs/reference/sotn-construct-index.md:649`

ANNOTATION-CONFORMANCE:
  /* FAKE: dead store to the existing local `s0` (its stored value is never
     read -- s0 is not read on the then-path and is re-assigned in the else arm
     before every use), mechanism: regclass.c reg_scan fixes regno_first_uid[p]
     before cse.c:836-864 make_regs_eqv runs, so `p` stays canonical and the
     tail base copy survives, while flow.c life_analysis deletes this statement
     and leaves reg_basic_block[p] block-local so local-alloc.c:1641
     qty_compare seats p in $v0 ahead of the tail scratches, lever-exhaustion:
     memory/grind/func_80045878/hypotheses.md (s1-s14) + 47 banked rejected
     forms in memory/grind/func_80045878/rejected/ */
  /* FAKE: fresh once-written named intermediate for the tail base pointer,
     the value target materialises as `addu $v0, $s1, $zero`, mechanism:
     cse.c:836-864 make_regs_eqv keeps `p` canonical so the base copy survives
     and local-alloc.c:1641 qty_compare seats its 7-ref quantity in $v0,
     lever-exhaustion: memory/grind/func_80045878/hypotheses.md (s1-s14) */
  /* FAKE: named intermediate for a real value that target materialises as
     `addiu $v1, $s2, 0x3`, mechanism: keeps the a0+3 truncation out of the
     HImode cse temp that otherwise outranks `p` in local-alloc.c:1641
     qty_compare (measured: without it the base loses $v0, 109 insns /
     score 10), lever-exhaustion: memory/grind/func_80045878/hypotheses.md */
  All three lines carry what + mechanism + lever-exhaustion and are present
  verbatim on their statements in `src/text1a_c.c` and in
  `memory/grind/func_80045878/candidate.c`.
