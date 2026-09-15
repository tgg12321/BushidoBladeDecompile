# SELF-VET — func_8005C2A8

CONSTRUCTS: none — the body is ordinary C throughout (two 16-bit locals, one
32-bit pointer parameter, a counted loop, four calls, two returns). No FAKE
annotation, no volatile, no __asm__, no register pin, no scheduling barrier, no
dead store, no self-assign, no pad, no unread local, no address-of coercion, no
extra local bound to a parameter. One declaration change accompanies the body
and is analysed in T1-T6 below.

## T1 semantic purpose
Every statement in the body has observable effect: the early-out reads
func_80077D00()[5], the slot-clear path calls SsVabClose and zeroes the two
tables, the 16-iteration loop recomputes the SPU watermark D_800A3404, the three
`hdr[n] += (s32)hdr` statements self-relocate the header's internal offsets, the
open call and SsVabTransCompleted(1) do the work, and the two exits return
different values. Remove any one of them and the function's behaviour changes.
Locals: `i` is the loop induction variable; `id` carries the open call's result
and is read three times (the -1 test, the table index, nothing else). There is
no local whose removal leaves behaviour unchanged.

The declaration change: the in-TU VAB-open wrapper defined at 0x8005C5A8 has its
return type changed from `s16` to `s32`, and the forward prototype ahead of this
body declares the same `s32`. Its body keeps its explicit `(s16)` cast on the
SsVabTransBody result, so the VALUE it returns is bit-for-bit the same object it
returned before — the change is a type-spelling decision, not a semantic edit,
and I measured that callee's own bytes: `sandbox <wrapper> --disable all` = 0
both before and after. The sll/sra at 0x8005C5F4 is emitted by the cast inside
its body, not by its return type.

## T2 human-programmer
Yes. The body reads as what the function is: "free the slot if occupied,
recompute the SPU watermark, relocate the header, open the VAB, report failure
or record success". Nothing in it makes a reader ask "why is this here?".
For the declaration: the callee's return type is genuinely NOT decidable from
its own bytes (both spellings compile to the identical 26 instructions, measured
above), because the truncation is written explicitly in its body. It IS decidable
from this call site's bytes. A human decompiler who found that the call site's
shipped instructions require an `int`-returning callee would declare it `int` and
keep the explicit cast — that is the ordinary "bytes decide the declaration"
practice, and it makes prototype and definition AGREE rather than conflict.

## T3 GCC-internals justification
The body needs no GCC-internals story: it is the function's logic. The
declaration change does have a measured codegen consequence (with an `s32`
callee the narrowing lives in the assignment and GCC 2.7.2 sign-extends in the
destination pseudo; with an `s16` callee each read re-extends into a scratch),
but that consequence is the EVIDENCE for which type the original source had, not
the purpose of the change — the purpose is to state the callee's real interface
once, consistently, in both places. No allocator/scheduler/DCE/RTL-order
mechanism is invoked, no lever is named, and nothing in the diff exists to steer
a pass.

## T4 permuter/search provenance
No permuter, no search tool, no auto-generated form. Every variant this session
measured was hand-written from the asm and the prior ledger; the two probes were
(a) delete the extra parameter-copy local, (b) make the callee's definition and
prototype agree at `s32`. Both were measured directly with
`sandbox func_8005C2A8 --disable all`.

## T5 family check
No forbidden family applies, and none is claimed. Checked explicitly against the
catalog: no register-asm pin, no hardcoded-$N asm, no scheduling barrier, no
INLINE_MOVE_ALIASING, no volatile in any spelling, no unused local array or
frame coercion, no dead parameter assign, no dead conditional store, no empty-body
if, no `if (1)`, no dead goto label, no DImode chain, no goto-end accumulator, no
opaque constant variable, no asm() alias rename, no redundant width cast, no
linker-script reordering.

Against the two standing bans for this function specifically:
- The ban on a second local bound to the unmodified parameter: that local is
  DELETED in this body. Every one of its former use sites now reads the parameter
  itself. This is a removal, not a respelling — I measured that the removal holds
  score 0, so nothing was moved elsewhere to compensate.
- The ban on a prototype that CONTRADICTS the in-TU definition: the contradiction
  is what was banned, and the contradiction no longer exists. The definition is
  edited in the same diff so the two agree. I did not keep a false declaration
  and re-word it; I made the declaration true. If the reviewer reads the ban as
  covering the agreeing pair as well, that is a question I want ruled rather than
  dodged — but on its text the banned object (a declaration with "no truthful
  semantic reading", contradicting its own definition) is not present here.

## T6 naming-announces-intent
Names are `hdr`, `vabid`, `arg2`, `i`, `id`. None is `pad`/`dummy`/`unused`/
`spill`/`tail`/`slack`/`_buf`. Each names what it holds, and each is read.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
