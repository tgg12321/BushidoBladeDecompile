# SELF-VET — func_8004473C

CONSTRUCTS: aggregate-merge declaration `Unk800A9CF8Header D_800A9CF8` in include/game.h
(+ the alias-suffixed splat rows in undefined_syms_auto.txt); TU-local `Rec4473C` record
typedef; assignment-expression `D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());`;
for-increment order `dst++, i++, src++`; loop-body statement order (`dst->unkC` before
`dst->unkA`, `dst->unk6 = 0;` placed after `dst->unk54 = src->unk34;`).

## T1 semantic purpose:
- Aggregate merge: yes. The seven splat per-word scalars are one 0x18-byte object in the
  original binary (base+offset addressing, see prong (a) evidence below); the struct is the
  declaration of that object and every member access is a real read/write of program state.
- `Rec4473C`: yes. It is the record type of the two 0x68-stride tables the function copies
  between; every member named is loaded or stored.
- Assignment-expression: yes. Both effects are needed by the program - the call result is
  published to `D_800A9CF8.unk10` AND kept in `src` for the copy loop. It is one statement
  doing exactly the two things the function must do; there is no discarded value, no dead
  read and no extra object.
- Increment order / statement order: these are not constructs, they are the order of
  statements that all have observable effect (fifteen field stores and three pointer/counter
  increments). Reordering independent stores does not add or remove anything.
## T2 human-programmer: yes for all. `x = (T)(p = f());` is a common C idiom for "store it and
keep it"; a record typedef and a header struct for a header block are how anyone would write
this; the field-store order is just an order, and no reader would ask "why is this here?" about
any line - every line writes a field the function is specified to write.
## T3 GCC-internals justification: the C is justified by program logic alone - it assigns the
table pointer, publishes it, and copies fifteen fields per entry. GCC internals appear in this
session's ledger only as the EXPLANATION of why one legal spelling emits the original's bytes
and another does not (sched1 T-slots, birthing_insn_p, local-alloc/global-alloc seating); no
construct here exists to steer a pass. Removing any of them changes what the function does
(the merge) or nothing at all (statement order) - none of them is a no-op inserted for codegen.
## T4 permuter/search provenance: no permuter, no auto-search. Every form this session was
hand-derived from the cc1 -da .sched/.lreg dumps and measured with
`sandbox func_8004473C --disable all`; the winning body is the natural spelling the dumps
predicted, not a survivor of a random search.
## T5 family check: no forbidden family is touched. There is no register pin, no `__asm__`, no
scheduling barrier, no volatile, no dead store, no dead local, no pad array, no alias rename,
no empty `if`, no `do{}while(0)`, no opaque arithmetic, no width cast for its own sake, and no
build-time asm rewriting. The only construct that needs a sanctioned family is the aggregate
merge (claimed below); everything else is ordinary C that any reader would take at face value.
## T6 naming-announces-intent: no name in the diff is `pad`/`dummy`/`unused`/`spill`/`tmp`/
`slack`. `src`, `dst`, `i` are the loop's real roles; `Rec4473C` / `Unk800A9CF8Header` are
address-derived type names in this repo's existing convention (cf. `Unk800F1198Record`);
the struct's `padXX` members are genuine layout padding inside a 0x68-byte hardware record,
not frame coercion, and no address of them is taken.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: per-word splat symbol -> aggregate merge
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: .claude/rules/no-new-park-categories.md:238
  Prongs: (a) independent, predating evidence - in the ORIGINAL binary func_8004473C forms
    `%lo(D_800A9D08)` in $a0 and reads D_800A9CF8 / D_800A9CFE as `lhu 0($a3)` / `lh 6($a3)`
    with `$a3 = $a0 - 0x10`, and the INCLUDE_ASM sibling func_80044800 forms `%lo(D_800A9D04)`
    in $v1 and reads D_800A9CFA as `$v1 - 0xA`: base+offset addressing of one object from two
    different functions, not symbol adjacency (recorded in s1's evidence.md, before this
    session). (b) the declaration is the documented shape - a flat 0x18-byte header of scalars
    at 0/2/4/6/8/0xC/0x10/0x14, no magic stride index. (c) complete under the 2026-09-03
    amendment: no C code names any merged per-word symbol; the six rows stay in
    undefined_syms_auto.txt suffixed `/* alias of D_800A9CF8+N; retire with func_80044800 */`
    because the still-`INCLUDE_ASM` sibling func_80044800 references D_800A9CFE/D_800A9D00/
    D_800A9D04 in its assembly. (d) spelled at the canonical declaration in include/game.h,
    next to the existing `Unk800F1198Record` merge, never TU-local, never a per-use pointer pun.
    (e) byte-neutrality: the three C siblings rewritten to members (func_80044670,
    func_80044B30, func_80044C70) were re-measured at their prior distance by s1; the full
    `verify-oracle --rebuild` and the layer-2 cheat-reviewer are the driver's/operator's step.
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
