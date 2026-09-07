# SELF-VET — _dws

CONSTRUCTS: (1) `GpuRect` struct typedef replacing the `_GpuChunkHdr_CE0C` typedef for the
first parameter; (2) local `s32 var_s4` transfer-direction selector, set to 0 and read in
`var_s4 ? 0xB0000000 : 0xA0000000`; (3) the odd-word copy loop written
`while (--var_s0 != -1)`; (4) `*(s32 *)&rect->x` and `*(s32 *)&rect->w` reads of the two packed
32-bit GPU packet words; (5) clamps written as nested conditional expressions on
`D_8009BE78` / `D_8009BE7A`; (6) `to_write / 2`, `% 16`, `/ 16` left as C division/modulo.

## T1 semantic purpose
(1) The struct determines which bytes of the caller's rectangle each access reads; a different
shape reads different memory. Real semantics.
(2) `var_s4` selects the GPU transfer direction: 0xA0000000 is CPU->VRAM, 0xB0000000 is
VRAM->CPU. In this entry point it is 0, so the ternary's value is constant, and a reader may
reasonably ask whether the variable "does" anything. It is not a codegen device invented here:
it is what the original Sony source contains, as independently reconstructed by two matching
decomps of this same function (see T2). It carries the direction concept that makes `_dws` and
its sibling `_drs` the same algorithm.
(3) Semantics: run the loop body `var_s0` times. `while (--n != -1)` and `while (n--)` are
equivalent for n >= 0; both are ordinary ways to write a countdown. Real semantics.
(4) The GPU command FIFO takes the rectangle as two packed 32-bit words (x with y<<16, and
w with h<<16). Reading them as 32-bit words IS the operation. Real semantics.
(5) The clamps bound the destination rectangle to the VRAM page. Real semantics.
(6) `(w*h + 1) / 2` is the halfword-to-word count; `% 16` / `/ 16` split it into the FIFO
remainder and the DMA-able bulk. Real semantics.

## T2 human-programmer
Yes to all six — and for the debatable one, demonstrably so: this function is Sony PsyQ libgpu
`sys.c::_dws` (the GPU body of LoadImage), and two version-correct matching decomps of that
exact Sony function independently ship the same body. sotn-decomp
`src/main/psxsdk/libgpu/sys.c:608-655` (PSX main binary, GCC 2.7.2; `psxsdk/libgpu/sys` is
listed at `config/splat.us.main.yaml:43`) has `int var_s4;` at :613, `var_s4 = 0;` at :620 and
`*GPU_DATA = var_s4 ? 0xB0000000 : CMD_COPY_CPU_TO_VRAM;` at :640, plus the same
`% 16` / `/ 16` split and the same clamps. psyz `decomp/src/libgpu/sys.c:745-785` (PsyQ 4.0)
has the identical shape. The only place BB2's spelling departs from both is the copy loop
(`while (--var_s0 != -1)` vs their `while (var_s0--)`) — an ordinary countdown spelling, not a
construct.

## T3 GCC-internals justification
No. No construct here is justified by a GCC pass. The reasoning that produced each line is
"this is what the function does" (T1) plus "this is what the vendor source says" (T2). The only
place a GCC internal appears in this session's reasoning at all is *diagnostic*: explaining why
`while (n--)` produced a pre-decrement copy, and why the scorer's residual 24 was an artifact of
`engine/score.py::_resolve_named_pair` rather than emitted code. Neither is a mechanism a
construct was built around; no construct exists here to move an allocation, a schedule or a
spill. Notably this session went the other direction — it REMOVED the pre-existing
GCC-internals construct that the retired 2026-08 chassis carried (a register pin plus an empty
inline-asm barrier) by showing the fold it defended against never happens.

## T4 permuter/search provenance
No permuter or automated search was run this session. The body came from a banked human
transplant of the vendor source (`memory/grind/_dws/psyz-seed.c`); the one change to it (the
loop spelling) came from reading the objdump diff, not from a search.

## T5 family check
No construct matches any forbidden family, by analogy or otherwise. There is no register pin,
no inline asm of any kind, no scheduling barrier, no volatile coercion, no dead or unused local,
no dead store, no self-assign, no pad, no alias rename, no dead conditional or empty-body `if`,
no `do {} while (0)` wrap, no goto chassis, no cast widening, no build-time asm rewriting. Every
declared local is written once and read; every statement contributes bytes that carry program
meaning. The nearest neighbour worth naming explicitly is the constant-holder family (a scalar
local initialised to a constant): `var_s4` is textually shaped like one, but it is read into a
live branch that exists in the target, and it is present in the vendor source rather than
introduced to steer codegen — so it is program logic, not an exception being claimed.

## T6 naming-announces-intent
No name in the diff announces coercion intent. `var_s4` and `var_s0` are the m2c-derived names
the two reference decomps of this same Sony function use verbatim; they were kept precisely so
the correspondence to those references is checkable line by line, rather than renamed to
something that would assert a meaning the evidence does not establish. `to_write`, `size`,
`rect`, `data` and `GpuRect` are descriptive. Every one of these locals is read.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C. No coercion, exception or match-hack
family is claimed or relied on, so no SCOPE sentence or family precedent is cited. (The SOTN and
psyz line references above are source-reconstruction provenance for the vendor body, not a
family precedent.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.

## Proof of the match (recorded for the reviewer)
- `sandbox _dws --disable all` = **0**, 143/143 insns, `rules_dropped: 0`.
- `verify-oracle --rebuild --allow-dirty` -> `build_sha1
  62efab4f73f992798c43e8c730aa43baa10bb4fa`, `build_matches: true`.
- Non-circular byte proof: `tmp/grind/_dws/s1/bytecheck.py` applies the object's
  R_MIPS_HI16/LO16/26 relocations with real linked symbol addresses and compares all 143 words
  against the shipped encodings in asm/funcs/_dws.s -> `MISMATCHED WORDS: 0`.
- regfix.txt / asmfix.txt gain no entries; the retired 2026-08 chassis's 23 regfix rules and its
  two cheat constructs are all gone.
