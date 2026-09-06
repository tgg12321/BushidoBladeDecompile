# SELF-VET — func_8004473C

Session s4 (synthesis). Diff = `memory/grind/func_8004473C/candidate_merge.patch` applied to a
clean HEAD, plus the prong-(c) alias-of suffixes on seven rows of `undefined_syms_auto.txt`.
Measured THIS session: `sandbox func_8004473C --disable all` -> `{"score": 0, "target_insns": 49,
"build_insns": 49, "scorable": true, "rules_dropped": 0}`; full-tree
`verify-oracle --rebuild --allow-dirty` -> `ok: true`,
`build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == original_sha1_locked`.

The diff also DELETES cheats: the register-pinned m2c body of `func_8004473C` currently on main
(`register s32 temp_v0 asm("v0")` and five siblings, `s32 _sp_pad[2]`, and an
`__asm__ volatile` memory-clobber line) is replaced by an ordinary C initialisation loop. Net cheat
count for this function goes from 8 constructs to 0.

CONSTRUCTS: (1) `Unk800A9CF8Header` aggregate declared in `include/game.h`, replacing the seven
per-word splat scalars D_800A9CF8/CFA/CFC/CFE/D00/D04/D08; (2) `Rec4473C` record typedef (stride
0x68) describing the pointee of the two tables the loop walks; (3) the chained assignment
`D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());`; (4) alias-of comment suffixes on
seven `undefined_syms_auto.txt` rows. NOT in this diff, declared for completeness because layer-1
will read the same file: the `do { } while (0);` in the neighbouring `func_80044670` is PRE-EXISTING
HEAD content (unchanged context lines in the patch), already carrying its `/* FAKE: ... */`
annotation; my diff neither adds nor modifies it.

## T1 semantic purpose
(1) YES. The aggregate is the declaration of a real 0x18-byte object in the game's data segment.
Every member is read or written by real code: `unk0` (a1 stashed by func_80044670, re-read by
func_8004473C and func_80044B30 and func_80044C70), `unk2`, `unk4` (stage id, switched on by
func_80044670 and func_80044B30), `unk6` (the entry count that bounds this function's loop),
`unk8`/`unkC` (two table pointers, both incremented by func_80044C70), `unk10` (the
game_GetCharData table, written here and read by func_80044B30). Changing the declaration does not
add or remove any storage; it names storage that already exists.
(2) YES. Every field the loop touches is a real field of a real record; the three copied fields
(`unk2C`/`unk30`/`unk34` -> `unk4C`/`unk50`/`unk54`) are a genuine 12-byte vector copy that
func_80044B30 also performs on the same offsets.
(3) YES. The store to `unk10` is a required side effect (func_80044B30 reads it), and `src` is the
required loop source pointer. Both halves of the chained assignment are consumed.
(4) YES for the reader / NO for the compiler: comments do not reach cc1. They exist because prong
(c)'s 2026-09-03 amendment mandates that exact text on a surviving per-word row.

## T2 human-programmer
YES for all four. A programmer holding the spec — "a 0x18-byte header of stage/entry bookkeeping;
initialise `count` records of the destination table, zeroing the scratch fields, stamping the
header's id and the constant 4, copying the source record's 12-byte vector, and marking the entry
unassigned with -1" — writes exactly this: a struct for the header, a struct for the record, a
`for` loop over two walking pointers. Nothing in the body would make a reader ask "why is this
here?". The chained assignment `p_field = (s32)(local = call())` is a common C idiom ("store it and
keep it") and reads truthfully.

## T3 GCC-internals justification
NO — no construct in this diff is justified by a GCC internal. The aggregate merge is justified by
the ORIGINAL BINARY's addressing: `asm/funcs/func_8004473C.s` forms `&D_800A9D08` in `$a0`, does
`addiu $a3, $a0, -0x10`, and then reads `lhu 0($a3)` (= D_800A9CF8) and `lh 6($a3)`
(= D_800A9CFE) off that one base register. That is one object addressed base+offset across a
0x10-byte span, which is object-model evidence, not codegen steering. `Rec4473C` is justified by the shared 0x68 stride
across three functions. The chained assignment is justified by the program logic (store and keep).
The 13-instruction residual that earlier sessions chased WAS attributed to sched1
`birthing_insn_p` — but that attribution is now spent history: this form reaches zero without
invoking it, and the C is not shaped by it.

## T4 permuter/search provenance
NO. This body was not produced or selected by decomp-permuter. Two permuter-derived forms were
explicitly REJECTED and are banked in `memory/grind/func_8004473C/rejected/`
(`PERM_float_cast_folds_src_to_zero.c`, `PERM_ptr_alias_plus_constant_holder.c`). The winning form
came from s1's object-model correction (40 -> 13) plus the header-canonical spelling of the same
merge (13 -> 0); it is the spelling a human would write from the corrected object model, and it
passes every detector on its semantics, not on a spelling gap.

## T5 family check
Constructs (2), (3) and (4) are ORDINARY C / config housekeeping and claim no family: a struct
typedef for a strided record table, a chained assignment, and a mandated comment. Construct (1) is
a restructuring and DOES need a family — it is the per-word splat symbol -> aggregate merge family,
claimed below with all five prongs met. No construct matches any forbidden-family entry: there are
no register pins (the diff removes six), no `__asm__` (the diff removes one), no scheduling
barrier, no `volatile`, no unused local array or pad (the diff removes `s32 _sp_pad[2]`), no dead
store, no self-assign, no constant holder, no alias rename, no empty-body `if`, no dead goto, no
redundant width cast, and no linker-script reorder.

## T6 naming-announces-intent
NO. `src`, `dst`, `i` are the loop's roles. `unkN` members follow the project's offset-naming
convention for un-named fields; the three fields with known meaning carry explanatory comments
(`stage id`, `entry count`, table pointers). `pad18` / `pad38` / `pad60` name REAL untouched spans
INSIDE a struct that describes an object of known size — they are layout padding in the ordinary
C-struct sense, not frame-coercion locals; they are members of a type, never declared as locals,
never taken the address of, and their presence is forced by the offsets of the members around them.
`Rec4473C` / `Unk800A9CF8Header` are address-derived type names in this project's existing style
(cf. `Unk800F1198Record` in `include/game.h`).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Per-word splat symbol -> aggregate merge
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:238`

  Prong-by-prong, all five mandatory prongs MET:
  (a) OBJECT MODEL, EVIDENCE INDEPENDENT OF AND PREDATING THE BYTE-CHASING. Base+offset addressing
      in the ORIGINAL binary: `asm/funcs/func_8004473C.s:11-12` builds `&D_800A9D08` in `$a0`,
      `:15` does `addiu $a3, $a0, -0x10`, and `:24` / `:43` then read `lhu 0($a3)` and
      `lh 6($a3)` — i.e. ONE base register reaching 0x800A9CF8, 0x800A9CFE and 0x800A9D08 with
      signed displacements, a 0x10-byte span of a single object. This is NOT symbol adjacency —
      the check that [[splat-symbol-names-are-not-evidence]] demands. Corroborating (C side, and
      already on main before this session): `func_80044C70` bumps `D_800A9D04` and `D_800A9D00`
      together by the same delta, the signature of two pointer fields of one header.
      (I re-verified this myself rather than inheriting it. I also checked the earlier claim that
      `asm/funcs/func_80044800.s` reads `$v1-0xA` off `&D_800A9D04`: it does NOT — it forms that
      address at `:6-7` and only does `lw 0($v1)` at `:17`. That claim is withdrawn and prong (a)
      does not rest on it; the func_8004473C evidence above is sufficient on its own.)
  (b) DECLARATION REFLECTS THE DOCUMENTED SHAPE. `Unk800A9CF8Header` is a flat 0x18-byte struct
      whose member offsets are exactly the addresses observed (0,2,4,6,8,C,10,14); no magic stride
      is encoded in any index. The separate strided table it points at is declared as its own
      record type (`Rec4473C`, stride 0x68), which is precisely the "a struct/record table where
      the evidence shows records" reading.
  (c) MERGE IS COMPLETE, under the 2026-09-03 amendment. No C code names any of the seven per-word
      symbols after this diff — verified by `grep -rn` over `src/` and `include/`, whose only hits
      are inside the explanatory comment block in `include/game.h`. The seven rows STAY in
      `undefined_syms_auto.txt:302-308` because the still-`INCLUDE_ASM` sibling `func_80044800`
      references three of them from `asm/funcs/func_80044800.s` and deleting them would break that
      assembly; each row is now suffixed in the mandated form,
      `/* alias of D_800A9CF8+0xN; retire with func_80044800 */` (the base row instead records that
      it is the base). `named_syms.txt` contains no rows for these symbols (grep: no hits), so it
      needs no edit. Precedent for this exact suffix spelling already in-tree:
      `undefined_syms_auto.txt:527` and `undefined_syms_auto.txt:977`.
  (d) SPELLED AT THE CANONICAL DECLARATION IN THE SHARED HEADER, NEVER TU-LOCAL. The typedef and
      `extern Unk800A9CF8Header D_800A9CF8;` are in `include/game.h`, alongside the existing
      `Unk800F1198Record` / `D_800F1198[]` pair. This prong is the ONE ground of the 2026-09-06
      04:14 layer-1 FAIL and is the entire content of this session's remedy; the TU-local spelling
      that FAILed is gone from `src/text1a_c.c` and is not re-introduced anywhere. The scope grant
      that makes `include/game.h` editable is `tools/grinder/scope_allow.txt:61`, written by the
      driver on the Judge's ESCALATE(integration-handoff) verdict of 2026-09-06.
  (e) BYTE-NEUTRALITY FOR EVERY OTHER CONSUMER. The three C siblings the patch rewrites to members
      (`func_80044670`, `func_80044B30`, `func_80044C70`) and every other TU are byte-neutral:
      full-tree `verify-oracle --rebuild --allow-dirty` this session returned `ok: true` with
      `build_sha1 == original_sha1_locked == 62efab4f73f992798c43e8c730aa43baa10bb4fa`.
      Layer-2 cheat-reviewer is the operator's step and is not claimed here.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in this diff. Nothing in it is fake: the aggregate
declares real storage, the record typedef describes a real strided table, the chained assignment
consumes both of its values, and the comment suffixes are the text prong (c) mandates. The one
`/* FAKE: ... */` visible in `src/text1a_c.c` (the `do { } while (0);` inside `func_80044670`) is
PRE-EXISTING HEAD content that this diff leaves untouched, and it already carries what+mechanism
(`reorg.c relax_delay_slots` / `NOTE_INSN_LOOP_BEG` / `LABEL_OUTSIDE_LOOP_P`); its family is the
do-while(0) wrap, `.claude/rules/do-while-zero-exception.md`, sanctioned at
`.claude/rules/no-new-park-categories.md:263`.
