# Evidence bank — func_800481E8

## s1 (recon, 2026-07-28) — floor 14 → 10, inline-move cheat dissolved

- canonical: verdict C, 56 target insns. sandbox --disable all: 14 at session
  start (12 rules dropped, cheat-asm stripped), **10 after s1 edits** (in src/).
- Frame equation (target): 72 = ALIGN8(vars=32) + args 24 (5th call arg at
  0x10) + regs 16 (s0/s1/s2/ra at 56/60/64/68). Our build: 40 with vars=0 —
  IDENTICAL otherwise. All 10 residual diffs are frame-offset instructions
  (2× addiu sp, 4× sw, 4× lw). Loop body, prologue staging, epilogue order,
  register allocation: all byte-match already.
- The prior session's "forbidden dead-frame floor / canonical-asm endpoint"
  conclusion is DISSOLVED: it predates [[phantom-frame-slots-gcc272]]
  (2026-07-13), which proves vars>0 with zero dead decls is reachable
  (witness tslLineG5Init, COMPLETED-C, vars=8 untouched). The 32 bytes need a
  live phantom-slot spelling, not a pad array and not canonical-asm.
- Sibling frame witnesses are NOT clean: AddTbpOfst_80047EE8 (parked) gets
  vars=32 via forbidden `s32 unused_slack[8]`; InitHiraRmd_80047FBC via
  declared `s32 buf[8]` + (void)buf. No legitimate vars=32 witness in cluster.
- InitHiraRmd_80047FBC's prologue technique (its in-file comment, levers 1+2)
  transfers verbatim: base-copy staging + function-scope `a0_for_call`
  precompute + FAKE `arg0 = 0;` produce target's `$s0=$a0; $s2=$s0` chain and
  `addu $s0,$s2,$v0` second-pointer binding. Measured both directions
  (with/without the dead store — see rejected/).
- Fresh find_duplicates.py run (s1): NO lead for func_800481E8 (stale 0.625
  lead to func_800483DC gone; that shape lacks the conditional increment).
- cc1 `.frame` probe harness: tmp/grind/func_800481E8/s1/frame_probe.sh
  (arg = .c path; prints per-func vars=; direct frame gradient per
  [[phantom-frame-slots-gcc272]]).

- WIP rejected_form: {'form': 's32 pad[8] / register pins + frame-size regfix', 'score': 0, 'reason': 'FORBIDDEN dead-frame coercion per dead-vars-local-array; engine refuses completion.'}

- == imported from memory/wip notes.md ==
# func_800481E8 (text1b.c) — BLOCKED: forbidden dead-frame floor (canonical-asm)

## TL;DR
Target frame is **72 bytes** (`addiu sp,sp,-72`, saves s0/s1/s2/ra at 56/60/64/
68); GCC produces a **40-byte** frame. The ~32 extra bytes have **no recoverable
semantic local** — the function uses only registers + one stacked 5th call arg to
efc_buki_draw_zanzou. The 12 regfix rules patch the frame size (40->72) + all 8
register save/restore offsets (24->56, 28->60, 32->64, 36->68 and the lw mirror)
+ a prologue reorder + an INLINE_MOVE_ALIASING `addu $16,$18,$5 -> addu $16,$16,$5`.
HEAD also carries a `register asm("$16")` pin + `__asm__("move %0,%1")`
INLINE_MOVE_ALIASING in the body. Honest distance 14.

## Why it's not pure-C (FORBIDDEN class)
Per [[dead-vars-local-array]] (FORBIDDEN 2026-05-31): matching the 72-byte frame
requires either `s32 pad[8];` (forbidden dead local array — the detector refuses
completion) or the register pins / frame-size regfix (cheats). There is NO local
with clear semantic meaning that accounts for the 32 dead bytes. The rule's
explicit guidance for this exact situation: park / request canonical-asm
authorization; do NOT add a pad array.

Sibling precedent: `AddTbpOfst_80047EE8` (same file) was PARKED 2026-06-07 for
the identical "dead-vars-local-array frame floor" (commit 0244c4dd).

## Endpoint
Canonical-asm authorization (or a genuine identification of what the 32 frame
bytes semantically held in the original — none found; the call-loop uses no stack
buffer). Blocked. This is a user/orchestrator decision, not worker pure-C work.


- [s1] canonical: verdict C, distance 14 at start, 56 target insns

- [s1] Target frame equation: 72 = ALIGN8(vars=32) + args 24 + regs 16 (s0/s1/s2/ra @ 56/60/64/68); our build 40 with vars=0

- [s1] After s1 edits (applied in src/text1b.c): sandbox --disable all = 10, build_insns 56 == target 56; instruction-text diff shows exactly 2x addiu sp + 4x sw + 4x lw offset diffs and nothing else

- [s1] Prior WIP conclusion 'forbidden dead-frame floor / canonical-asm endpoint' is dissolved by phantom-frame-slots-gcc272 (post-dates it): vars>0 with zero dead decls is reachable pure-C (witness tslLineG5Init, COMPLETED-C, vars=8 untouched)

- [s1] No legitimate vars=32 witness in the sibling cluster: AddTbpOfst_80047EE8 (parked) and InitHiraRmd_80047FBC both get vars=32 via forbidden unused/dead arrays

- [s1] Fresh find_duplicates.py run: no near-duplicate lead for func_800481E8 (stale 0.625 lead to func_800483DC did not survive re-scan)

- [s1] Reusable frame gradient harness banked: tmp/grind/func_800481E8/s1/frame_probe.sh <file.c> prints per-function cc1 .frame vars=
