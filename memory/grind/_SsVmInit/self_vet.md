# SELF-VET — _SsVmInit  (session 8, synthesis)

Measured this session: `sandbox _SsVmInit --disable all` → score 0,
target_insns 200 == build_insns 200. Full `build` → SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (MATCH).

CONSTRUCTS: (1) forward `extern` declaration block for globals this function is
the first user of in the TU; (2) `u16 masked = (u8)a0;` read in the clamp
condition and in the else arm's store; (3) `s32 buf[16]` local record filled and
passed by address to func_8008B488; (4) `s32 offset = i * 54;` per-voice slot
base; (5) byte-offset writes through the per-voice slot symbols
(`*(s16 *)((u8 *)&D_800F4E1A + offset) = 0x18;` and the 21 siblings);
(6) `_svm_cur.voice = i;` — the TU's shipped aggregate field for 0x8010280A.

## T1 semantic purpose
(1) Declarations; without them GCC 2.7.2 assumes implicit int and mis-types the
store widths — a real, observable behavioural difference. (2) The mask IS the
semantics: `_SsVmMaxVoice` is a u8 voice count and the parameter is taken modulo
256 both for the `>= 0x18` clamp test and for the stored value; delete the cast
and the function behaves differently for a0 > 255. (3) `buf` is the SpuVoiceAttr
record the call consumes — every field written is read by the callee. (4)
`offset` is the slot base address arithmetic; it is read 22 times. (5) The
stores are the function's actual work (initialising 24 voice slots). (6) The
store of the current voice index that `_SsVmKeyOffNow` immediately reads
(src/main.c:1290 region → the callee reads `_svm_cur.voice`). No construct in
the diff is removable without changing behaviour.

## T2 human-programmer
Yes for all six. A programmer told "clamp the caller's voice count to 24, then
initialise that many 54-byte voice slots and key each one off" writes exactly
this: a masked local for the clamp, a strided offset, per-field slot stores, and
a write of the current-voice field before the key-off call. Nothing here would
make a reader ask "why is this here?".

## T3 GCC-internals justification
No construct is justified by a GCC pass. The `u16 masked` local was FOUND by
reasoning about which value the target's `sb` sources (target masks once and
stores the masked value), but it is justified by the program logic above: the
stored value and the compared value are the same masked quantity, and the C now
says so. No allocator/scheduler/DCE/RTL mechanism is invoked, and no `/* FAKE */`
annotation is present or needed.

## T4 permuter/search provenance
None. s5's ~26k-iteration permuter campaign found nothing and is not the source
of any construct here. The closing form came from reading the target asm
(one masked value feeding both the compare and the store) — a source-level
reading, not a search artifact. s8 additionally re-measured and REMOVED four
constructs earlier sessions had carried (a 0xFF named intermediate, a
variable-reuse of `offset`, an `s32 idx = i;` copy, and an expanded
shift-subtract spelling of the ×54 stride): all four were inert on this chassis,
so the body is strictly simpler than any previously banked form.

## T5 family check
No forbidden family applies. Specifically not: no register pins, no `__asm__`,
no scheduling barrier, no `volatile` anywhere, no dead store or self-assign, no
constant-holder, no dead local or unwritten pad, no empty-if / `if (1)` / dead
goto, no `do {} while (0)` wrap, no alias rename, no redundant width cast, no
linker-script touch. `masked` is multi-read and consumed, so it is not a
named-intermediate-family construct and makes no such claim.

DISCLOSURE — construct (5), byte-offset writes through per-voice splat symbols.
The auto-scan flags these as declaration puns. They are the spelling this TU
already ships and the oracle already proves: src/main.c:1168
(`*(s16 *)((u8 *)&D_800F4E1E + _svm_cur.voice * 54) = 0x7FFF;`) and
src/main.c:1413/1422/1423 are byte-matched C on main using the identical form
for the same 54-byte slot array. The sanctioned fix — merging `_svm_voice[]` at
the declaration in include/sound.h — is not available to this session: the
symbols carry no header declaration at all (D_800F4E18/1A/1C/1E/20/22/24/2E/35…
have `decl: NONE in include/*.h`), and the aggregate-merge family's prong (c)
amendment of 2026-09-03 keeps the per-word rows alive while they still serve
still-INCLUDE_ASM siblings, which here are SsUtKeyOnV, _SsVmFlush,
_SsVmKeyOnNow and vmNoiseOn — all four still asm at HEAD. Merging the
declaration is therefore a later, TU-wide integration step coupled to those
siblings' retirement, not a change this function can make; this candidate uses
the on-main, bytes-proven spelling in the meantime and claims no family for it.

DISCLOSURE — the s7 layer-1 FAIL is addressed at its root, not respelled. The
previous body wrote the current-voice index through a per-word splat scalar for
0x8010280A. That address IS already merged into the TU's shipped object model
(include/sound.h:28, `struct struct_svm _svm_cur`; base 0x801027F0, `.voice` at
+0x1A = 0x8010280A), so this body writes `_svm_cur.voice = i;`, identical to the
bytes-proven store in the sibling at src/main.c:993. The corresponding forward
`extern` and the header comment the reviewer objected to are both gone from the
body entirely.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `slack`, `tail`, `buf`-
as-coercion, or any variant. `masked` names the value it holds (the parameter
masked to a byte) and is read twice; `offset` names the slot byte offset and is
read 22 times; `buf` is the voice-attribute record actually passed to
func_8008B488; `i` is the loop counter. Every local is read.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and carries no FAKE
construct, so no family carve-out is claimed or relied upon.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.
