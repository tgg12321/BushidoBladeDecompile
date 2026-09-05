# SELF-VET — func_80022F34   (grind s10, rederive, 2026-09-05)

Scope note: this session's outcome is `owner-gated` (INTEGRATION HANDOFF), not
`candidate-ready`, because two of the three required edits are outside a grind
session's allowed surface. The vet is written anyway because the banked form IS
the byte-verified COMPLETED-C body and a fresh layer-2 cheat-reviewer will rule
on it when the operator/driver lands it.

CONSTRUCTS: none (no construct from any forbidden or sanctioned-exception family
appears in the diff). The diff consists of: two `extern` DECLARATION corrections
in include/code6cac.h (scalar -> array, matching the object model the target's
own addressing proves), one line in maspsx_label_nop_funcs.txt (the documented
assembler-fidelity gate), and one ordinary C function body replacing an
INCLUDE_ASM line.

## T1 semantic purpose
Every statement in the body has an observable effect. `rec`, `mode`, `idx1`,
`idx2`, `val1`, `tbl`, `i`, `offset` are all read and all feed either a store or
the call. `val1` is a named intermediate ONLY because C evaluation order in the
call argument list would otherwise not express "read idx1 from the record BEFORE
following the record's next-pointer" — the pointer reload `rec = *(u8 **)rec;`
destroys `rec`, so the first status-up value genuinely must be read first. It is
a real, load-bearing sequencing local, not a codegen holder. Removing it changes
the program's meaning, not just its bytes. Nothing else is dead: there is no
self-assign, no dead store, no unused local, no pad, no address-of discard.

## T2 human-programmer
Yes throughout. "For each of the two active fighters: if the record is enabled,
set its display value from one of three sources depending on the voice/audio
mode, then push this fighter's and its opponent's status-up entries to the HUD"
is exactly this code. A human writing it from the spec would index a table of
20-byte records as `tbl[idx][0]` and would hold the first value in a local
across the pointer hop. Nobody reading it would ask "why is this here?" of any
line. The declaration changes are what a human would write on discovering that
the symbol is indexed by a runtime value.

## T3 GCC-internals justification
No construct in the diff is justified by a GCC internal. The GCC mechanism
(expand's ARRAY_REF path vs break_out_memory_refs) is recorded in the ledger as
the EXPLANATION for why nine prior sessions were stuck at 11 — it is not the
reason any line is written the way it is. The body is written the way the
program means, and the correct declaration is written because the object model
says so. Test the inverse: if GCC 2.7.2 had produced identical bytes from the
old scalar-pun spelling, this body would still be the better source, because
`D_801027BC[idx][0]` states the object model and `(&D_801027BC)[idx * 5]` hides
it. There is no lever, no barrier, no pin, no coercion, no annotation.

## T4 permuter / search provenance
None. No permuter output is in this form. The permuter axis was killed in s4/s5
and was not re-run. The form came from re-deriving the object model out of the
target's own addressing (`lui %hi(D_801027BC); addu $at,$at,idx*20;
lw %lo(D_801027BC)($at)` twice => an array of 20-byte records, indexed by a
runtime value) — i.e. from reading the program, which is the rederive modality's
whole point.

## T5 family check
No forbidden family matches, by shape or by analogy:
 - not a register pin, not `__asm__`, not a scheduling barrier, not volatile in
   any spelling, not an alias rename;
 - no unused/written-never-read local, no local array at all, no `(void)&x`;
 - no dead param assign, no dead conditional store, no empty-body `if`, no
   `if (1)`, no dead goto label, no DImode chain, no opaque constant holder;
 - no `do { } while (0)` wrap (the loop is a real 2-iteration loop);
 - no redundant width cast; no build-time assembly rewriting; no linker reorder.
The two declaration edits are the OPPOSITE of the banned per-use pun: the brief's
own INDEXED-ACCESS signal states that a per-use `*(&D_80102782 + i)` /
`(&D_801027BC)[idx * 5]` pun is a banned construct and that the sanctioned fix is
to declare the symbol as the array it is. This diff removes both puns; the
previously banked candidate carried them.
The maspsx_label_nop_funcs.txt line is not a source construct at all: it is the
per-function assembler-fidelity gate documented in
.claude/rules/maspsx-label-nop-gate.md, whose stated purpose is to make maspsx
emit a load-delay nop it drops only because of a `.L`-vs-`$L` prefix blind spot.
It replaces nothing in the C; the C contains no compensator.

## T6 naming-announces-intent
`rec`, `mode`, `tbl`, `idx1`, `idx2`, `val1`, `i`, `offset`. All descriptive of
the value they hold. No `pad`, `dummy`, `unused`, `spill`, `tmp`, `slack`,
`_buf`, `tail`, `sp_*`. Every one is read.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and claims no exception
family. (Consequently no FAKE annotation is owed; see below.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. This is not the "n/a on a
family that mandates FAKE" failure mode: no family is claimed, because the diff
contains no construct from any exception family. The body compiles to the
target's bytes as plain C over correctly-declared objects.
