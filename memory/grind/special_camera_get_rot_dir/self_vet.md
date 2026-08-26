# SELF-VET — special_camera_get_rot_dir

CONSTRUCTS: `for (;;) { ... continue; ... break; }` retry loop; `copyloop:` label +
backward `goto copyloop;` post-test copy loop; six ordinary locals (index, cam_base,
v0, constant_80, copy_end, buf2_ptr) plus two block-scope locals (dst_q, src);
`Quad`/`Triple` struct types for the 16-byte / 12-byte block copies (both PRE-EXISTING
at HEAD, above the function, not added by this diff); arity cast
`((s32 (*)())func_800372F4)(a, b, c)` on the two calls to a function whose in-TU
definition takes one parameter.

## T1 semantic purpose
- `for (;;)`+continue/break: the function retries the CD read until it succeeds. Both
  back edges are in the target's control-flow graph (`bnez $v0, .L80037394` at
  0x800373B0 and 0x80037438, both branching to the single loop header). Removing the
  loop changes what the function DOES (one attempt instead of retry-until-success).
  It is the function's specification, not decoration.
- `copyloop:`/`goto copyloop`: performs the 0x30-byte block copy from the CD buffer to
  `dest`. Removing it means no data is copied. Purely semantic.
- every local is written once and read at least once, and every one of them occupies a
  named callee-saved register in the target bytes ($s0 dest, $s1 buf2_ptr, $s2 index,
  $s3 cam_base, $s4 constant_80, $s5 copy_end). There is no local whose removal leaves
  the output byte-identical — the six-register save/restore block in the target
  prologue/epilogue IS these variables.
- the arity cast is what lets the call pass the three arguments the target's `jal`
  actually sets up ($a0/$a1/$a2 at 0x800373A0-0x800373AC and 0x80037428-0x80037434).
  Without it the third and second arguments cannot be expressed at all.
ANSWER: every construct changes the emitted bytes and the program's behaviour. None is
byte-neutral. PASS.

## T2 human-programmer
A programmer given only the spec ("re-read the special-camera record from CD until the
read succeeds, copy 0x30 bytes of it into the caller's buffer, then seek to the next
frame and verify") writes exactly this: an infinite retry loop with `continue` on read
failure and `break` on success, and a pointer-bumping copy loop that stops when the
source pointer reaches a precomputed end address. Nothing here provokes "why is this
here?" — there is no extra statement, no extra variable, no annotation, no cast whose
value is discarded. The only stylistic choice a reader might query is the `goto`-spelled
inner loop rather than `do {...} while (src != copy_end);` — but a backward `goto` to a
label is a plain loop, present in the era's C and in SOTN master (see PRECEDENT below),
and it introduces no object that a `do/while` would not also introduce. PASS.

## T3 GCC-internals justification
The MECHANISM I used to CHOOSE between two semantically identical loop spellings is
flow.c's loop_depth weighting of `reg_n_refs` feeding global.c's `allocno_compare`.
That is a selection criterion, not the construct's justification: the program logic
fully explains BOTH spellings on its own (each is a loop that performs a required copy),
and neither spelling adds anything to the program. This is the ordinary situation in a
matching decompilation — several correct C spellings exist and the bytes pick one.
The test's cheat signal is "the program logic is not the explanation"; here the program
logic IS the complete explanation of every construct present, and nothing in the diff
exists FOR the allocator. No lever named after a pass, no pin, no barrier, no dead
carrier, no annotation. PASS.

## T4 permuter/search provenance
No permuter, no auto-search, no randomised proposal. This form was DERIVED: I extracted
the exact allocno table with the instrumented cc1 (tools/gcc-2.7.2/cc1 + BB2_ALLOC_DEBUG,
via tmp/grind/special_camera_get_rot_dir/s6/alloc.sh), reproduced global.c's priority
arithmetic in tmp/grind/special_camera_get_rot_dir/s7/manifold.py, and enumerated the
four realistic loop-spelling / reference-count combinations against it. Exactly one of
the four produces the target permutation, and it was then spelled and measured. The
construct is not "whatever passed the detectors"; it is the one of four ordinary C
spellings that is correct, and the other three are banked as measured rejections. PASS.

## T5 family check
No construct in the diff matches any forbidden family, by analogy or otherwise: no
register-asm pin, no `__asm__` of any kind, no regfix/asmfix rule (rule count is zero;
regfix.txt and asmfix.txt are untouched), no scheduling barrier, no volatile of any
kind, no alias rename, no `asm("Sym")`, no dead store, no self-assign, no dead
conditional, no `if (1)`, no `do {} while (0)` wrap, no opaque constant holder, no
unused local array, no `(void)&local`, no dead-goto pad, no redundant width cast, no
linker/rodata reorder. The diff replaces one `INCLUDE_ASM` line with a function body and
touches nothing else in the repository (`git status`: src/code6cac_b2_post.c only,
besides the engine's own metrics/events.jsonl append). PASS.

## T6 naming-announces-intent
Names present: `dest`, `sp_buf`, `sp_buf2`, `index`, `cam_base`, `v0`, `constant_80`,
`copy_end`, `buf2_ptr`, `dst_q`, `src`, `copyloop`. None is `pad`/`dummy`/`unused`/
`spill`/`slack`/`tail`/`_frame_pad`. `sp_buf`/`sp_buf2` are the inherited stack-buffer
names carried since s1 and are genuinely stack buffers that the CD read fills and the
copy loop reads — they are not frame-coercion pads (`sp_buf` is passed as the read
destination at 0x800373A4, `sp_buf2` is passed to CdIntToPos and CdControl). Every named
object is read. PASS.

SANCTIONED-FAMILY-CLAIMS: none. No construct in this diff requires a sanctioned-family
carve-out; all of it is ordinary C whose every object is consumed and whose every
statement is load-bearing for the function's behaviour. For the one construct a reviewer
is most likely to probe — the backward-`goto` loop — SOTN master ships the same shape:
  PRECEDENT: `src/dra/5F60C.c:579` (SOTN master, PSX/GCC 2.7.2; label
  `loop_check_equip_id_1:` at brace depth 3, backward goto from line 582), indexed in
  `docs/reference/sotn-construct-index.md:1015`.
And for the arity cast, this repository already ships a committed, byte-matching
instance of the identical idiom in the same translation unit:
  PRECEDENT: `src/code6cac_b2_post.c:374` (`((s32 (*)())func_80037110)()`).
Neither is offered as a carve-out claim; both are cited only to show the spellings are
ordinary and precedented rather than novel.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
