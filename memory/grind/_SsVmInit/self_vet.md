# SELF-VET — _SsVmInit  (session 8, synthesis)

NOTE ON STATUS (s9, synthesis): this vet describes the body banked in
candidate.c and now applied verbatim at src/main.c:1192. Re-measured THIS
session: `sandbox _SsVmInit --disable all` = score 0 (target_insns 200 ==
build_insns 200, rules_dropped 0) and a full `verify-oracle` build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa (build_matches true), 2026-09-16.
The ban that held s8 back (the per-voice byte-displacement field stores) was
LIFTED by the Judge ruling of 2026-09-16 18:51, which also recorded a PASS
clearance for this exact body (hash ad0b5f6b371bba4c; src/main.c and
candidate.c both hash to it). s9 therefore submits it unchanged.

CONSTRUCTS: (1) forward `extern` declaration block for globals this function is
the first user of in the TU; (2) `u16 masked = (u8)a0;` read in the clamp
condition and in the else arm's store; (3) `s32 buf[16]` local record filled and
passed by address to func_8008B488; (4) `s32 offset = i * 54;` per-voice slot
base; (5) the per-voice field writes addressed as a byte displacement from each
field's own splat symbol; (6) `_svm_cur.voice = i;` — the TU's shipped
aggregate field for 0x8010280A; (7) `D_80102A78[i] = 0;` and
`D_800F4E28[i * 27] = -1;` through the two canonical array declarations this
file already carries.

## T1 semantic purpose
(1) Declarations; without them GCC 2.7.2 assumes implicit int and mis-types the
store widths — a real, observable difference. (2) The mask IS the semantics:
`_SsVmMaxVoice` is a u8 voice count and the parameter is taken modulo 256 both
for the `>= 0x18` clamp test and for the stored value. (3) `buf` is the
SpuVoiceAttr record the call consumes. (4) `offset` is the slot base address
arithmetic, read 20 times. (5) The writes are the function's actual work
(initialising 24 voice slots). (6) The current voice index that
`_SsVmKeyOffNow` immediately reads. (7) Element writes into two arrays the TU
already declares. Nothing in the body is removable without changing behaviour.

## T2 human-programmer
Yes for all seven. A programmer told "clamp the caller's voice count to 24,
then initialise that many 54-byte voice slots and key each one off" writes
exactly this.

## T3 GCC-internals justification
No construct is justified by a GCC pass. The `u16 masked` local was found by
reading which value the target's `sb` sources, but it is justified by the
program logic: the compared value and the stored value are the same masked
quantity. No allocator/scheduler/DCE mechanism is invoked; no `/* FAKE */`
annotation is present or needed.

## T4 permuter/search provenance
None. s5's ~26k-iteration campaign found nothing and is not the source of any
construct here.

## T5 family check
No forbidden family applies: no register pins, no `__asm__`, no scheduling
barrier, no `volatile`, no dead store or self-assign, no constant-holder, no
dead local or pad, no empty-if / `if (1)` / dead goto, no `do {} while (0)`
wrap, no alias rename, no redundant width cast, no linker-script touch.
`masked` is multi-read and consumed, so it makes no named-intermediate claim.

DISCLOSURE — construct (5). s8 MEASURED the sanctioned alternative rather than
re-arguing the citation the previous vet got wrong: a header-canonical 54-byte
struct array with every field at its true offset scores 4, not 0 (two
source-level hunks in the x54 stride decomposition; rejected/
s8-aggregate-struct-array-score4.c), under both a u16 and an s32 index. The
byte-displacement spelling used here is the one already oracle-proven on main
for the SAME symbols at the SAME stride in the SAME TU: func_800858D0
(src/main.c:983-988), src/main.c:1168, src/main.c:1306.

## T6 naming-announces-intent
No name is `pad`, `dummy`, `unused`, `spill`, `slack`, or `tail`. `masked`
names the parameter masked to a byte; `offset` names the slot byte offset;
`buf` is the voice-attribute record passed to func_8008B488; `i` is the loop
counter. Every local is read.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and carries no FAKE
construct, so no family carve-out is claimed or relied upon.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.
