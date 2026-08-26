# SELF-VET — func_80061710

Diff under review: `src/text1b.c` — `INCLUDE_ASM("asm/funcs", func_80061710);`
replaced by the pure-C body in `memory/grind/func_80061710/candidate.c`.
Measured `sandbox func_80061710 --disable all` = **0** (46/46 insns, rules_dropped 0,
zero register pins, zero inline asm) this session with the edit in place in src/.

CONSTRUCTS: (1) `s32 *v1 = (s32 *)&D_800F116C;` local pointer alias to a global;
(2) `s32 val;` per-arm value merged into a shared trailing block; (3) `u8 *q;`
per-arm pointer merged into the same shared block; (4) `default: goto done;`
skipping that shared block; (5) `s32 *p;` walking pointer `D_800F1140 = *p++;`
for the three arg0[] loads; (6) `D_800A3464 = 0x10FF10;` inline constant store.

## T1 semantic purpose: per construct
- (1) alias `v1`: has a real semantic role — its VALUE is stored (`D_800A3468 =
  (s32)v1;`) and it is the object written (`*v1 = val;`). It is nonetheless
  byte-relevant beyond a simpler form: the direct-global spelling
  (`D_800A3468 = (s32)&D_800F116C; ... D_800F116C = val;`) is semantically
  identical and measures sandbox 5
  (rejected/v9e-no-pointer-alias-direct-global-floor5.c). Because a simpler form
  with identical behaviour exists, this construct is declared as a
  sanctioned-family (pointer-alias) FAKE construct below rather than defended as
  plain ordinary C. It is NOT dead, NOT write-only, and NOT an alias RENAME
  (`asm("Sym")`) — it is an ordinary C local whose address computation GCC emits.
- (2) `val`: semantically REQUIRED. The two switch arms select different
  constants (0x21000E / 0x21000F) that are consumed by ONE shared statement
  `*v1 = val;`. Deleting `val` is impossible without duplicating the shared
  block into both arms — i.e. it is a genuine merge value, not a holder. Its
  value is materialised in the target bytes (`lui/ori 0x21000E|F` then
  `sw $v1, 0($a0)`). Written once per arm, read once.
- (3) `q`: same shape as (2) — the two arms select `&D_800F115C + 2` / `+ 3`,
  consumed by the shared `*q = 0; D_800F1180 = (s32)q;`. Semantically required;
  its value is in the bytes (`sb $zero, 0($v0)`, `sw $v0, %lo(D_800F1180)($at)`).
- (4) `default: goto done;`: semantically REQUIRED. For `arg1` outside {0,1}
  neither `val` nor `q` is initialised, so the shared block must be skipped.
  A `break` would fall into it and read uninitialised locals — a real bug.
  This is exactly target's control flow (`j .L8006177C` past the shared block).
- (5) walking pointer `p`: ordinary C, the same three-store idiom the
  COMPLETED-C sibling `func_8006156C` uses verbatim (src/text1b.c:3316-3319).
  Real consumed value; GCC folds the increments into 0/4/8($s0) offsets.
- (6) inline constant store: the simplest possible spelling — no temp at all.
  It REMOVES the prior sessions' `mask` local rather than adding anything.

## T2 human-programmer: A human given only the spec (publish arg0 and a mode
word, then push three words and a constant to the display globals) writes
exactly this: a switch that selects a value+pointer pair, one shared block that
applies them, a default that skips it, and a walking pointer over the three
consecutive words. Nothing here reads as "why is this here?" except construct
(1), whose presence a reader would explain as "we keep a pointer to the buffer
we just published" — and (1) is declared as a FAKE-annotated sanctioned family
below rather than defended as free of smell. Note the form is a direct
transplant of the tail of a sibling function that is ALREADY COMPLETED-C in this
repo (func_8006156C, src/text1b.c:3296-3322) — it is literally the house style.

## T3 GCC-internals justification: The program logic IS the explanation for
constructs (2)(3)(4)(5)(6): each is required by, or is the simplest spelling of,
the function's semantics; none was chosen for a named GCC pass. Only construct
(1) carries a GCC-pass mechanism in its rationale (base-register allocation /
address-materialisation caching in local-alloc), and that is disclosed openly in
its mandatory `/* FAKE: ... */` annotation as the sanctioned-family rule requires
— not smuggled in as program logic.

## T4 permuter/search provenance: NONE of this came from a search. s4's native
permuter campaigns (~4574 + ~19k iterations) never found this basin — every
permuter score-0 was the refused constant-staging coercion, and campaign 2
concluded no other score-0 existed. This form was derived by hand this session
from a HUMAN-READABLE in-repo source: the COMPLETED-C sibling func_8006156C,
whose target tail is byte-shape-identical to 710's. It survives the detectors
because there is nothing for them to catch, not because of a spelling.

## T5 family check: Walked the forbidden-family catalog construct by construct.
- register-asm pins / hardcoded-$N asm / regfix / asmfix / scheduling barriers /
  volatile of any spelling / DImode chains / dead-goto pads / `if (1)` wrapping /
  `do{}while(0)` / unused arrays / dead param assigns / opaque `s32 one = 1;` /
  `asm("sym")` renames / redundant width casts / .ld reorders — NONE PRESENT.
  The diff contains no asm, no volatile, no cast other than the `(s32)`
  pointer-to-word casts the target's stores literally require, and touches no
  config file.
- **F1 constant-staging through a REUSED LIVE local** (REFUSED; survey WEAK,
  docs/grind/borderline.md 2026-08-18) — the construct that blocked s1-s4 — is
  ABSENT. The tail (`D_800A3464 = 0x10FF10;`) has no temp whatsoever, and `val`
  is (a) fresh per arm, (b) never reused for a second unrelated value, (c) dead
  before `func_80060A68()` is even called, so it cannot be a copy source for the
  tail's constant. Verified by construction: the s5 winner's tail is character-
  for-character the COMPLETED-C sibling's tail.
- `default: goto done;` is a real exit-path skip, not a dead-goto label pad.
- The only construct matching a sanctioned family is (1), claimed below.

## T6 naming-announces-intent: No `pad`, `dummy`, `unused`, `spill`, `slack`,
`tail`, `_buf`, `_frame_pad` or similar. Names are `v1`, `p`, `q`, `val` —
`v1` follows the existing sibling func_8006156C's identical declaration
(src/text1b.c:3297); `p`/`q`/`val` are generic and every one of them is read.
No construct's only uses are discards / address-of / declaration.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: C-level pointer alias to a global (pointer-alias FAKE exception)
  SCOPE: "a local pointer that provides a second C handle to a global — where using the global directly would be semantically identical — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: src/text1b.c:3297
  (Prereq 1 lever-exhaustion: the s1-s4 ledger — structural axis dead (s2/s3),
   native permuter dead (s4) — plus the s5 direct-global measurement
   memory/grind/func_80061710/rejected/v9e-no-pointer-alias-direct-global-floor5.c
   = sandbox 5. Prereq 2 GCC-pass named: base-register allocation /
   address-materialisation caching in local-alloc. Prereq 3 annotation: present,
   quoted below. Prereq 4: this vet plus the pipeline layer-1/layer-2 reviewers.
   Rule file: .claude/rules/pointer-alias-fake-exception.md. The cited precedent
   src/text1b.c:3297 is the identical declaration inside the COMPLETED-C sibling
   func_8006156C; the rule file additionally records SOTN-master precedent for
   the family, e.g. `src/dra/cd.c:539` `new_var3 = &g_Cd;`.)

ANNOTATION-CONFORMANCE:
  /* FAKE: local pointer alias to D_800F116C, mechanism: base-register
   * allocation / address-materialization caching in local-alloc (the alias
   * gives GCC one pseudo holding &D_800F116C, kept live in $a0 across the
   * switch instead of being re-materialized per use), lever-exhaustion:
   * memory/grind/func_80061710/hypotheses.md (s1-s4: structural, mask-position
   * sweep, native permuter all measured dead) + s5 direct-global form
   * (tmp/grind/func_80061710/s5/v9e_noalias.c) measured sandbox 5. */
  Confirmed: carries WHAT (local pointer alias to D_800F116C), MECHANISM
  (base-register allocation / address-materialization caching in local-alloc),
  and LEVER-EXHAUSTION (ledger pointer plus the measured direct-global
  alternative). It is the only FAKE construct in the diff.
