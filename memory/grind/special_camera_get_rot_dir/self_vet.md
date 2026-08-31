# SELF-VET — special_camera_get_rot_dir (grind s8, 2026-08-30)

Diff under vet (three files, all in this function's granted scope):
  - `include/code6cac.h:510` — `extern void CdRead(s32);` → `extern s32 CdRead(s32, s32, s32);`
    (scope granted: `tools/grinder/scope_allow.txt`, entry
    `special_camera_get_rot_dir include/code6cac.h`, owner ruling 2026-08-30,
    decisions.md escalation-batch entry, ruling 2)
  - `src/code6cac_b2_post.c` — `func_800372F4` widened from `(s32 arg0)` to
    `(s32 nbytes, s32 buf, s32 mode)`, forwarding all three to `CdRead`
  - `src/code6cac_b2_post.c` — `INCLUDE_ASM("asm/funcs", special_camera_get_rot_dir);`
    replaced by the pure-C body; the two stale unused `Quad`/`Triple` typedefs
    (leftovers from an abandoned s2-era word-splat copy form) deleted; a `CamRot`
    typedef added for the 60-byte aggregate copy.

Measured THIS session with the diff in place in src/ and include/:
  sandbox special_camera_get_rot_dir --disable all -> score 0, 72/72 insns
  sandbox func_800372F4              --disable all -> score 0, 21/21 insns
  verify-oracle --rebuild --allow-dirty -> ok:true, build_matches:true,
      build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
  rules_dropped 0 for both functions (zero regfix/asmfix carriers).

CONSTRUCTS: (1) `CamRot` struct typedef + the aggregate assignment
`*(CamRot *)dest = *(CamRot *)&sp_buf[0x10];`; (2) `s32 mode = 0x80;` local read
at both `func_800372F4` call sites; (3) `for (;;) { ... continue; ... break; }`
retry loop; (4) `func_800372F4`'s widened 3-parameter signature, all three
parameters read in the body; (5) the `include/code6cac.h:510` prototype
correction; (6) plain locals `index`, `cam_base`, `v0` and the stack buffers
`sp_buf[0x800]` / `sp_buf2[8]`.

## T1 semantic purpose
(1) The aggregate assignment IS the function's payload: it moves the 60-byte
rotation record out of the sector buffer into the caller's `dest[]`. Remove it
and the function returns nothing to its caller. Real semantics.
(2) `mode` is the CD transfer mode (0x80 = double-speed) passed to both reads.
Remove it and `CdRead` receives no mode. Real semantics — and it is a *read*
value at two sites, not a dead scalar.
(3) The retry loop is the function's error handling: a failed read (`v0 != 0`)
restarts the whole seek/read sequence. Removing it changes behaviour on read
failure. Real semantics.
(4) `buf` and `mode` are loaded into `$a1`/`$a2` and consumed by `CdRead`
(`src/system.c:901` stores them to `D_800A14D4` / `D_800A14DC`). Removing them
leaves `CdRead` reading indeterminate registers. Real semantics.
(5) The header correction makes every TU that includes `code6cac.h` call
`CdRead` with the argument count its byte-matched definition actually reads.
Real semantics, and correct independently of this grind.
(6) Every local is written and read; both buffers are filled by the CD driver
through the addresses passed to it and then read back (`sp_buf[0x10..0x4B]` by
the aggregate copy, `sp_buf2` by the `CdControl(2, (s32)sp_buf2, 0)` seek).
No construct in this diff is behaviour-neutral.

## T2 human-programmer
Yes for all six. A programmer writing "seek, read a sector, copy the 60-byte
record out of it, seek to the next sector, read the variable-length block,
retry the whole thing on failure" writes exactly this: a struct copy for the
fixed-size record, a named `mode` constant for the CD mode argument, a `for (;;)`
retry with `continue` on failure and `break` on success, and a wrapper that
forwards its arguments to the library call it wraps. Nothing here reads as
"why is this here?" — the diff is strictly SIMPLER than the code it replaces
(one aggregate assignment where s2-s6 had 15 word stores; two dead typedefs
deleted; no pre-loop pointer local).

## T3 GCC-internals justification
No construct is justified by a GCC internal. The mechanism note in the ledger
(that GCC's MIPS backend `expand_block_move`/`block_move_loop` lowers the
aggregate assignment into the target's 4-word loop plus 3-word tail) is a
POST-HOC EXPLANATION of why the honest source form produces the target bytes —
it is not the reason the construct was chosen, and no construct was shaped to
manipulate an allocator, scheduler, or reference-count heuristic. The s1-s6
sessions that DID reason from allocno priority and `reg_n_refs` produced nothing
and their forms are all banked as rejected; this form was reached by asking what
the source said, not what the passes would do with it.

## T4 permuter/search provenance
None. Zero permuter output in this diff. The s4 permuter campaign (~46k iters)
found nothing and is banked as dead. The form was derived by reading
`asm/funcs/special_camera_get_rot_dir.s` against the MIPS backend's block-move
expansion shape and against `src/system.c:901`. No directed enumeration over
candidate spellings was run this session.

## T5 family check
No forbidden family, by analogy or otherwise.
- Not the BANNED asymmetric loop-spelling pair: there is no inner loop in this
  source at all. The target's copy loop is emitted by the backend from one
  aggregate assignment, so there is no structured-vs-goto choice to make. The
  single `for (;;)` is the function's only C loop and it is the natural spelling
  of an unconditional retry.
- Not the BANNED unread-parameter widening: the earlier `(s32 arg0, u32 *buf,
  s32 mode)` form was FAILed because `buf`/`mode` appeared nowhere in the body
  and the call was still `CdRead(arg0 >> 11);`. Here both are read and forwarded,
  and the arity is backed by the byte-matched definition at `src/system.c:901`.
- Not the BANNED block-scope `extern s32 CdRead(s32, s32, s32);` scope-gate
  workaround: no redeclaration of `CdRead` appears anywhere in
  `src/code6cac_b2_post.c`. The prototype is corrected once, at its canonical
  location in `include/code6cac.h`, under the owner's scope grant — which is the
  exact remedy the layer-1 reviewer named on 2026-08-26 01:42.
- `mode` is not a constant-holder / dead scalar local: dead-scalar forms are
  never read; this one is read at two call sites and its absence costs 3
  instructions and 12 score points (banked:
  `rejected/literal-mode-no-local-score12.c`).
- The `CamRot` typedef is an ordinary local aggregate type for a fixed-size
  record, not an aggregate-merge of splat `D_80xxxxxx` scalars — it types stack
  and caller memory, touches no global symbol, and makes no claim about the
  original object model.

## T6 naming-announces-intent
No name in the diff announces coercion intent. `nbytes`, `buf`, `mode`, `index`,
`cam_base`, `v0`, `sp_buf`, `sp_buf2`, `dest`, `CamRot`, `rot` — every one names
what the value IS, and every one is read. No `pad`, `dummy`, `unused`, `spill`,
`slack`, `tail`, or `_frame_pad`. `sp_buf`/`sp_buf2` follow this file's existing
stack-buffer naming convention (cf. `sp` at `src/code6cac_b2_post.c:212, 440`)
and are genuine live buffers, not frame coercion.

SANCTIONED-FAMILY-CLAIMS: none — this diff contains no coercion construct and
claims no carve-out. Every construct is ordinary C that a programmer would write
from the specification alone.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
