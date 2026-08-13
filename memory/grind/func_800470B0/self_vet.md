# SELF-VET — func_800470B0

Diff surface: `src/sound.c` only — one added `#include "gte.h"` line, and the body
of `func_800470B0` (lines 435-455). No other file touched. No regfix/asmfix/rules/
engine/tools/Makefile/*.ld edits. Full build SHA1 == oracle
`62efab4f73f992798c43e8c730aa43baa10bb4fa`; sandbox `--disable all` distance 0.

CONSTRUCTS: (1) `#include "gte.h"`; (2) local retyped `s16 sp10[9]` -> `MATRIX sp10`
with `sp10.m[r][c]` member accesses; (3) params retyped `void *arg1, void *arg2` ->
`s32 *arg1, s32 *arg2` with `arg1[n]` / `arg2[n]` indexing replacing
`*(s32 *)((s8 *)p + 0xNN)`; (4) third argument `arg2` restored to the
`func_80052930(...)` call; (5) REMOVED — `register void *var_s1 asm("s1")`,
`register s32 var_s2 asm("s2")`, `volatile s32 _sp_pad[2]`, and the `var_s1`/`var_s2`
param aliases (these were the pre-existing cheats; the local `var_s0 = arg1` alias
is retained from the pre-existing body and is unchanged by this session).

## T1 semantic purpose
(1) `#include "gte.h"` — required to name the `MATRIX` type; ordinary header use.
(2) `MATRIX sp10` — this local IS a 3x3 rotation matrix. It is filled with
m[0][0]=0x1000, m[2][2]=0x1000, a computed m[0][1] and m[2][1], zeros elsewhere,
and its address is then handed to `func_80052930`, the matrix-transform leaf that
`camera_Transform` in this same file calls with `&g_cam_matrix`. Declaring it as
its actual type has direct semantic purpose: it names what the object is and gives
the callee the layout it expects. The 32-byte size (18 bytes of `m` + 2 pad + 12
bytes of `t`) is the type's real size, not a padding device.
(3) Typed pointers — `arg1`/`arg2` are word arrays; `arg2[5]` says what
`*(s32 *)((s8 *)arg2 + 0x14)` says, and matches the sibling `camera_Transform`'s
`s32 *a0, s32 *a1` signature. Pure readability; byte-identical either way (measured:
both spellings scored 7 before the call fix).
(4) The third call argument is the DESTINATION the callee writes. Its absence was a
defect: `src/sound.c:46` declares `extern void func_80052930(void *, void *, void *)`
— three parameters — and the call was passing two, leaving the callee's third
parameter undefined by the language. Restoring it makes the call well-defined.
Observable effect: yes — the callee receives its destination argument.
(5) Removals only.

## T2 human-programmer
Yes to all. A human writing "build a rotation matrix on the stack, transform a
point through it, then apply a Z-offset correction to the result" writes
`MATRIX m;`, indexes word arrays with `[]`, and passes a three-parameter function
three arguments. Nothing here reads as "why is this present?" — the opposite: the
pre-existing two-argument call and the `s16 sp10[9]` stand-in are what a reader
would question. There is no dead code, no unused declaration, no padding variable,
no self-assignment, no wrapper with an empty body.

## T3 GCC-internals justification
NO construct rests on a GCC internal for its justification.

Full disclosure of how the s1/s2 lever was FOUND, since honesty here matters more
than a clean-sounding answer: I read `sound.i.greg` (pseudo 74 = arg2 in hard reg
18, pseudo 75 = arg3 in hard reg 17) and reasoned about `global.c`'s
`allocno_compare` priority `floor_log2(n_refs)*n_refs*size / live_length` to
predict that arg2 needed one more reference to outrank arg3. That is diagnosis —
it told me WHERE to look. What it pointed at turned out to be a genuine bug in the
decompiled source (a dropped argument), and the fix is justified entirely by
program logic: the function's own prototype, `camera_Transform`'s three-argument
call to the same callee at `src/sound.c:424`, `src/text1a_post.c:166`, and
`include/m2c_context.h:738`. Delete every sentence about GCC from the reasoning and
the change still stands on its own as a correctness fix. That is the test, and it
passes. (Note also the fix costs zero instructions only because `arg2` is already
live in `$a2` from the incoming ABI at the call site — GCC coalesces the copy; the
emitted bytes are otherwise unchanged.)

## T4 permuter/search provenance
No permuter, no directed search, no auto-search tool was run this session. Every
form was hand-derived and hand-measured. No construct exists because a detector
failed to catch it.

## T5 family check
No construct matches any forbidden family, directly or by analogy. Specifically
checked: no register-asm pin (three were REMOVED); no hardcoded-`$N` `__asm__`;
no `asm("Sym")` alias rename; no scheduling barrier; no volatile coercion (the
`volatile s32 _sp_pad[2]` was REMOVED); no unused or written-never-read local array
(`sp10` is written and read); no dead store, dead self-assign, dead conditional
store, or empty-body `if`; no `do {} while (0)` or `if (1)` wrapper; no dead goto or
shared-label accumulator; no DImode chain; no opaque constant variable; no
combine-foldable chain-extender; no redundant width cast; no `.ld`/rodata reorder.
The `MATRIX` retype is NOT the frame-coercion family: that family is unused arrays
and `(void)&local` declared to reserve bytes, whereas this is the object's actual
type, written through and read back, with the callee requiring that layout.

## T6 naming-announces-intent
No name announces coercion intent. The `_sp_pad` local — which did — was REMOVED.
Remaining names are `sp10`, `temp_v1`, `var_s0` (pre-existing auto-generated
decompiler names, unchanged by this session), `arg0..arg3`. None is `pad`, `dummy`,
`unused`, `spill`, `slack`, `_buf`, or `tail`, and every one of them is read.

SANCTIONED-FAMILY-CLAIMS: none. This session claims no sanctioned-exception family
and needs none — every construct is ordinary C standing on program logic.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
