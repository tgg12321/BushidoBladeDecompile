# SELF-VET — func_80062020 (grind s19r, REDERIVE modality; submitted after the 2026-09-03 23:06 Judge PASS)

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
— every token names a member that is being cleared. Two INDEPENDENT community decompilations on THIS
compiler write the identical statement from spec, re-verified from the raw corpus files by this
session (tmp/grind/func_80062020/s19r/corpus_verify.txt, not taken on a prior session's word):
decomp.me scratch `wTOCG` (function drawAll_YA) — compiler `gcc2.7.2-psx`, flags
`-O2 -G0 -g -Wa,--aspsx-version=2.34 -Wa,--expand-div` (this project's own optimisation level,
-G0 and aspsx version), score 0 of max_score 17600, `match_override: false` — authors
`dB[actSw].draw.r0 = dB[actSw].draw.g0 = dB[actSw].draw.b0 = 0;` and byte-matches
`sb $zero,0x1B($v0) / sb $zero,0x1A($v0) / lui $at,%hi(dB+0x19) / addu $at,$at,$v1 /
sb $zero,%lo(dB+0x19)($at)` at 0x8016F0A4-B4; scratch `w4QFC` (InitEnemies), same compiler,
score 0, `match_override: false`, authors
`enemies[numEnemies].rotationVec.vx = ... .vy = ... .vz = 0;`. The earlier sotn-decomp citation
(src/dra/62DEC.c:961) is WITHDRAWN from this vet as evidence about this compiler: sotn-decomp
builds every PSX TU with cc1-psx-26 (tools/builds/gen.py:777), a different compiler line, and
s19 already recorded that correction.
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
Stated plainly, because two layer-1 reviews read the previous wording as evasive. An epilogue
spelling enumeration WAS run across s16-s18 (38 spellings over four declaration shapes), and its
result is recorded in the ledger. What the enumeration did was ELIMINATE alternatives, not invent
this statement: every other reaching spelling it found buys its second consumed store with dead
code (dead scalar local, dead re-store, empty-bodied `if` — all refused by s18 itself and banked
in rejected/epilogue-nonchain-consumption-carriers-deadcode-s18.c), and the plain chained
assignment is the only dead-code-free member of the reaching set. The chained assignment itself is
not a detector-evading spelling and is not permuter output: the s4 permuter campaigns (~46k
iterations) produced only the dual-spelling pointer body that is `banned_constructs[0]`, which is
not in this diff. The decisive provenance fact is external and was verified from raw data by this
session, not inherited: two independent byte-matched `gcc2.7.2-psx` decompilations by unrelated
projects (`wTOCG`, `w4QFC`; score 0, match_override false; wTOCG on this project's own -O2 -G0
aspsx-2.34 flags) contain the same authored construct producing the same DISP|DISP|LOSUM
arrangement — so this is the spelling programmers write for "zero these three members", not a
spelling reverse-engineered here. Finally, the governing rule forecloses the objection in terms:
choosing among semantically-truthful spellings by observed codegen effect is the method of
matching decompilation, not a cheat (.claude/rules/ordinary-c-judge-decidable.md, owner ruling
2026-08-31, Ruling 1 sec.3), as the 2026-09-03 23:06 Judge PASS states for this exact body.

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
The third standing entry is not a C construct at all (it names a disposition history); the
2026-09-03 23:06 Judge ruling states it is superseded by that ruling.

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

## MEASUREMENTS (this session, grind s19r, chassis HEAD 3aa1995f + this diff)
- `verify-oracle --rebuild --allow-dirty` then `verify-oracle --allow-dirty` -> ok true,
  build_matches true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
  original_sha1_locked. Taken WITH the two suffixed rows in undefined_syms_auto.txt in place.
- `sandbox func_80062020 --disable all` -> score 0, target_insns 38, build_insns 38,
  scorable true, rules_dropped 0.
- Diff LEFT IN PLACE in src/text1b.c, src/text1b_b.c, include/game.h,
  undefined_syms_auto.txt for the driver's re-verification. This is the EIGHTH independent
  SHA1==oracle proof of this body.
- Corpus re-verification performed this session from the raw scratch JSON:
  tmp/grind/func_80062020/s19r/corpus_verify.txt.
