# SELF-VET — func_800453E0

Session 2 (2026-08-17), modality `recon`. Sandbox `--disable all` == 0 (7 rules
dropped, cheat-asm stripped, 76/76 instructions, frame 0x30). The diff REMOVES a
cheat (`volatile s32 sp_pad;`) and replaces an asm transcription (goto labels
named after target registers: `L_search`/`L_copy`/`L_clear`/`L_not_found`, locals
named `s0`/`s1`/`s2`/`s3`/`v0`/`v1`/`t0`) with recovered program logic.

Relative to session 1's bounced submission this body is STRICTLY SIMPLER: the
do-while + `count` re-read + entry-guard scaffold and the `s32 next;` local are
both GONE, each after being measured byte-neutral from the zero baseline
(evidence.md facts 8 and 9). The construct that carried session 1's banned
citation (`.claude/rules/loop-counter-fills-load-delay.md`, an unresolvable path)
is not in this diff and that citation appears nowhere in this vet.

CONSTRUCTS: (1) `typedef struct { s16 id; s16 unk2; s32 unk4; s32 amt; void (*fn)(s16,s32); } SubEntry;`; (2) `#define SUBSLOT ((SubEntry *)D_800EED00)`; (3) struct assignment `SUBSLOT[j] = SUBSLOT[j + 1];`; (4) local `s32 off = i << 4;` used as a byte offset into the table; (5) locals `i`, `j`, `last`; (6) `for (i = 0; i < D_800A33AC; i++)` outer loop; (7) `if (i < D_800A33AC - 1)` guard around the shift loop. REMOVED by this diff: `volatile s32 sp_pad;` (a cheat), all seven register-named locals, all four goto labels.

## T1 semantic purpose: Every construct has one, and every local is READ.
- (1) `SubEntry`: the table genuinely is an array of 16-byte records. The stride
  0x10 and the field offsets 0/2/4/8/0xC are used by five other functions in this
  same file (func_80045294, func_80045510, func_800455AC, and the two setters
  near the end). Declaring the type is a description of the data, not a codegen
  device.
- (2) `SUBSLOT`: a typed view of a declared global so the records can be indexed
  and assigned. `D_800EED00` pre-exists in the file's extern block (line 14) and
  was already referenced by the PREVIOUS version of this function.
- (3): the shift-down copy — the whole point of removing an entry from a packed
  array. Byte-observable (it is target's 4×lw/4×sw block).
- (4) `off`: read twice per iteration (the id compare and the amt load). Live.
- (5) `j`, `last`: the shift-loop counter and the tail slot index. Both read.
  `i` is the search index, read four times.
- (6): an ordinary counted search loop.
- (7): the guard that skips the shift when the removed entry is the last one.
ZERO dead stores, ZERO self-assignments, ZERO unused declarations, ZERO
address-of coercions, ZERO padding/frame constructs, ZERO volatile, ZERO inline
asm. Deleting any construct changes what the function DOES, not merely which
registers GCC picks. Notably the function's frame (0x30, `vars= 8`) comes out
correct with no declaration reserving it — see T5 on `sp_pad`.

## T2 human-programmer: Yes for every construct.
Given the spec "remove the entry with this id from a packed array of records", a
human writes: a record struct, a counted linear search, an undo call, a
shift-down loop with a struct assignment, a tail clear, a decrement. That is
exactly the body, and it is now the shortest spelling of it that this session
could find — every simplification tested was either taken (the loop scaffold,
`next`) or measurably changed the emitted code (the byte-offset accesses). The
one spelling a reviewer might question, answered:
- `off = i << 4` with `(u8 *)SYM + off` instead of `SUBSLOT[i + 1].id`: this is
  the house idiom for this table — every other function in this file addresses
  it exactly this way (e.g. `src/text1a_c.c:1767` `*(s16 *)((s32)&D_800EED10 + v1)`,
  and the setters near the end of the file). Writing it any other way would be
  the odd choice here. It is also not free: the struct-indexed spelling emits a
  different, shorter instruction stream (72 insns vs target's 76), so the two are
  not interchangeable descriptions of the same code.
- `SUBSLOT[j] = SUBSLOT[j + 1]` rather than a D_800EED10-based `[j-1] = [j]`:
  both are the same shift-down and both produce the SAME FINAL LINKED BYTES. See
  T3/T5.

## T3 GCC-internals justification: NO construct is justified by a GCC internal.
Each construct is explained by program logic first; the codegen consequences
recorded in candidate.c and evidence.md are OBSERVATIONS about which of several
semantically-equivalent spellings the original author used, not the reason any
construct exists. Concretely: I did not add anything to steer GCC — session 1
removed a transcription and wrote the function, and session 2 SUBTRACTED from
that body every construct whose removal was byte-neutral. The one place a
spelling was chosen between two equally-natural alternatives is the SUBSLOT base
for the shift loop, and there the difference is NOT a codegen coercion: both
emit the identical instruction stream and the identical linked immediate
(`%lo(D_800EED10)` with reloc addend -16 resolves to `%lo(D_800EED00)`; `%hi` is
0x800F either way). The SUBSLOT spelling emits the addend-0 reloc, which is what
target's object encodes and what the scorer reads. That is choosing the
attribution the original source had, not manufacturing bytes.

## T4 permuter/search provenance: NONE. No permuter, no auto-search, no directed
PERM macros, no sweep script was run in either session. Every form came from
reading the target asm and the matched siblings in the same file, and each was
measured with `sandbox --disable all` plus object-level disassembly diffs
(`tmp/grind/func_800453E0/s2/{forloop,dowhile,final}_func.txt`). Session 2's two
removals were adversarial probes against session 1's OWN claims, with a live
control probe (`i + 1` -> `i + 2` moved the score 0 -> 1) confirming the builds
were fresh rather than stale artifacts.

## T5 family check: no construct matches any forbidden family, by spelling or by
analogy. Walking the catalog: no register-asm pins; no `__asm__` of any kind
(the function contains none); no regfix/asmfix edits (regfix.txt untouched — the
7 stale rules remain for the operator to delete); no scheduling barriers; no
volatile of any kind; no alias renames (`SUBSLOT` is a macro expanding to a cast
of a global under its own splat name — not a second C handle for one symbol via
`asm("Sym")`); no unused/written-never-read arrays; no dead param assigns; no
dead conditional stores; no empty-body `if`; no `if (1)`; no `do {} while (0)`
(there is no do-while in this body at all any more); no dead-goto label pads; no
DImode chains; no goto-end accumulator; no opaque `s32 one = 1;`; no redundant
width casts; no `bb2.ld` / rodata reorder; no chain-extender.
This diff moves in the ANTI-cheat direction: it DELETES `volatile s32 sp_pad;`,
which was a live member of the forbidden "unused-local frame coercion /
volatile-typed scalar" family. Measured facts about it, recorded because they
matter: with `sp_pad` present cc1 reported `.frame $sp,48 # vars= 8` (target's
frame), and the sandbox's cheat-stripper removed it, dropping the honest build to
frame 40 and contributing 12 of the inherited 26 diffs. The replacement body
reaches `vars= 8` with NO declaration reserving frame bytes — an instance of the
ordinary GCC 2.7.2 phantom-slot artifact documented in
memory/project/phantom-frame-slots-gcc272.md, obtained honestly.

## T6 naming-announces-intent: no coercion-announcing names. The locals are `i`,
`j`, `last`, `off` — all role names describing what they hold. Nothing named
`pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`,
`fake`, or `_frame_pad`. The one such name that existed, `sp_pad`, is DELETED by
this diff. The struct field names are `id`, `unk2`, `unk4`, `amt`, `fn`:
`id`/`amt`/`fn` are justified by use (compared against the search key; negated
and passed as the delta to func_80045294; called through at
src/text1a_c.c:1697-1700), and the two fields this function only copies are left
deliberately as `unk2`/`unk4` rather than guessed, per
memory/feedback/names-require-evidence.md.

SANCTIONED-FAMILY-CLAIMS: none. This diff claims NO carve-out and NO exception.
It contains no FAKE-eligible construct, no dead code, and no coercion; it is
ordinary program logic plus the deletion of one pre-existing cheat. (Stated
explicitly so the reviewer does not need to hunt for an unstated claim: if any
construct above were judged to need a family, I would not have it, and the
correct outcome would have been `ruling-request`.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. There is no `/* FAKE */`
annotation in the diff because there is nothing in the diff that requires one.
