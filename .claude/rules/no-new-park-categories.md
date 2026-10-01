---
name: no-new-park-categories
paths: ["src/*.c", "engine/queue.py", "engine/cheats.py"]
description: "The anti-cheat policy and the single authority for the FROZEN SOTN-accepted construct-family list: no new cheat-tolerant categories, cheats by any spelling are cheats, auto-search finds are proposals, and the exact prerequisites of every sanctioned family."
metadata:
  type: rules
---

# No cheat-tolerant categories; the frozen SOTN family list

Owner (2026-06-01): *"We won't be sanctioning any kind of new policy or methodology that is
akin to a cheat ... If it's a cheat, it will not be accepted. Full stop."* Bar: SOTN community
standard: pure C, or canonical-body asm only for code that was originally hand-written asm.

## Forbidden

- **New park / "infrastructure" categories** that retire functions with a cheat or a gap:
  register-rotation walls, cross-jump merge walls, prologue save-order, RA plateaus, anything
  labeled "X infrastructure". These are pure-C problems not yet solved ([[no-compiler-divergence]]).
- **Build-time assembly rewriting in ANY form** (the retired regfix/asmfix class): no
  rule/config files transforming compiler output, no new stages or passes between cc1 and the
  linker, no Makefile/engine edits altering bytes per function, no prebuilt `.o` or asm
  substitution for a function claimed as C. Bytes come from compiling the committed C or an
  authorized canonical-asm body. Automatic FAIL.
- **Speculative rodata/link reorders** to force a match. Evidence-based TU re-attribution
  (function↔data adjacency, single-owner xrefs, byte signatures, resolved siblings) is
  legitimate SOTN workflow.

Dispositions for a stuck function: more search; the canonical-asm grant path if hand-coded
signals support it ([[judge-sole-gate]]); rotation ([[rotation-not-foreclosure]]). There is no
"new carve-out", and a proposed frozen-list extension is FAIL(CONSTRUCT) + a borderline entry.

## Cheats by any spelling

The detectors (`engine/volatile_cheats.py`, `engine/inlineasm.py`) catch literal forms (unused
arrays, `(void)&local`, dead param self-assigns, dead conditional stores, pins, hardcoded-`$N`
asm, volatile coercion casts, `asm("Sym")` alias renames, macro-hidden asm, general-purpose
opcode asm). **They are a backstop, not the standard; the standard is intent.** Signals of a
cheat: no semantic purpose (a programmer would not write it from the behaviour; names like
`pad`, `buf`, `dummy`, `spill`, `slack`, `_unused`, `_tmp`); dead in the output while changing
decisions upstream of DCE; "necessary" only because removing it raises the score; justified
only by GCC internals ("defeats CSE", "changes allocno priority"). Outside the frozen list
below (and the [[ordinary-c-judge-decidable]] rulings), such a construct is refused.

## Auto-search output is a PROPOSAL

Permuter, PERM_* macros, sweepers and enumerators cannot judge cheats. Vet every closing form
before surfacing it: (1) any catalog construct, directly or by analogy? (2) any code with no
semantic purpose? (3) would a programmer write it from a specification? (4) does its
justification cite GCC internals instead of program logic? Any yes ⇒ reject it yourself, bank
it as a rejected form, keep searching. Layer-2 is still mandatory.

## The FROZEN SOTN-accepted family list (owner-only to extend)

Each entry's prerequisites are mandatory. "Standard prerequisites" = documented lever
exhaustion, a named GCC-pass mechanism, the stated annotation, layer-1 + layer-2 review.
Read the linked rule before using or judging an entry. Do not generalize from one entry to a
broader category.

**2026-06-02 (SOTN borderline research):**
- **Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]).
- **Opaque arithmetic variables** ([[loop-rotation-two-shift]]): `s32 one = 1;` to prevent a
  bit-test transform; never as a dummy subscript or pointer offset (Q22,
  [[named-local-fake-exception]]).
- **Sub-word param reads** ([[narrow-stack-param-subword-offset]]): `*(u16 *)&local`.
- **Mixed exit forms** ([[cross-jump-store-tail-merge]]): mix `goto endK` with inline `return`.
- **Duplicate read into branch arms** ([[split-read-defeats-hoist]]).
- **Named-intermediate declaration** ([[narrow-byte-args-packed-call]] hi/lo): a fresh named
  local for a sub-expression, whatever GCC pass it acts through, provided ALL: (1)
  once-written (any number of reads, [[ordinary-c-judge-decidable]] Ruling 1; multi-write
  locals only via Rulings 5-12 or Q51, which then govern exclusively); (2) real value: a
  computation in the target's bytes that only relocates where the value is named (no-op copies
  stay with the dead-store family; param copies only via Ruling 12); (3) byte-neutral
  (`build_insns == target_insns`); (4) fresh, not a borrow ([[staged-value-reused-variable]]
  keeps its bounds); (5) destination not live-pre-initialized; (6) standard prerequisites with
  `/* FAKE: ... */`. Licenses no extra handles to one object.
- **`do { ... } while (0);` wrap** ([[do-while-zero-exception]]): for ANY codegen effect
  (2026-07-06), natural geometry preferred, mandatory `/* FAKE: ... */` naming the effect,
  nested wraps need a single-level-insufficient justification. The ONE sanctioned
  no-semantic-purpose wrapper; `for(i=0;i<1;i++)`, `while(1){...break;}`, `if (1)` are not.

**Per-word splat symbol → aggregate merge** (2026-08-17; SOTN `Vram`-style merges). Two or
more splat `D_<addr>` scalars may become one aggregate when ALL hold:
- **(a)** The object model is established by evidence independent of and predating the
  byte-chasing (cross-TU stride indexing, base+offset addressing in the original, a committed
  naming-census schema), OR by compiler necessity (a1)-(a4) below.
- **(b)** The declaration reflects that shape (records where evidence shows records; a flat
  array only for a flat array); a record stride encoded as a magic index fails.
- **(c)** Complete: every merged symbol leaves C and the splat config (one C handle per
  location). A row may stay in `undefined_syms_auto.txt`/`named_syms.txt` while an
  `INCLUDE_ASM` sibling's `.s` references it, if no C names it and it is suffixed
  `/* alias of <base>+N; retire with <sibling> */`.
- **(d)** Canonical declaration in the shared header; never TU-local, never a per-use pointer
  pun.
- **(e)** Byte-neutral for every other consumer, `verify-oracle --rebuild`, layer-2.

*Compiler-necessity evidence for (a) (2026-09-26):* **(a1)** banked dumps (`.cse`, `.loop`,
`.lreg`, `.greg` and/or `BB2_*_DEBUG`, with command lines) for split and merged spellings name
the decision (pass + `tools/gcc-2.7.2` location) that needs ONE object, so that no
separate-object spelling can produce it; best split floor + a structural respelling recorded;
split spellings relying on refused/banned constructs are set aside, but an admissible
sanctioned-family split spelling that reaches the target defeats (a1). **(a2)** cc1psx
(calibration only) on the same preprocessed TU: split does NOT, merged DOES produce the
target's address-forming instructions (opcode, register, offset); outputs banked. **(a3)** Minimal
span: from the lowest to the highest label the function references; every label inside merged,
none outside. **(a4)** One record layout across the span, array length = span / record size.
**(a4′) mixed-field struct (Q7)**, in place of (a3)/(a4), with (a1)/(a2) against the separate
labels: (1) span = exactly the bytes the function accesses (incl. reachable indexed/walk
ranges); (2) members only for used bytes, width from the accesses, ONE declared signedness
under which every access is ordinary C (value casts ok, puns not), arrays only for indexed/
walked bytes; (3) unaccessed gaps only as compiler padding or one offset-named filler
(`u8 unkNN[k];`); (4) the ledger lists each member's offset, width, type and every accessing
instruction; (5) prongs (b)-(e) apply. **Forced-in bytes (Q13):** a byte inside the span the
function never touches may be a named member only if another function's ORIGINAL bytes access
exactly that offset/width (all accessors agree); its signedness equals an accessor's revealed
signedness: `lb`/`lh` vs `lbu`/`lhu`, or for words/store-only an ordered compare
(`slt`/`slti` vs `sltu`/`sltiu`), a non-divide `sra` vs `srl`, or `div` vs `divu` (equality
tests, bounds checks, divide-expansion shifts and `mult`/`multu` are NOT evidence); otherwise a
filler. **Q14:** when no instruction anywhere reveals signedness (full no-reveal listing incl.
sign branches and sub-word reads) AND signed/unsigned builds of every C accessor are
byte-identical (banked), the member keeps main's current type (or the SDK type assigned to it),
named in the ledger. Names follow naming-evidence rules (offset names always ok).

*Per-file declarations of the same bytes (Q21, exception to (c)/(d)).* Two TUs may declare the
same bytes with different C types ONLY when ALL hold:
1. **No single declaration compiles both files.** Under Q25 ("mechanism + search"): (a) the
   compiler mechanism by which each file's target depends on its declaration is named from
   banked dumps (pass + location); (b) every single-declaration spelling proposed by anyone
   (at least each file's own form as the shared one, plus every admissible aggregate) is
   banked, with at least one counting spelling per covered file, measured under cc1 and
   cc1psx, and its hit/miss of the file's full target recorded; (c) no declaration is
   DEFEATING (for every covered file, some counting spelling under it hits the full target).
   Spellings relying on refused/banned constructs (pun, F4, F5, alias rename, Q22 dummies) are
   set aside; under **Q23** so are spellings needing any FAKE/!FAKE-annotated construct (the
   ledger names the family and quotes its requirement), and the per-file form itself carries
   no FAKE construct in the covered functions. **Q24 cc1psx agreement:** for every counting
   spelling, cc1psx hits/misses the non-exempt governed instructions exactly as cc1 does. The
   reference form is the exact body/TU that will be on main; take cc1's and cc1psx's `-S` of it
   (before maspsx), strip directives/labels/comments, canonicalise symbol+offset by address,
   placeholder every register (except `$0`) and label, align with
   `difflib.SequenceMatcher(autojunk=False)`; branches compare by aligned destination. A
   governed instruction is EXEMPT only if it lies in a changed run with no equal cc1psx
   counterpart (then cc1's dumps decide). For mechanism parts that do not decide position,
   presence with multiplicity counts. Bank the script, alignment, governed instructions (by
   address and mechanism part) and exemptions; registers chosen outside the mechanism are
   compared under cc1 only.
2. **Same bytes, same accesses**: the declarations cover identical bytes and differ only in
   grouping (array vs scalars, struct vs member scalars); widths equal every access in that
   file's original instructions; no cast/pun/union/alias reach. Exception (Q52): a SOTN-cited
   local cast re-view `(T *)&D_...` marked `/* FAKE */` (with Q53 paperwork) is not refused.
3. **File-local**: each declaration in its own `.c`, none in a shared header; only the proven
   files; one handle per location within each.
4. **Annotated**: each names the other's file:line, states what the banked measurements show,
   cites the ledger and this ruling.
5. **Bytes**: `verify-oracle --rebuild` and sandbox 0 for functions landing with it.
6. **Layer-2** walks (1)-(5); a Judge PASS is not enough.

*Union word view over small fields (Q33; struct members Q46; trailing padding Q57).* An object
(global, local, or struct member) may be a union of the real object and one `s32`/`u32` member
ONLY when: (1) at every word-member site the original shows ONE `lw`/`sw` spanning exactly the
word (cited), and the build emits it; (2) exactly two members, the word at offset 0 (object ≥4
bytes, access at its first byte), no filler/third member/nesting; word signedness follows any
revealing instruction; name claims only "word"; (3) the word member is named only at those
sites, all other accesses through the real members; (4) no cast to/from/through the union, no
cast-to-union constructor, no union-typed param/return; (5) a global's union is its canonical
header declaration under prongs (a)-(e) (the word member is not object-model evidence); a
struct member's union replaces the members it spans at the same offsets, every other offset
and the size unchanged, except the compiler's own trailing round-up when the word raises
alignment (Q57: no offset moves, no filler, SHA1 byte-identical, and a whole-program search of
C and target bytes for containing layouts, size/stride uses and anything inside the added bytes,
each hit decided: added-bytes / containing-layout / old-size ⇒ FAIL, unrelated ⇒ ignored with
reason, new size ⇒ passes); (6) layer-2 on every commit adding a union or a word site (manual
path); (7) the F5 CLOBBER union, single-member unions and two-view (Silent Hill) unions stay
refused.

*One cast store on a local array (Q36, locals only).* Exactly one `*(s32 *)arr = v;` (or
`u32`) when: `arr` is a local array of a sub-word element type, total 4 bytes, with consumed
element reads elsewhere; the target has ONE `sw` at exactly its frame offset there (cited) and
the build emits it; no other cast of the array's address; the Q33 union spelling was banked and
missed first; inline annotation (address, ledger, ruling); layer-2 (manual path). Globals,
statics, members, merged aggregates and pointed-to objects never qualify.

**2026-07-01 (owner rulings; `pre-slim-2026-10-01:memory/project/sotn-family-research-2026-07-01.md`), standard prerequisites:**
- **Dead stores / self-assigns to locals+params** ([[dead-store-fake-exception]]),
  `/* FAKE */`; deadness is store-level (Ruling 2). Pins stay forbidden.
- **Constant-holder / dead scalar locals** ([[named-local-fake-exception]]); arrays and frame
  coercion stay forbidden; Q22 dummy subscripts/offsets refused.
- **C-level pointer aliases to globals** ([[pointer-alias-fake-exception]]); `asm("Sym")`
  alias renames stay forbidden.
- **Type-level MMIO volatile** ([[mmio-volatile-type-level]]): 0x1F801000-0x1F802FFF
  declarations, any shape, no annotation. Game-state globals keep
  [[legitimate-volatile-interrupt-touched]].
- **Duplicated statement into arms** ([[duplicated-statement-into-arms]]): a REAL statement
  in 2+ arms, byte-neutrality verified, FAKE annotation; duplicated calls only when cross-jump
  merges them to identical bytes (Q47).
- **Written-never-read local array** ([[dead-vars-local-array]]): only when the target bytes
  contain the dead stores; written, FAKE-annotated, dual-reviewed. `(void)&local` stays
  forbidden.
- **Fabricated dead call site** (`if (0) { call(); }`) — REFUSED (2026-08-17), any spelling.

**2026-08-18 (owner ruling b; SOTN surveys @8bd7c777), standard prerequisites:**
- **F3 compound-address duplication across call arg-lists**: `&base[i] + k`-class expressions
  written out at several argument positions instead of a pointer local; value consumed at each
  site; annotation at the site.
- **F6 cancellation pair / redundant condition**: exactly `i++; i--;` adjacent same-variable
  pairs, or an empty-if / redundant condition (`if (!i) { }`, `if (p && p)`); `!FAKE`-style
  annotation; exhaustion ledger. The `+= 2 / -= 1` respelling stays banned.
- **F7 unconditional common store duplicated into both arms**: the stores' values real and
  required; annotation.
- **Phantom-frame-slot volatile pad**: `volatile u32 pad[N];` (array form), FIRST local, never
  referenced, `// !FAKE` annotated, a per-function row in
  `engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS`, ledger frame forensics showing the
  slot untouched ([[phantom-slot-frame-lever]]), honest producers measured inert first, no
  `(void)pad;`. Earlier grants stand: func_8001E404, func_8001E6E4, func_8003CF84 (leading),
  func_8003CF84 `pad2[2]` (trailing, that function only).
  - **Trailing array with sibling evidence (Q35)**, relaxing only position, name and element
    type: (1) frame forensics list every `$sp` access and show the region untouched, in the
    locals area; (2) ≥2 COMPLETED-C functions in the same file declare a real, used array of the
    same type/count at the same offset after an object of the same layout, ≥1 as a separate
    local right after it (cited); (3) the declaration copies that sibling array's type and
    count as a separate local right after the object, and the frame then equals the target's;
    (4) the sibling's exact declaration + `volatile`, never referenced, with the non-volatile
    form banked byte-identical; (5) the sibling's identifier as name; (6) `/* FAKE */` comment
    citing siblings, ledger, ruling; (7) honest producers measured inert; (8) its own
    `_SANCTIONED_UNWRITTEN_PADS` row and layer-2 (manual path).

**Surveyed and NOT extended (2026-08-18, refusals stand):** F1 constant→local→local staging
chain; F2 signedness-split dual read; F4 cross-symbol arithmetic (SOTN's norm is the struct
merge); F5 union-constructor CLOBBER. Never relaxed: pins, hardcoded-`$N` asm, build-time
rewriting, alias renames, redundant width casts.

**Two narrow F4 exceptions (2026-09-30), nothing else may cite them:**
- **D_800A37D2 / D_800A37D3 (Q63):** only after the one-array and two-member-struct whole-tree
  forms are banked failing (scratch SHA1 + differing words); then two adjacent `u8` scalars,
  indexed via the first's address (`p = &D_800A37D2; p[t != 0]++`,
  `(&D_800A37D2)[D_800A3748]`), each indexed use and the `p` assignment `/* FAKE */`-annotated,
  `p` meeting [[pointer-alias-fake-exception]]. For func_8001C8DC and func_8003CF84 only.
- **Practice-menu per-word reads (Q73):** for `D_80101FA0`, `D_80101FA8`, `D_801023EC`,
  `D_801023F4`, `D_80101FBC`, `D_80101FC4`, `D_80102408`, `D_80102410`, read by own name only in
  func_8002BC68 / func_8002BEA0, only after the three single-object spellings (direct struct
  field, typed base pointer, per-record pointer) are banked missing for the shared-base reason
  (`.cse` `use_related_value` and `.greg`/final `.s` addressing), with Q31 mechanism + search;
  each per-word declaration `/* FAKE */`-annotated; every other access goes through
  `g_practice_menu_table`.

## Owner ruling 2026-09-30 — SOTN precedent suffices (Q50; Q51-Q53, Q55)

A construct is admissible when it verifiably exists in the SOTN repo. "Verifiably" (author's
reading): (1) a file:line in a PS1-build file (`config/splat.us.*` / `splat.hd.*` membership;
not PSP/Saturn, not inside a non-PS1 version guard), naming the commit read (local clone
`C:/Users/Trenton/Desktop/sotn-decomp` @db41b28), C source only; a header construct counts with
a splat-member use site that itself meets (2)/(4); (2) it does the same thing as ours when read
in context (a shape-index hit is not a citation); (3) a fresh layer-2 verifies (1), (2), (4)
against the SOTN source, manual path only (never a Judge PASS); (4) matched code (Q55): not
`INCLUDE_ASM`/`INCLUDE_RODATA`, not `NON_MATCHING`/disabled under SOTN's PS1 defines, not in a
function SOTN still carries as asm. Psyz and other projects are not evidence. Every
SOTN-admitted construct carries `/* SOTN: <file>:<line> @<commit> */` (no symbol names).
- **Q55 precedence:** a citation meeting (1)-(4) admits a construct that an older rule refuses
  (inline-asm default ban, volatile two-prong, fabricated dead call, "may not be re-proposed"),
  with Q53's prerequisites.
- **Q51:** a variable reused exactly as a cited SOTN variable is reused (same roles, written and
  read at corresponding statements) is admitted without a Ruling 5-11 package.
- **Q52:** a construct SOTN marks as hack/debt (any `FAKE`/`fake`/`hack`/`TODO`/`FIXME`
  comment, hack-named label/identifier/macro, `HACKS`-only code) counts, and ours carries
  `/* FAKE: ... */`.
- **Q53:** the family's paperwork is still owed (annotation, exhaustion, byte-neutrality,
  review), but not its "is this kind allowed" test; match-motivated ⇒ `/* FAKE */` + ledger
  proof simpler spellings were tried.

Related: [[ordinary-c-judge-decidable]] · [[review-discipline-before-commit]] ·
[[judge-sole-gate]] · [[no-compiler-divergence]] · [[register-alloc-pure-c]]
