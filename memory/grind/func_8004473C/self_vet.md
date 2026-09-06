# SELF-VET — func_8004473C  (s4, 2026-09-06)

CONSTRUCTS: (1) chained assignment `D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());`;
(2) loop-body statement order (`dst->unk6 = 0;` second-to-last, `dst->unkC = 0;` before
`dst->unkA = 4;`); (3) comma-increment order `dst++, src++, i++`; (4) the `Unk800A9CF8Header`
aggregate merge of the per-word splat symbols D_800A9CF8/D_800A9CFA/D_800A9CFC/D_800A9CFE/
D_800A9D00/D_800A9D04/D_800A9D08 (chassis, inherited from s1); (5) the `Rec4473C` stride-0x68
record typedef. NO FAKE construct, no inline asm, no register pin, no volatile, no dead store,
no dead local, no pad, no opaque variable, no cast that is not a real type conversion.

## T1 semantic purpose:
(1) Both halves are observable: `src` is the game_GetCharData() table the loop reads from, and
the header field unk10 must record that same pointer for the sibling func_80044B30, which reads
`D_800A9CF8.unk10 + a0 * 0x68`. Writing it as one assignment expression is a spelling of the
same two effects, not an added effect; removing either half changes behaviour.
(2)/(3) Statement order and comma-operator order are the program's own text; every statement
stores a value the function is required to store, and every increment advances a pointer or the
counter the loop needs. Nothing is added or removed relative to the natural body.
(4) The aggregate replaces seven splat-invented per-word names with the one object they
actually denote; every field is read or written by this TU (unk0/unk2/unk4/unk6/unk8/unkC/
unk10 across func_80044670, func_8004473C, func_80044B30, func_80044C70).
(5) `Rec4473C` is the record type of the two stride-0x68 tables; every named field is accessed.

## T2 human-programmer:
Yes for all five. `g.field = (s32)(p = get());` is a common C idiom for "publish the pointer and
keep it"; a reader asks nothing about it. The field-store order and `dst++, src++, i++` are
choices a human makes freely, with no reader-visible "why is this here?". A struct for a
header the code addresses as base+offset, and a record type for a stride-0x68 table, are what a
programmer would have written in the first place — the per-word `D_<addr>` names are splat's
invention, not the original source's.

## T3 GCC-internals justification:
The candidate does NOT rest on a GCC internal for its right to exist. Each construct is
justified by the program: the chained assignment is how the pointer is published and kept, the
statement order is the initialisation order, the increment order is the loop's own text. GCC
internals appear in the ledger only as the EXPLANATION of why the previous spellings cost extra
instructions (sched1's birthing_insn_p boost of a standalone copy insn); that explanation is
diagnostic, not the semantic warrant, and no construct here exists solely to steer a pass. The
constructs would all still be in the source if the compiler scheduled differently.

## T4 permuter/search provenance:
A permuter campaign ran this session and its two best proposals were REJECTED as cheats and
banked: `(float)` round-trip of the pointer (PERM_float_cast_folds_src_to_zero.c — semantically
wrong, folds src to 0) and a redundant store to the GLOBAL (`D_800A9CF8.unk10 += 0;` /
`D_800A9CF8.unk10 = (s32)src;` — PERM_dead_global_selfstore.c; the sanctioned dead-store family
is LOCALS/PARAMS only, so this is a first reach of an unsanctioned family). Neither is in the
diff. The submitted chained-assignment form was written BY HAND (tmp/grind/func_8004473C/s4b/
search6.py, form `O4_embedded_assign`) as the ordinary-C hypothesis for what the rejected
global self-store was doing mechanically, and it measures 0 on its own merits. Levers (2) and
(3) came from an exhaustive hand-written statement-order sweep, not from auto-search output.

## T5 family check:
(1)/(2)/(3)/(5) are ordinary C and match NO family in the forbidden catalog: no register pin,
no `__asm__`, no scheduling barrier, no volatile in any spelling, no dead/self store, no dead
conditional or empty-body `if`, no `if (1)`, no dead goto or label pad, no DImode chain, no
alias rename, no opaque constant variable, no redundant width cast (`(s32)` of a pointer and
`(Rec4473C *)` of an `s32` are real conversions consumed by the assignment; `(float)` was the
permuter's proposal and is rejected), no linker/rodata reordering. (4) is the sanctioned
per-word-splat-symbol -> aggregate-merge family; claim below.

## T6 naming-announces-intent:
No name in the diff announces coercion intent. The locals are `src`, `dst`, `i` — each read and
each carrying the value its name states. There is no `pad`, `_pad`, `dummy`, `unused`, `spill`,
`sp_*`, `_buf`, `tail`, `slack` or `_frame_pad`. (The `pad18`/`pad38`/`pad60` members of
`Rec4473C` are struct layout filler for unmapped bytes of a real 0x68 record — they are never
declared as locals, never addressed and never stored to; they exist so the mapped members land
at their measured offsets.)

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Per-word splat symbol -> aggregate merge
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: .claude/rules/no-new-park-categories.md:238
  PRONGS: (a) object model established before this byte-chasing session — the naming census
    rows quoted in the dispatch DATA MODEL block ("header state at -0x12 from buffer base",
    "header pointer at -0xC from buffer base", "struct field accessed at base+0 and base-0x10")
    plus the base+offset addressing in the shipped binary (`addiu a0,&D+0x10` then
    `addiu a3,a0,-0x10`, target insns 10/11/14), recorded in evidence.md in s1. (b) the merged
    declaration is a header/record struct, which is the shape the evidence shows; no magic
    stride index. (c) every merged per-word symbol is removed from C — no C code in the tree
    names D_800A9CFA/D_800A9CFC/D_800A9CFE/D_800A9D00/D_800A9D04/D_800A9D08 any more; the
    `undefined_syms_auto.txt` rows are still referenced by the still-`INCLUDE_ASM` sibling
    `asm/funcs/func_80044800.s`, which the 2026-09-03 amendment at
    .claude/rules/no-new-park-categories.md:252 expressly allows, and adding the required
    `/* alias of D_800A9CF8+N; retire with func_80044800 */` suffix is an operator packaging
    step: that file is OUTSIDE a grind session's edit surface (the previous session on this
    function was discarded for editing exactly it). (d) NOT YET SATISFIED IN THIS DIFF AND
    DECLARED PLAINLY: the aggregate is declared TU-locally in src/text1a_c.c because
    include/game.h is outside a grind session's edit surface. `memory/grind/func_8004473C/
    candidate_merge.patch` is the header-canonical variant (declaration in include/game.h),
    which s3 measured byte-identical; moving the two typedefs + the `extern` there is the
    one operator step needed to close prong (d), and it changes no bytes. (e) byte-neutrality
    for every other consumer is verified: `verify-oracle` on the applied tree returns
    ok=true, build_sha1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked oracle, so
    all four sibling functions in the TU still match; layer-2 cheat-reviewer is the driver's step.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. The diff contains no construct from any family
that mandates a `/* FAKE */` annotation. (The `do { } while (0);` with its FAKE comment inside
`func_80044670` is pre-existing at HEAD, is untouched by this diff, and is not part of this
candidate.)
