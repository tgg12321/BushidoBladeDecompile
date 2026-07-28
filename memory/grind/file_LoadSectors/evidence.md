# Evidence bank — file_LoadSectors

- WIP rejected_form: _pad[2] (DCE'd, pure dead-vars-local-aggregate) — adjudicator ruling 2026-06-14

- WIP rejected_form: _pad[4] instead of _pad[2] (frame still 40; GCC DCEs both; no effect)

- WIP rejected_form: Rect r (unused struct, 8 bytes) — score=0 but cheat-reviewer FAIL 2026-06-16; saved rejected/dead-vars-local-aggregate-rect.c

- WIP rejected_form: v9-result[2]-used (result[0]=count<<11; return result[0]) — score=1, one extra sw v0,16(sp) before epilogue

- WIP rejected_form: v15-assign-before-call (result[0]=sector<<11; ang_hosei(fd,result[0],0)) — score=3, sw moved to before ang_hosei call + bne delay slot changed

- WIP rejected_form: struct_return (function returns 8-byte struct) — adds addiu a0,sp,16 to body (extra instruction vs oracle)

- == imported from memory/wip notes.md ==
# file_LoadSectors — NEEDS DECISION (empirical lock-out)

## TL;DR
Every pure-C mechanism GCC 2.7.2 can use to produce frame=48 either (a) adds a body
instruction not in the oracle, or (b) uses an unused local aggregate that the
cheat-reviewer classifies as dead-vars-local-array. GCC 2.7.2 provably never DCEs dead
stores to local arrays. The adjudicator must decide whether an unused 8-byte local
is acceptable here given it was demonstrably in the original source.

## Oracle structure
- frame=48 (`addiu sp,sp,-48`), saves at sp+24..44
- ZERO loads/stores to sp+16..23 in the body
- Body is 100% instruction-for-instruction identical to the clean baseline (no local vars)
- Clean baseline compiles to frame=40 (16 arg area + 24 saved regs, no local slot)

## What this session tried

**v9** (`s32 result[2]; result[0] = count<<11; return result[0];`):
- frame=48 ✓, score=1
- GCC emits `sw v0,16(sp)` before epilogue — one extra instruction not in oracle
- CSE removes the `lw v0,16(sp)` (replaces with register value), but the SW persists

**v15** (assign `result[0] = sector<<11` before `ang_hosei(fd, result[0], 0)`):
- frame=48, score=3 — GCC moves `sll` into bne delay slot and keeps SW before ang_hosei

**Struct return**: Adds `addiu a0,sp,16` to body (extra instruction). Ruled out.

**Unused struct `Rect r`** (score=0): Cheat-reviewer FAIL → saved to `rejected/`.

## Empirical proof: GCC 2.7.2 NEVER DCEs dead array stores
Tested three DCE-candidate forms with the actual toolchain (test_dce.sh, 2026-06-16):
1. `arr[0]=x; return arr[0];` → GCC keeps `sw v0,0(sp)` even after CSE removes the load
2. `arr[0]=x; return x;` → GCC keeps `sw v0,0(sp)` even though array never read again
3. `arr[0]=x<<4; use(y,arr[0],0); return y<<4;` → GCC keeps `sw a1,16(sp)` (in jal delay)
**Conclusion: no USED local array form can produce zero body stores to sp+16..23.**

## Only score=0 path: unused local aggregate
The only mechanism that gives frame=48 with zero body stores is an unused local array
or struct declaration (GCC's `assign_stack_local()` allocates the slot in `expand_decl`
before any body expression is evaluated; if never referenced in RTL, no store is emitted).

This is the `_pad[2]` / dead-vars-local-aggregate pattern.

The oracle MUST have come from a source containing an unused 8-byte local. GCC 2.7.2
cannot produce frame=48 for this function's logic from any other mechanism.

## Adjudicator question
Prior ruling (2026-06-14) classified `_pad[2]` as a dead-vars-local-aggregate cheat.
This session proves (via DCE tests) that the original source provably contained an unused
8-byte local. The cheat-by-intent standard asks: "was the construct added by a human
programmer for a reason, or manufactured by an agent to hit the score?" Here the answer
is: the unused local WAS in the original source — we can prove GCC couldn't produce the
frame without it. Should unused 8-byte locals be accepted for this function given the
empirical evidence?

## Floor
- HEAD: frame=40, distance=14 (14 save/restore offset diffs, body identical)
- v9 form: frame=48, score=1 (1 extra sw; see candidate.c)
- Only score=0 form (unused aggregate): cheat-reviewer FAIL (see rejected/)

== s1 (recon, 2026-07-28) ==

- [s1] FRAME DECOMPOSITION CORRECTED. Target frame 48 = args **24** + regs 24 + vars **0**
  — NOT vars=8. The census (tmp/grind/func_80037540/s2/bb2_frame_slack2.txt line for
  func_800165F8) assumed args=16 by resolving callee arity; but GCC's outgoing-args area
  is set by CALL EXPRESSIONS EXPANDED, not by surviving calls. The untouched sp+16..23
  bytes are outgoing-args bytes. This dissolves the s0-era "original must have had an
  unused 8-byte local" proof — it only covered the vars axis.

- [s1] VARS AXIS MEASURED FULLY DEAD (7 probes + 1 structural kill): HImode-const forms
  fold at tree (vars=0); DImode cast folds; DImode local live-across-calls gives frame 48
  via regs=8 (wrong offsets); tail-block DImode adds 2 junk insns; s16 count param gives
  regs=7 (wrong); s16 sector param vars=0. The tslLineG5Init HImode-bitwise phantom needs
  narrow global loads AND dies when calls are present (m_two_s16_himode_call witness) —
  structurally unreachable in this all-calls, zero-loads body. See
  rejected/phantom-vars-slot-all-probes-vars0.c + tmp/grind/file_LoadSectors/s1/.

- [s1] SANDBOX 0 ACHIEVED (real TU, engine sandbox --disable all: score 0, 51/51 insns):
  provably-dead guarded 5-arg call `if (fd == -1) { debug_printf(fd,0,0,0,0); }` placed
  after the `if (fd == -1) return -2;` early-out. Mechanism: calls.c expand_call bumps
  current_function_outgoing_args_size to 24 during RTL expansion; RTL jump/cse then
  deletes the provably-dead arm; the bump is monotonic and never shrinks. Emits ZERO
  instructions, ZERO relocs, ZERO rodata (scalar args only — a string-literal arg would
  leak rodata even from a deleted call). Body byte-identical incl. save offsets sp+24..44.

- [s1] CLASSIFICATION OPEN → ruling-request filed. The construct is dead code whose only
  effect is the args-area size — not in any sanctioned FAKE family (dead-store,
  named-local, pointer-alias, do-while-0, written-never-read array). It is also the ONLY
  remaining mechanism: source space reaching frame 48 with a byte-identical body =
  {dead 8-byte local aggregate (FORBIDDEN + reviewer-FAILed s0), deleted >=5-arg call
  (this form, unclassified)}. Both axes now measured, so the ruling decides the function.

