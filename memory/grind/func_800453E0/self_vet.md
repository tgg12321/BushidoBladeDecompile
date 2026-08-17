# SELF-VET — func_800453E0

Session 1 (2026-08-17), modality `recon`. Sandbox `--disable all` == 0 (7 rules
dropped, cheat-asm stripped, 76/76 instructions identical). The diff REMOVES a
cheat (`volatile s32 sp_pad;`) and replaces an asm transcription (goto labels
named after target registers: `L_search`/`L_copy`/`L_clear`/`L_not_found`, locals
named `s0`/`s1`/`s2`/`s3`/`v0`/`v1`/`t0`) with recovered program logic.

CONSTRUCTS: (1) `typedef struct { s16 id; s16 unk2; s32 unk4; s32 amt; void (*fn)(s16,s32); } SubEntry;`; (2) `#define SUBTBL ((SubEntry *)D_800EED10)`; (3) `#define SUBSLOT ((SubEntry *)D_800EED00)`; (4) struct assignment `SUBSLOT[j] = SUBSLOT[j + 1];`; (5) local `s32 off = i << 4;` used as a byte offset into the table; (6) local `s32 next = i + 1;` distinct from the shift-loop counter `j`; (7) local `s32 count` re-read from `D_800A33AC` at the bottom of the loop body; (8) `do { ... } while (i < count);` with the `if (i >= count) return;` entry guard; (9) locals `j`, `last`. REMOVED: `volatile s32 sp_pad;` (a cheat), all register-named locals, all four goto labels.

## T1 semantic purpose: Every construct has one, and every local is READ.
- (1) `SubEntry`: the table genuinely is an array of 16-byte records. The stride
  0x10 and the field offsets 0/2/4/8/0xC are used by five other functions in this
  same file (func_80045294, func_80045510, func_800455AC, and the two setters at
  lines ~1862/1875). Declaring the type is a description of the data, not a
  codegen device.
- (2)/(3): typed views of two declared globals so the records can be indexed and
  assigned. Both symbols pre-exist in the file's extern block (lines 10-14) and
  `D_800EED00` was already referenced by the PREVIOUS version of this function.
- (4): the shift-down copy — the whole point of removing an entry from a packed
  array. Byte-observable (it is target's 4×lw/4×sw block).
- (5) `off`: read twice per iteration (the id compare and the amt load). Live.
- (6) `next`: read twice (the call argument, and `j`'s initializer). Live.
- (7) `count`: read once per iteration as the loop bound. It must be re-read
  because `func_80045294` can change `D_800A33AC`; caching it would be a
  behavioural bug, not an optimization.
- (8): an ordinary hand-rolled loop with its entry guard.
- (9) `j`, `last`: the shift-loop counter and the tail slot index. Both read.
There are ZERO dead stores, ZERO self-assignments, ZERO unused declarations,
ZERO address-of coercions, ZERO padding/frame constructs, ZERO volatile.
Deleting any construct changes what the function DOES, not merely which
registers GCC picks. Notably the function's frame (0x30, `vars= 8`) now comes out
correct with no declaration reserving it — see T5 on `sp_pad`.

## T2 human-programmer: Yes for every construct.
Given the spec "remove the entry with this id from a packed array of records",
a human writes: a record struct, a linear search, an undo call, a shift-down
loop with a struct assignment, a tail clear, a decrement. That is exactly the
body. The three spellings a reviewer might question, answered:
- `off = i << 4` with `(u8 *)SYM + off` instead of `SUBTBL[i].id`: this is the
  house idiom for this table — every other function in this file addresses it
  exactly this way (e.g. line 1798 `*(s16 *)((s32)&D_800EED10 + v1)`, line 1866,
  line 1879, line 1896). Writing it any other way would be the odd choice here.
- `i = 0; count = D_800A33AC; if (i >= count) return;` — the guard is the
  negation of the loop condition, the standard shape for a hand-rolled
  do-while. It is copied verbatim in shape from the ALREADY-MATCHING adjacent
  sibling func_80045510 (src/text1a_c.c:1760-1763), which uses
  `s32 i = 0; s32 count = D_800A33AC; if (i >= count) return;`.
- `SUBSLOT[j] = SUBSLOT[j + 1]` rather than `SUBTBL[j - 1] = SUBTBL[j]`: both are
  the same shift-down and both produce the SAME FINAL LINKED BYTES. See T3/T5.

## T3 GCC-internals justification: NO construct is justified by a GCC internal.
Each construct is explained by program logic first; the codegen consequences
recorded in candidate.c and evidence.md are OBSERVATIONS about which of several
semantically-equivalent spellings the original author used, not the reason any
construct exists. Concretely: I did not add anything to steer GCC — I removed a
transcription and wrote the function. The one place a spelling was chosen
between two equally-natural alternatives is `SUBSLOT[j] = SUBSLOT[j + 1]` vs
`SUBTBL[j - 1] = SUBTBL[j]`, and there the difference is NOT a codegen coercion:
both emit the identical instruction stream, and the identical linked immediate
(`%lo(D_800EED10)` with reloc addend -16 resolves to `%lo(D_800EED00)`; `%hi` is
0x800F either way). The SUBSLOT spelling emits the addend-0 reloc, which is what
target's object encodes and what the scorer reads. That is choosing the
attribution the original source had, not manufacturing bytes.

## T4 permuter/search provenance: NONE. No permuter, no auto-search, no
directed PERM macros, no sweep script was run this session. Every form came from
reading the target asm and the matched sibling func_80045510, and each was
measured with `sandbox --disable all` plus an instruction-level side-by-side
(tmp/grind/func_800453E0/s1/final_diff.txt). The measured floor path was
26 -> 37 -> 21 -> 18 -> 6 -> 3 -> 1 -> 0, each step attributable to one named
semantic change.

## T5 family check: no construct matches any forbidden family, by spelling or
by analogy. Walking the catalog: no register-asm pins; no `__asm__` of any kind
(the function contains none); no regfix/asmfix edits (regfix.txt untouched — the
7 stale rules remain for the operator to delete); no scheduling barriers; no
volatile of any kind; no alias renames (`SUBTBL`/`SUBSLOT` are macros expanding
to casts of two DISTINCT globals at two DISTINCT real addresses under their own
splat names — not a second C handle for one symbol via `asm("Sym")`); no
unused/written-never-read arrays; no dead param assigns; no dead conditional
stores; no empty-body `if`; no `if (1)`; no `do {} while (0)` (the do-while here
is a real loop with a real body and a real back-edge test); no dead-goto label
pads; no DImode chains; no goto-end accumulator; no opaque `s32 one = 1;`; no
redundant width casts; no `bb2.ld` / rodata reorder; no chain-extender.
This diff moves in the ANTI-cheat direction: it DELETES `volatile s32 sp_pad;`,
which was a live member of the forbidden "unused-local frame coercion /
volatile-typed scalar" family. Measured facts about it, recorded because they
matter: with `sp_pad` present cc1 reported `.frame $sp,48 # vars= 8` (target's
frame), and the sandbox's cheat-stripper removed it, dropping the honest build to
frame 40 and contributing 12 of the inherited 26 diffs. The replacement body
reaches `vars= 8` with NO declaration reserving frame bytes — an instance of the
ordinary GCC 2.7.2 phantom-slot artifact documented in
memory/project/phantom-frame-slots-gcc272.md, obtained honestly.

## T6 naming-announces-intent: no coercion-announcing names. The locals are
`i`, `j`, `next`, `last`, `off`, `count` — all role names describing what they
hold. Nothing named `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`,
`tail`, `slack`, `fake`, or `_frame_pad`. The one such name that existed,
`sp_pad`, is DELETED by this diff. The struct field names are `id`, `unk2`,
`unk4`, `amt`, `fn`: `id`/`amt`/`fn` are justified by use (compared against the
search key; negated and passed as the delta to func_80045294; called through at
src/text1a_c.c:1698-1700), and the two fields this function only copies are left
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
