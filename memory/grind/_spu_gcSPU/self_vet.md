# SELF-VET -- _spu_gcSPU

CONSTRUCTS: one `goto scanned;` (in place of `break;`) exiting phase 1's inner
scan loop, with the label `scanned:` placed immediately after that loop. Nothing
else. No inline asm, no `volatile`, no register pins, no dead locals, no dead
stores, no pads, no arrays, no casts added for codegen, no FAKE annotation, no
scheduling barrier, no `do { } while (0)`. The only two locals are `s32 i` and
`s32 j`, both real induction variables read and written on every path. The body
is a straight transcription of Sony's shipped `libspu/s_m_int.c`
(Xeeynamo/psyz, MIT, PsyQ 4.0 -- the same library version BB2 links).

## T1 semantic purpose
`goto scanned;` is the scan loop's *exit*, not an addition to it: the loop
`for (j = i + 1;; j++)` has no controlling condition, so removing the jump makes
the function loop forever. It has the same observable effect as the `break` it
replaces, which is the point -- this is a choice between two ordinary C spellings
of one control-flow edge, not an extra construct layered on top of a simpler
form. There is no simpler form: the loop needs an exit and this is it.

## T2 human-programmer
Yes. "Scan forward past the sentinels, then continue with the entry you landed
on" is naturally written either way, and a `goto` past the end of a scan loop is
everyday C in this era's systems code (and in this repo). A reader does not ask
"why is this here?" -- they ask "where does it go?", and the label is on the next
line. Nothing in the diff is semantically inert or unexplainable from the
function's specification.

## T3 GCC-internals justification
The *reason I chose `goto` over `break`* is a compiler-behaviour observation and
I state it plainly: `stmt.c:expand_end_loop` rolls a leading conditional exit to
the bottom of the loop only when the exit jumps to that loop's own `end_label`
(which is what `break` compiles to), and the rotation then feeds
`jump.c:2163 duplicate_loop_exit_test`, peeling a guard copy. But the construct
does not *depend* on any GCC internal to be meaningful: it is a normal loop exit
whose program logic stands on its own, and it carries no name, no dummy value,
and no unused declaration whose only explanation is codegen. Both spellings are
things a programmer writes for their own sake; I picked the one the original
author demonstrably picked. This is not a lever inserted to move an allocator.

## T4 permuter/search provenance
No permuter, no automated search, no campaign was run in this session. The body
came from the version-correct Sony source (psyz `s_m_int.c`) and the one-edge
change came from reading `tools/gcc-2.7.2/stmt.c` and `tools/gcc-2.7.2/jump.c`
directly. It does not pass detectors by virtue of an unusual spelling -- there is
nothing for a detector to catch.

## T5 family check
It matches no forbidden family, by analogy or otherwise: it is not a register
pin, not asm, not a scheduling barrier, not volatile coercion in any spelling,
not a frame-coercing local or array, not a dead param assign, not a dead
conditional store, not an empty `if (cond) { }`, not `if (1) { ... }`, not a
dead-goto *label pad* (the label is a live branch target reached on the only
exit from the loop, and the jump is the loop's only exit -- nothing here is
dead), not a DImode chain, not an opaque constant variable, not an alias rename,
not a redundant cast, not a linker-script reorder. No construct in the diff is
inert: delete any line and the function either stops terminating or stops doing
what `_spu_gcSPU` specifies.

## T6 naming-announces-intent
The only name introduced is the label `scanned`, which describes the program
state at that point (the forward scan is finished). It is not `pad`, `dummy`,
`unused`, `spill`, `slack`, `tail` or any other coercion-announcing name, and it
is a real branch target, not a discard or an address-of.

SANCTIONED-FAMILY-CLAIMS: none -- no /* FAKE */ construct is present and no
sanctioned-family carve-out is being invoked. The diff is ordinary C.

ANNOTATION-CONFORMANCE: n/a -- no FAKE construct.

## Out-of-scope build input (declared, not hidden)
Reaching 0 also requires appending the single line `_spu_gcSPU` to
`maspsx_label_nop_funcs.txt`. That is not a C construct and not a cheat: it
turns on maspsx's per-function correction for the documented `.L`-label
load-delay blind spot (`is_label()` matches only `$L`-prefix locals while this
GCC fork emits `.L`), restoring two `nop`s that the ORIGINAL binary genuinely
contains. It is the retirement path `.claude/rules/maspsx-label-nop-gate.md`
mandates ("a pure-C RETIREMENT path, not a park"), and two siblings in this same
TU -- `SpuFree` and `_spu_init` -- are already listed in that file. The
alternative (a source `__asm__("nop")`) is exactly the cheat the rule exists to
retire. Because that file sits outside the one-build-input candidate scope, this
session returns `owner-gated` as an integration handoff rather than
`candidate-ready`.
