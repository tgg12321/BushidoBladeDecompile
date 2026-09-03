# SELF-VET — func_80033550

Diff surface: `src/code6cac_b.c` only. `INCLUDE_ASM("asm/funcs", func_80033550);`
replaced by a file-scope `typedef struct { s32 word0; s32 word1; s32 word2; } Word3;`
(plus a two-line explanatory comment) and the function body. No header change, no
`.claude/rules/`, `engine/`, `tools/`, `Makefile` or `*.ld` touched. No inline asm, no
register pins, no scheduling barriers, no `volatile`, no `/* FAKE */` construct.

CONSTRUCTS: (1) file-scope `typedef struct {s32 word0; s32 word1; s32 word2;} Word3;`
(2) `for (i = 0; i < 6; i++) { if (flagbyte == 0) break; }` search loop
(3) `if (i == 6) return;` early exit
(4) `*(&D_800A3918 + i) = 1;` flag store
(5) `*(Word3 *)((u8 *)&D_80107850 + i * 12) = *(Word3 *)arg0;` — 12-byte struct assignment

## T1 semantic purpose
(1) The typedef names the 12-byte record type the function copies; it is the type of
statement (5)'s operands and cannot be removed without removing the copy.
(2)-(4) are the function's logic: find the first free slot in the 6-entry flag table,
bail out when the table is full, mark the slot used.
(5) IS the observable effect of the function — it copies 12 bytes from the caller's
record into slot `i` of the table. Deleting it deletes the function's output.
No construct in the diff is byte-neutral-but-present: every statement writes memory
that the caller can observe, and the typedef is load-bearing for the statement that
does. Nothing here exists "to move a register".

## T2 human-programmer
Yes, straightforwardly. Given the spec "find a free slot in a 6-entry table, mark it
in use, store the caller's 3-word record there", a human writes exactly this: a linear
search with `break`, a full-table early return, a flag store, and a record assignment.
The only thing a reader might ask about is the pointer arithmetic + cast in (5), and
the answer is ordinary decomp hygiene: splat exported the table as scalar
`extern s32 D_80107850`, so the record type is applied at the use site rather than by
re-typing a shared header symbol. (Re-typing the header would be the aggregate-merge
family and would need base-register/stride evidence — deliberately NOT done.)

## T3 GCC-internals justification
The CONSTRUCT is not justified by GCC internals — statement (5) is justified by the
function's semantics (copy the record). GCC internals appear only in the ledger's
EXPLANATION of why this spelling reaches the target bytes when three scalar
load/store pairs did not (the MIPS block-move expansion keeps the source address
register live across all three loads, so hard_reg_conflicts[72] becomes {2,3,4,5,6}
and find_reg's pass-0 scan lands on $a3). That is an after-the-fact mechanism
account of an ordinary C statement, not a construct whose only reason for existing is
a compiler pass. Remove the mechanism story and statement (5) is still exactly the C
you would write; remove statement (5) and the function no longer does its job. There
is no "lever" here, nothing named `pad`/`dummy`/`_spill`, and no construct that a
simpler C form could replace while keeping the semantics.

## T4 permuter/search provenance
None. No permuter run in this session. The form came from reading the target's
instruction pattern (three `lw` from one base followed by three `sw` to consecutive
offsets of one symbol) and recognising it as a 12-byte aggregate copy, then measuring
the resulting `find_reg` inputs with `BB2_FINDREG_DEBUG=72`. It is not a spelling that
passes detectors by accident; it would pass a reviewer who had never heard of the
detectors.

## T5 family check
No forbidden family applies, and no sanctioned-exception family is claimed. Checked
against the catalog line by line: no register-asm pin, no hardcoded-`$N` asm, no
scheduling barrier, no aliasing move, no volatile coercion in any spelling, no unused
local array or frame coercion, no dead param assign, no dead conditional store, no
empty-body `if`, no `if (1)`, no dead goto label pad, no DImode chain, no goto-end
accumulator, no param-local alias declaration-order trick, no opaque `s32 one = 1;`,
no lowercase `asm(...)`, no build-time asm rewriting, no `asm("sym")` rename, no
redundant width cast, no linker-script rodata reorder. The diff contains only
statements with observable effects. In particular the previous ledger frontier
(manufacturing a byte-free register occupant via dead locals / duplicated arms /
chain-extenders) was ABANDONED, not respelled — the conflict set arrives from the
real semantics of an aggregate copy.

## T6 naming-announces-intent
`Word3`, `word0/word1/word2`, `i`, `arg0`. `Word3` is a neutral structural name for a
three-word record (deliberately not a semantic claim — [[names-require-evidence]]);
its members are read and written by the copy. No `pad`, `dummy`, `unused`, `spill`,
`slack`, `tail`, `_buf`, `_frame_pad`. No declaration whose only uses are discards,
address-of, or nothing.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C and claims no exception
family. (Consequently no `/* FAKE */` annotation is required or present; the
prior candidate's sanctioned `do { ... } while (0)` wrap was DELETED by this form.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.

## Verification actually run this session (not claimed, measured)
- `& tools/wteng.ps1 main sandbox func_80033550 --disable all` ->
  `"score": 0, "target_insns": 34, "build_insns": 34, "rules_dropped": 0`
- `& tools/wteng.ps1 main build` (full clean-driver build) ->
  `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, `MATCH`
- `& tools/wteng.ps1 main verify-oracle` -> `"build_matches": true`
- Linked-ELF disassembly of 0x80033550..0x800335D4 compared word-for-word against
  `asm/funcs/func_80033550.s`: all 34 instructions identical
  (`tmp/grind/func_80033550/s16/linked_disasm.txt`).
