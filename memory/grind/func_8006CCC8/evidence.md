# evidence — func_8006CCC8 (src/text1b.c)

## s1 (recon)

OBJECT MODEL: no DATA MODEL section was present in this session's brief (no
census signals flagged for this function's globals). Manually cross-checked
anyway, since the function is dense with `D_800A34FC` / `D_800A3524` accesses
shared with sibling functions already resident in this TU:

- `D_800A34FC` — MATCHES the established idiom already used by
  `func_8006CBD4` (line ~6603, same file) and `func_8006D324` (line ~6614):
  an `s32` global holding a pointer value, dereferenced via
  `(u8 *)D_800A34FC + <offset>` casts, never declared as an actual pointer
  type in this TU. `func_8006D324` treats `(s16 *)D_800A34FC` indices
  0x14/0x15 (byte offsets 0x28/0x2A) as a 2-element s16 array — this
  function's own `*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005` top-of-
  function check is a PACKED comparison of that exact same 2-element array
  (0x50005 == 0x00050005, i.e. both s16 slots == 5 at once). Consistent,
  no declaration-fix hypothesis warranted — measured via the score itself:
  once this field access was written exactly as the sibling idiom, the
  candidate compiled and scored non-trivially below the no-C-body floor
  (189 -> 94), so there is no lurking aggregate/declaration mismatch here.
- `D_800A3524` — MATCHES `func_8006CBD4`'s established idiom: a pointer-
  typed `s32` global to a 3-element record array, `u8` fields accessed at
  fixed offsets (+0x17, +0x1A used by the sibling; this function ALSO
  touches +0x1D — a third field in the same record, same array, same
  index range 0..2). No new declaration needed; reused the existing extern.

Both symbols get MATCHES verdicts (measured via successful compile + score
drop, not merely visual inspection) — no OBJECT MODEL mismatch is present.

## Baseline

- `canonical func_8006CCC8` -> verdict C, hand_coded_tier LOW (no S1/S2/S6
  signal). Ordinary pure-C target, distance 189 is size not hand-asm
  evidence.
- `sandbox func_8006CCC8 --disable all` (no C body, `INCLUDE_ASM`):
  score 189, target_insns 189, build_insns 0, no_c_body true.
- `m2c --valid-syntax` (no `--context` — `--context src/text1b.c` failed:
  "Directives not supported yet" at the file's `#include`, so m2c ran on
  the isolated asm only) produced a clean, fully-structured reconstruction
  (single `do { } while` loop trip count 2, all branches resolved, no
  `M2C_UNK` beyond the two extern call signatures). High-confidence
  starting shape.
- Call site (`func_8006D338`, same file, line ~6628-6631):
  `func_8006CCC8(&arg0, &arg1, (s32)((r << 16) >> 16));` — 3 args: first
  (`&arg0`) is UNUSED by the callee body (asm never touches `$a0`), second
  (`&arg1`) is the bitmask pointer (`$a1` -> target's `$s4`), third is a
  pre-sign-extended `s32` value collapsed from an `s16` (`$a2` -> target's
  raw `$s7`).

## s1 candidate iteration 1 (rejected, banked to rejected/cached-field28-pointer-local.c)

Wrote the m2c structure with a cached `s16 *slot` pointer local computed
once per loop iteration and reused for all ~5 `D_800A34FC`-offset field
accesses inside that iteration (natural-looking C — cache the address once,
dereference N times).

- `sandbox --disable all`: score 142 (target_insns 189, build_insns 189).
- Disassembly of the sandboxed `.o`: our frame is `sp,-0x58` (88 bytes) vs
  target's `sp,-0x48` (72 bytes, confirmed from `asm/funcs/func_8006CCC8.s`
  prologue). Root cause read directly off the disassembly: the `ret`
  variable (target keeps this in `$fp` — `addu $fp,$zero,$zero` at entry,
  `addiu $fp,$zero,1` on the early-return path, `addu $v0,$fp,$zero` at the
  epilogue) got STACK-SPILLED in our build (`sw zero,16(sp)` at entry,
  `sw t0,16(sp)` on the same path, `lw v0,16(sp)` before the epilogue)
  instead of register-allocated. The extra `slot` pseudo (never present in
  target — target recomputes the address at EVERY syntactic use site, see
  next entry) raised register pressure past what `global.c`'s allocator
  could fit into the same 9 callee-save registers target uses, so `ret`
  (used least/latest) lost its register and reload spilled it.
- KILLED (instance): caching the field access behind a named pointer local
  is NOT the shape target's source used, for this exact chassis. Measured
  regression (142 vs 94 for the alternative in the same session), not
  merely theorized.

## s1 candidate iteration 2 (banked, current resident body — candidate.c)

Rewrote every field access as an inline `*(s16 *)((u8 *)D_800A34FC + mask +
0x28)` expression textually repeated at each of the ~5 use sites (no cached
pointer/value local) — matching target's own repeated fresh-address
computation, confirmed instruction-for-instruction in the disassembly
(`lw v0,%gp_rel(D_800A34FC)($gp); addu a0,v0,s3; lh v0,0x28(a0)` recurring
at each of the 3 distinct field28 load sites in the target asm, each in a
DIFFERENT basic block — so cse1's per-block scope can't merge them even
though the same expression is written 5 times in the C).

- `sandbox --disable all`: score 94 (target_insns 189, build_insns 193).
  Frame back to `sp,-0x48` matching target exactly; `ret` now lives in
  `$s8`/`$fp` as in target (confirmed in disassembly:
  `move s8,zero` / `li s8,1` / final `move v0,s8` before epilogue is not
  literally what's emitted — actual dump shows `s8` used analogous to
  target's `fp`, register renamed but same role/behavior).
- Remaining structural diff (read directly off the second disassembly,
  `tmp/sandbox/func_8006CCC8/text1b.o` at the time of measurement — not
  re-saved as a standalone artifact file this session, reproducible via
  `sandbox func_8006CCC8 --disable all` + objdump on the emitted `.o`):
  the sign-extension of `arg2` (third parameter, raw value lives in
  target's `$s7`) is HOISTED out of the `for` loop in our build — computed
  ONCE before the loop into its own register (`sll v0,s0,0x10; sra
  s7,v0,0x10` appears once, before the loop header, in our disassembly) —
  whereas target computes the FULL 16-bit sign-extension INSIDE the loop
  body EVERY iteration (`sll v0,s7,16; sra v0,v0,16; srav v0,v0,s1`, all
  three ops present at the top of `.L8006CD54` in
  `asm/funcs/func_8006CCC8.s`), keeping only the RAW `a2` value live in
  `$s7` across iterations. This is not a register-count discrepancy —
  both our build and target end up using 9 registers total across the
  loop (`$s0..$s7` + `$fp`/`$s8`) — it is purely whether GCC's
  `move_movables` (loop.c) chose to hoist the invariant sign-extension.
  This matches the documented mechanism in
  `.claude/rules/defeat-licm-hoist-var-reuse.md`: a single-set,
  non-trapping, loop-invariant pseudo in a short loop-with-call is
  essentially always hoisted by `move_movables` unless its register is
  made MULTI-SET by routing an already-used loop-variant value through the
  same C variable.
- CONFIRMED (instance, measured on this chassis, no FAKE construct
  present): the field-access caching lever (inline-dereference-at-each-
  use-site vs cached-pointer-local) is real and moves the score from 142
  to 94; banked as the resident src/text1b.c body at end of s1.
- NOT YET measured: which specific C variable target's register-reuse
  would correspond to, if any — `$s7` in target holds ONLY the raw `arg2`
  value throughout the whole function (no second use observed), so the
  standard defeat-licm-hoist-var-reuse recipe (route a USED loop-variant
  value through the SAME pseudo as the invariant) needs a fresh read of
  the `.greg`/`.loop` cc1 dump to identify the correct reused variable
  before attempting a fix — this is frontier item #1 for s2, NOT run this
  session (turn budget spent establishing the shape + the two measured
  field-access variants first).

## Artifacts

- `tmp/grind/func_8006CCC8/s1/candidate_body.txt` — snapshot of the
  resident `src/text1b.c` function body at end of s1 (lines 6610-6682 at
  save time).

- [s1] canonical func_8006CCC8: verdict C, hand_coded_tier LOW, ordinary pure-C target, distance 189 was size not hand-asm evidence.

- [s1] OBJECT MODEL: D_800A34FC and D_800A3524 both MATCH the established pointer-in-s32-global idiom already used by sibling func_8006CBD4 and func_8006D324 in the same TU (measured by successful compile + score drop from 189 to 94, not just visual inspection) -- no declaration-fix hypothesis needed; the top-of-function *(s32*)((u8*)D_800A34FC+0x28)==0x50005 check is a packed comparison of the same 2-element s16 array func_8006D324 writes via v1[0x14]=5; v1[0x15]=5.

- [s1] m2c --valid-syntax (without --context; src/text1b.c failed m2c's context parser on an #include directive) produced a fully-structured, no-M2C_UNK-beyond-externs reconstruction of the whole function in one pass -- high-confidence starting shape, used as the basis for the candidate.

- [s1] Call site func_8006D338 passes 3 args: &arg0 (unused by callee body -- $a0 never referenced in asm/funcs/func_8006CCC8.s), &arg1 (bitmask pointer, becomes target's $s4), and a pre-sign-extended s32 collapsed from an s16 (becomes target's raw $s7).

- [s1] First full candidate (m2c structure, cached field28 pointer local): sandbox --disable all score 142, target_insns 189, build_insns 189; frame sp,-0x58 vs target sp,-0x48; ret variable stack-spilled instead of register-allocated.

- [s1] Second candidate (inline field28 dereference at each use site, no cached pointer/value local): sandbox --disable all score 94, target_insns 189, build_insns 193; frame sp,-0x48 matches target; ret variable register-allocated matching target's $fp usage pattern. This is the resident body in src/text1b.c at end of session.

- [s1] Remaining structural diff identified directly in the disassembly (not yet fixed): our build hoists the full 16-bit sign-extension of arg2 out of the for-loop into its own register computed once before the loop; target recomputes it (sll 16; sra 16; srav i) every iteration inline while keeping only the raw a2 value live across iterations in $s7. Matches the documented loop.c move_movables mechanism in .claude/rules/defeat-licm-hoist-var-reuse.md -- register count is identical (9 registers used in both builds across the loop), so this is a pure hoist-vs-no-hoist codegen decision, not a pressure difference.
