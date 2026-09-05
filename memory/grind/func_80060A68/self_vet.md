# SELF-VET — func_80060A68

Session s29 (synthesis).  Diff = src/text1b.c only: the single line
`INCLUDE_ASM("asm/funcs", func_80060A68);` is replaced by the body saved at
`memory/grind/func_80060A68/candidate.c`.  No other file in the tree is touched, no
header is edited, no pipeline `*.txt` is edited.

Measurements this session, with those edits in place in src/:
  `sandbox func_80060A68 --disable all` -> score 0, build_insns 66, target_insns 66.
  `verify-oracle` -> ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa,
  original_sha1_locked identical, build_matches true.

CONSTRUCTS:
  C1 `extern struct Ob *D_800A3468;` — the object handle is declared with the pointer
     type src/text1b.c already assigns into it (:3361 scratchpad base; :3452, :3469,
     :3503 a callee's returned pointer).
  C2 `struct Ob { union { s32 w; u16 h; } id; s32 u04; s32 u08; s32 *p0C; u16 *p10;
     s8 *p14; u16 m18; u16 m1A; u16 m1C; u16 u1E; s32 m20; s32 m24; s32 m28; };` — a
     block-scope declaration of the shape of the object that handle points at.  Offset 0
     is a union because src/text1b.c:3479 writes that whole 32-bit word as one constant
     (0x210009) while this function reads it both whole (the 0x200000 test) and as its
     low halfword (the character index).
  C3 Every access to that object written as a member reference (`->m20`, `->p0C[0]`,
     `->p10[1]`, `->id.h`, `->id.w`, `*...->p14`) rather than as arithmetic through an
     integer cast.
  C4 `extern u8 D_8009BA60[]; extern s32 D_800F10D0[]; extern s32
     chractar_use_pset_combo_id_table[];` plus three subscript reads — the three tables
     declared as the arrays the naming census already documents them to be (24-entry
     flag table; per-index offset table; per-character combo-id table).
  C5 `((s32 (*)(void)) chractar_use_pset_combo_id_table[...])()` — the dispatch call
     through the selected table entry, spelled `()` per src/text1b.c:3174 and the callee
     prototype at :4042.
  C6 `s32 result;` — one local, holding the value the dispatch call returns.

## T1 semantic purpose
Per construct, each has an observable job in what the function DOES, and deleting it
does not leave a simpler form producing the same behaviour — it leaves the same
operations respelled less informatively.  C1/C2/C3: the function's entire job is to read
and write named members of one object reached through a handle; a type declaration is
what names them, and every member the struct declares that this function touches is read
or written by a statement in the body.  C4: the three table reads ARE the function's data
flow (character index -> per-index offset, plus per-index flag -> combo-id entry).  C5:
the call is the function's only side-effecting call.  C6: `result` is the value the call
produces and the value the following byte store consumes — one write, one read, both real.
Members declared but not touched here (`u04`, `u08`, `u1E`) exist because the object's
layout is what it is; they emit no code, exactly as any struct member the current function
does not use emits none.

## T2 human-programmer
Yes, for every construct.  Given only the specification — "the current character object is
reached through a global handle; clear its flag-table slot, copy three words and three
halfwords into it from two sub-objects it points at, publish two of its interior locations
into two scalars, dispatch through the per-character combo table, store the returned byte
into the object, and raise a flag if the object's status word has bit 21 set" — a
programmer writes a struct, `->` for each member, arrays for the tables, and a cast for
the dispatch entry.  No line in this body invites the reader question "why is this here?"
on semantic grounds.  The one spelling a reader might query is the union, and the answer is
in the same file: one word, written whole at :3479, read at two widths here.

## T3 GCC-internals justification
No construct in this diff is justified by a compiler pass.  The justification for C1/C2/C3
is the program's object model, evidenced independently of codegen by the assignments and
stores in src/text1b.c listed under C1/C2 above; for C4 it is the naming census entries for
the three tables; for C5 the callee prototype; for C6 the call's return value.  The
candidate.c header comment does contain a pass-level explanation of WHY this shape happens
to land on the target's instruction stream (cse memory-table invalidation at stores through
a pointer; MEM_IN_STRUCT_P in `true_dependence`).  That paragraph is an explanation of an
observation written for the next reader, and it is severable: deleting it changes no C, no
declaration, and no byte.  Nothing in the body exists that the object model does not
already require.

## T4 permuter/search provenance
None.  No permuter ran this session and no construct here came from a search.  The shape
was derived by reading the sibling call sites in src/text1b.c that establish the handle's
pointer type, and by reading the target's tail, which loads the handle twice more after
the `jalr` — so no source-level variable can be holding it across the call.  The competing
spellings that were measured and rejected are banked in
`memory/grind/func_80060A68/rejected/s29-*.c` with their scores in their names.

## T5 family check
No forbidden family is matched, by shape or by analogy.  This body contains no register-asm
pin, no `__asm__` of any kind, no scheduling barrier, no `volatile`, no `asm("sym")` rename,
no local array, no pad, no dead store, no self-assignment, no constant holder, no dead
conditional, no empty-body `if`, no `if (1)`, no `do { } while (0)`, no `goto`, no label,
no duplicated statement across arms, no staged local, no named intermediate, and no local
written more than once.  The body declares exactly one local (C6), so none of the five
constructs the Judge has banned for this function — every one of which is a scratch local
carrying a staged value, under the names `src`, `temp2`, `temp_a1`, `p10` or `cp` — is
expressible in it.  The sixth ban, on a blanket self-vet claim standing in for analysis, is
answered by this vet reasoning per construct, C1 through C6, above.

Two adjacent families a reviewer may reach for, and why neither is claimed:
  * per-word splat symbol -> aggregate merge (`.claude/rules/no-new-park-categories.md:238`).
    That family merges two or more splat-invented per-word scalars into one aggregate
    declaration.  Nothing is merged here: `D_800A3468` stays exactly one symbol at exactly
    one location and simply carries the pointer type the file's own assignments give it,
    and the object it points at is dynamically assigned (scratchpad at :3361, callee return
    at :3452/:3469/:3503) so it owns no splat symbols at all.  C4's three arrays likewise
    rename, add and remove no symbol; each declares the extent of a table the census already
    describes as a table, and the target's own indexed loads carry the stride.
  * type-level `volatile` families.  Not applicable — there is no `volatile` in this diff,
    and no allowlist grant is needed or requested.

## T6 naming-announces-intent
No name here announces coercion intent.  The sole local is `result`, named for the value it
holds.  The struct members are named for their offsets where their meaning is not yet
established (`u04`, `u08`, `p0C`, `p10`, `p14`, `m18`, `m1A`, `m1C`, `u1E`, `m20`, `m24`,
`m28`), with a `p` prefix where the member is a pointer and `m` where it is a copied
member, and `id` where the meaning is established by the file.  That is this project's
ordinary vocabulary for undocumented members.  The strings `pad`, `_pad`, `dummy`, `unused`,
`spill`, `sp_`, `_buf`, `tail`, `slack` and `_frame_pad` appear nowhere in the diff.

SANCTIONED-FAMILY-CLAIMS: none — this body is ordinary C.  It claims no sanctioned
exception family, so there is no scope sentence to quote and no precedent to cite.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.  No family that mandates a `/* FAKE */`
annotation is claimed or present; every construct listed under CONSTRUCTS is an ordinary
declaration, member reference, array subscript, call or return-value local.
