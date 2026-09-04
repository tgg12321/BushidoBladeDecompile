# SELF-VET — func_80062020 (grind s19, forensics; submitted after the 2026-09-03 22:38 Judge PASS)

CONSTRUCTS: (1) an aggregate record declaration for the table at 0x800F1198, spelled once in
include/game.h; (2) an ascending 3-deep chained assignment zeroing all three members of the
terminator record; (3) a byte-neutral marker suffix appended to two linker-script rows.

## T1 semantic purpose
(1) The declaration states what the storage is: a table of 3-word records. It changes what the
C means (one object, three members) and is how every consumer now names it. Semantic purpose:
yes — it is the object model.
(2) The chained assignment clears the terminator record's three columns. Its observable effect
is the three zero stores; without it the terminator record is not cleared and the function is
wrong. Semantic purpose: yes.
(3) The suffix is a comment in an ld script (`undefined_syms_auto.txt` is consumed as a linker
script, Makefile:99). It emits nothing and is required by the aggregate-merge rule's prong (c)
amendment as documentation of the pending retirement. It has no codegen role at all — it is
paperwork mandated by the rule, and the SHA1 proof below was taken with it in place.

## T2 human-programmer
(1) A programmer with the spec ("a table of 3-word records, terminated by a record whose first
word has bit 0 clear") declares a record array. That is the ordinary declaration; the per-word
`D_800F1198` / `D_800F119C` / `D_800F11A0` scalars are splat inventions, not source.
(2) `rec[i].unk0 = rec[i].unk4 = rec[i].unk8 = 0;` is the textbook way to zero the three
members of a record in one statement. Nothing in it would make a reader ask "why is this here?"
— every token names a member that is being cleared. sotn-decomp ships the identical shape
unannotated (`src/dra/62DEC.c:961`, `D_80137B20[i].vx = D_80137B20[i].vy = D_80137B20[i].vz = 0;`),
which is corroborative on how a matching-decomp author writes this, though it is a different
compiler line (cc1-psx-26) and is NOT credited as GCC-2.7.2 family precedent.
(3) Yes — a comment saying "this row is an alias of base+4 and retires with func_800620B8" is
exactly what a maintainer writes when a row must survive for a not-yet-decompiled sibling.

## T3 GCC-internals justification
No construct in the diff is justified by a GCC internal. The mechanism is *explained* in the
ledger (this session dump-proved it: `tools/gcc-2.7.2/expr.c:3457-3464`, `store_field`'s
"if a value is wanted ... make the address stable for multiple use" gate, which copies a
consumed store's address to a register and leaves a discarded store's address symbolic), but
the explanation is not the reason to write the statement. The reason is "clear the three
columns of the terminator record". This is the direction the policy asks for: the program logic
IS the explanation, and the compiler note is documentation of what the compiler then does.
There is no lever naming, no pass-targeted construct, and no FAKE annotation anywhere.

## T4 permuter/search provenance
The chained assignment was not produced by the permuter. The s4 permuter campaigns
(~46k iterations) produced only the dual-spelling pointer body that is `banned_constructs[0]`
and is not in this diff. The form here comes from asking what the storage is (the record
declaration) and then writing the ordinary clearing statement for it. It survives the
provenance test independently: five semantically unrelated bodies containing no chain reach the
same instruction arrangement (s18), so the arrangement is not a signature of this spelling, and
this spelling is the only member of the reaching set that contains no dead code.

## T5 family check
Walking the catalog: no register-asm pin, no `__asm__`, no scheduling barrier, no volatile in
any spelling, no unused/dead local, no dead store or self-assign, no dead conditional or empty
`if`, no `if (1)`, no dead goto or label pad, no DImode chain, no pointer local, no alias
rename, no redundant width cast, no `.ld` reorder. The two source constructs are (1) a
declaration in the sanctioned aggregate-merge family and (2) an ordinary C statement with a
truthful semantic reading, ruled ordinary C on the merits by the 2026-09-03 22:38 Judge.
Standing bans re-checked one by one: `banned_constructs[0]` (a pointer local for two columns
plus a second, differently-spelled materialisation of the row address for the third) — this body
has no pointer local and all four record writes use the single spelling `D_800F1198[i].unkN`.
`banned_constructs[1]` (re-filing a merits-FAILed body with only the comments changed) — this
submission is not a comment edit: it follows a Judge ruling that adjudicated the construct on
the merits and PASSED it, and that ruling explicitly states the body may now be submitted.

## T6 naming-announces-intent
Identifiers in the diff: `Unk800F1198Record`, `D_800F1198`, `unk0`, `unk4`, `unk8`, `i`, `ofs`,
`t`, `arg0`. `unkN` is the project's standard unknown-member convention (offset-named), not a
coercion marker; there is no `pad`, `dummy`, `unused`, `spill`, `slack`, `_buf` or `tail`.
Every declared name is read and written by real code: `i` and `ofs` drive the copy loop, `t`
carries the terminator test, and all three members are stored.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Per-word splat symbol -> aggregate merge
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: .claude/rules/no-new-park-categories.md:238
  Prongs, all mandatory, each answered:
    (a) Object model established by evidence independent of and predating the byte-chasing:
        the original binary walks the table with a 12-byte-stride induction register
        (asm/funcs/func_80062020.s:27, `addiu $v1,$v1,0xC`) and addresses record members
        through one base register at displacements 0x8 / 0x4 (0x8006209C / 0x800620A0).
        Both signals the prong names are present in the target's own bytes; symbol adjacency
        is NOT relied on ([[splat-symbol-names-are-not-evidence]]). The sibling
        asm/funcs/func_800620B8.s reads record 0's three members as absolute loads, which is
        consistent with a 3-word record but is not independent stride evidence and is not
        claimed as such.
    (b) The declaration reflects that shape: a 3 x s32 record table, indexed `D_800F1198[i]`.
        No magic-number stride appears in any index expression.
    (c) Complete: all nine vestigial per-word externs are deleted from src/text1b.c and
        src/text1b_b.c; a grep of src/ + include/ finds no C site naming D_800F119C or
        D_800F11A0 (the single hit, include/game.h:26, is inside the merge's own comment).
        The two `undefined_syms_auto.txt` rows stay because asm/funcs/func_800620B8.s is still
        INCLUDE_ASM and references them, and they carry the amendment's mandated suffix
        `/* alias of D_800F1198+N; retire with func_800620B8 */`
        (.claude/rules/no-new-park-categories.md:245-259).
    (d) Spelled at the canonical declaration in the shared header include/game.h. Never
        TU-local, never a per-use pointer pun or cast.
    (e) Byte-neutrality verified for every consumer by a full
        `verify-oracle --rebuild --allow-dirty` this session: build_sha1
        62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked, build_matches true.

  (No second sanctioned-family claim. The epilogue statement is ORDINARY C and needs no
  frozen family: it has a truthful semantic reading — clear the terminator record — is
  writable from the spec, and carries zero surplus text. The rule that governs it is
  .claude/rules/ordinary-c-judge-decidable.md:58, under which only constructs with NO
  semantic reading are refused; the 2026-09-03 22:38 Judge ruling at
  docs/grind/decisions.md:22216 adjudicated this exact statement on the merits and PASSED it.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. There is no `/* FAKE: ... */` in the diff
because no construct in it belongs to a FAKE-requiring family: the aggregate merge's prongs do
not mandate an annotation, and the epilogue statement is ordinary C.

## MEASUREMENTS (this session, chassis HEAD 3c508c4b + this diff)
- `sandbox func_80062020 --disable all` -> score 0, target_insns 38, build_insns 38,
  scorable true, rules_dropped 0, cheat_asm_stripped 165.
- `verify-oracle --rebuild --allow-dirty` then `verify-oracle --allow-dirty` -> ok true,
  build_matches true, build_sha1 == original_sha1_locked
  (62efab4f73f992798c43e8c730aa43baa10bb4fa). Taken WITH the two suffixed rows in place.
- Diff LEFT IN PLACE in src/text1b.c, src/text1b_b.c, include/game.h,
  undefined_syms_auto.txt for the driver's re-verification.
- Forensic write-up: tmp/grind/func_80062020/s19/forensics_s19b.md; dumps
  tmp/grind/func_80062020/s19/s19b_func80062020.{rtl,combine,cse2,greg,s}.
