# SELF-VET — file_LoadSectors

DIFF UNDER VET: src/ings.c line 161 — `INCLUDE_ASM("asm/funcs", file_LoadSectors);` replaced
by the pure-C body in memory/grind/file_LoadSectors/candidate.c. No other file in the build
surface is touched (no regfix.txt, no asmfix.txt, no headers, no .ld, no engine/tools).
Measured this session: `sandbox file_LoadSectors --disable all` -> score 0, target_insns 51,
build_insns 51, rules_dropped 0.

CONSTRUCTS: none

The body contains only: four parameters used for their stated purpose, two ordinary locals
(`fd` — the file descriptor, read on every subsequent line; `i` — the loop counter, the `for`
statement's induction variable), five real calls (open / seek / read / close / close-final),
an early-out `return -2` on open failure, an error `return -1` on a short read, and
`return count << 11` (the byte count read). Nothing in it is dead, unread, unwritten,
volatile, aliased, re-stored, pinned, or annotated. There is no `__asm__`, no `register ...
asm("$N")`, no dead local array or scalar, no dead/guarded call, no `do { } while (0)` wrapper,
no self-assign, no pointer alias, no width-coercing cast, no opaque constant holder.

The one thing worth naming explicitly, because three prior sessions spent themselves on it:
the target's frame is 48 bytes while its body never references sp+0..23. Prior sessions
inferred from this that the 1998 source must have declared a DEAD object (a `_pad[2]`-style
local, or a deleted >=5-arg call to inflate the outgoing-args area) and escalated both
readings, which the owner refused (docs/grind/decisions.md:1985). This diff contains NEITHER.
The 8 reserved bytes come from writing the read loop as `for (i = 0; i < count; i++)` instead
of `i = 0; if (count > 0) { do { ... i += 1; } while (i < count); }`: GCC 2.7.2 allocates a
stack temp while expanding the strength-reduced induction variable, `get_frame_size()` counts
it (cc1's own `.frame` comment reports `vars= 8`), and the allocator then keeps the value in a
register so no store is ever emitted — the documented phantom-frame artifact
(memory/project/phantom-frame-slots-gcc272.md, byte-verified in-tree witness `tslLineG5Init`).
No source object corresponds to those bytes, so there is nothing here to classify as a
frame-coercion construct: the loop is the loop.

## T1 semantic purpose
Every construct is load-bearing. Remove the `for` loop and the function no longer reads
anything; remove `fd` and there is no descriptor to seek/read/close; remove `i` and the loop
has no bound; remove either `return` and the error paths vanish. The function's observable
behaviour changes if any line is deleted. There is no construct in the diff whose removal
leaves behaviour byte-identical. PASS.

## T2 human-programmer
Given only the specification ("open file `a0`, seek to sector `sector`, read `count`
2048-byte sectors into `dest`, close, return the byte count; -2 if the open fails, -1 on a
short read"), a programmer writes exactly this, and writes the loop as a `for`. A reader has
no "why is this here?" line to ask about — indeed the `for` spelling is the MORE natural of
the two loop forms, and the previous sessions' hand-unrolled `if (count > 0) { do { ... }
while (...); }` is the one that would draw the question. PASS.

## T3 GCC-internals justification
The program logic is the whole explanation of what the code DOES. GCC internals appear in
this vet only as the post-hoc explanation of WHY the frame is 48 rather than 40 — i.e. as an
account of a compiler artifact of an already-natural construct, not as the mechanism that
motivated writing an otherwise-unmotivated construct. Nothing was added to steer the
allocator, the scheduler, DCE, combine, reorg, or reg_n_refs; no construct in the diff exists
that would be deleted if the compiler behaved differently — the `for` loop stays either way.
PASS.

## T4 permuter/search provenance
No permuter was run this session. The form came from a hand-written 13-variant structural
sweep of ORDINARY, semantically-equivalent spellings of the same function
(tmp/grind/file_LoadSectors/s2/sweep.py — loop shape, narrow-typed locals, named
intermediates, block scoping, real >4-word calls), read out through cc1's `.frame` comment.
The winning variant is not "a spelling that passes the detectors"; it is the plainest
spelling in the sweep, and it would survive any detector because there is nothing to detect.
PASS.

## T5 family check
No forbidden family applies, by shape or by analogy: no register-asm pin, no hardcoded-`$N`
asm, no regfix-style injection, no scheduling barrier, no volatile coercion in any spelling,
no unused local array or scalar, no `&local` / `(void)local` discard, no dead param assign,
no dead conditional store, no empty or always-true `if`, no dead goto/label pad, no DImode
chain, no goto-end accumulator, no alias rename, no redundant width cast, no rodata reorder,
and no dead/deleted call of any arity (the s1 args-area form is banked as REFUSED in
rejected/deleted-call-args-area-owner-refused.c and is NOT what this diff contains). PASS.

## T6 naming-announces-intent
Identifiers are `fd`, `i`, `dest`, `sector`, `count`. None is a `pad` / `dummy` / `unused` /
`spill` / `slack` / `tail` name, and none has discard-only, address-of-only, or
declaration-only uses: `fd` is passed to four calls, `i` is compared and incremented by the
`for` statement, `dest` is read and advanced, `sector` and `count` feed the seek offset, the
loop bound and the return value. PASS.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C and claims no FAKE-family exception.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.
