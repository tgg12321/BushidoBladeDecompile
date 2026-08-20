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


== s1b (recon, 2026-07-28, post-Judge-FAIL) ==

- [s1b] BASELINE re-confirmed: sandbox --disable all = 14 (51/51 insns, cheat_asm_stripped
  strips the on-main _pad[2]; pure save-offset diffs from frame 40 vs 48).

- [s1b] SOTN-MASTER CENSUS (Judge-mandated frontier item 3) — NEGATIVE on the deleted-call/
  args-area mechanism: zero >=5-arg dead calls anywhere in SOTN master. Genre-adjacent
  if(0) dead code in MATCHED code exists: e_hellfire_beast.c:827 `if (0) { while (posY); }`
  FAKE-annotated US PSX; e_collect.h:290 if(0) arm with real <=2-arg calls (beta);
  game_handlers.c:1465 empty if(0) arm; sel_psp/94D8.c:67 `if (0) { s32 var_s4 = 0; // fake }`.
  Genre precedent yes, args-area precedent no.

- [s1b] SPELLING GRID (project cc1, tmp/grind/file_LoadSectors/s1/probe2.sh):
  * `if (0) { debug5(fd,sector,count,0,0); }` -> cc1 output IDENTICAL to v_deadcall5.s
    (sandbox-0-proven form) except .file line. GCC 2.7.2 DOES expand if(0) bodies to RTL
    (args bump) before jump-opt deletes. p2_if0_call5.s.
  * `s32 dbg = 0; if (dbg) {...}` -> ALSO identical. Most human-plausible spelling
    (debug-flag local, RTL cse folds, zero residue). p2_dbgflag_call5.s.
  * unreachable call after `return -2;` -> label-numbering diffs only (byte-equivalent).
  * STRING LITERAL KILL: `if (0) { printf5("...", fd, sector, count); }` leaks the literal
    to .rodata even though the call is deleted (p2_if0_strlit.s). => the original's deleted
    call (if interpretation B) had SCALAR-ONLY args; printf-with-format-string originals
    are oracle-excluded. (NB that probe was a 4-arg call: frame stayed 40, confirming
    args bump needs >=5 args and the rodata leak is independent of the bump.)

- [s1b] OWNER-ESCALATION FILED: docs/grind/decisions.md "2026-07-28 — file_LoadSectors —
  OWNER-ESCALATION". Options: (a) dead 1-8B local under oversized-locals carve-out with
  partition-based prerequisite-1 reading; (b) narrow new-family sanction for the deleted-call
  spelling (census-negative, first-reach, honestly stated); (c) endgame-lock
  INCOMPLETE-owner-accepted keeping byte-correct _pad[2] on main. Function parked owner-gated.

- [s1] Baseline re-confirmed: sandbox file_LoadSectors --disable all = 14 (51/51 insns; strips the on-main _pad[2]; residual is purely save-offset diffs frame 40 vs 48)

- [s1] Frame dichotomy is complete and owner-decidable: target frame 48 admits exactly two source mechanisms — (A) args=16 + declared-unwritten 1-8B local, (B) args=24 via deleted >=5-arg scalar-only call; nothing in the bytes pins args (unlike func_80037540 whose addiu $a1,$sp,0x10 pinned args=16), so the 2026-07-13 oversized-locals carve-out prerequisite-1 is ambiguous here

- [s1] if(0) and dbg-flag-local spellings of the deleted call are byte-identical to the proven sandbox-0 form (only .file differs); unreachable-after-return differs only in label numbering

- [s1] String-literal args in deleted calls leak .rodata (measured), oracle-excluding printf-with-format-string originals; scalar-only args forced

- [s1] SOTN census negative on the args-area mechanism, positive on the if(0)-dead-code genre (4 instances with file:line in evidence.md s1b)

- [s1] OWNER-ESCALATION filed in docs/grind/decisions.md presenting options (a) dead-local under carve-out with partition-based prerequisite-1 reading, (b) narrow deleted-call family sanction (honestly census-negative), (c) endgame-lock INCOMPLETE-owner-accepted keeping byte-correct _pad[2] on main

== s2 (structural, 2026-08-20) — MATCH ==

- [s2] **SANDBOX 0 ON ORDINARY C.** Writing the sector-read loop as `for (i = 0; i < count;
  i++) { ... dest += 0x800; }` instead of the prior sessions' `i = 0; if (count > 0) { do {
  ... i += 1; } while (i < count); }` gives `sandbox file_LoadSectors --disable all` =
  **score 0, target_insns 51, build_insns 51, rules_dropped 0** in the REAL TU (body in
  place of the INCLUDE_ASM at src/ings.c:161). ZERO added constructs: no dead local, no dead
  call, no volatile, no alias, no FAKE annotation, no sanctioned-family claim. Banked as
  candidate.c; self-vet in self_vet.md.

- [s2] **MECHANISM (measured, not inferred).** cc1's own `.frame` comment is the instrument:
    do-while spelling : `.frame $sp,40,$31 # vars= 0, regs= 6/0, args= 16` -> sandbox 14
    for   spelling    : `.frame $sp,48,$31 # vars= 8, regs= 6/0, args= 16` -> sandbox 0
  The two cc1 outputs are instruction-for-instruction identical apart from the 14 frame-offset
  instructions and label numbering (diff of tmp/grind/file_LoadSectors/s2/sweep/v_base.s vs
  v_for_loop.s: only `.file`, `.frame`, sp +/-40 vs +/-48, the 12 save/restore offsets, and
  .L3/.L4 -> .L4/.L6). The vars=8 is the documented PHANTOM-FRAME artifact
  (memory/project/phantom-frame-slots-gcc272.md): GCC 2.7.2 allocates a stack temp while
  expanding the strength-reduced `for` induction variable, get_frame_size() counts it, and the
  allocator then keeps the value in a register, so no store is ever emitted. NO SOURCE OBJECT
  EXISTS for those 8 bytes.

- [s2] **THE s0/s1 PARTITION WAS FALSE.** Three sessions treated "target frame 48, body never
  touches sp+0..23" as proof that the original source contained a DEAD object, and closed the
  space to exactly two owner-refused families: (a) dead 1-8B local [vars axis], (b) deleted
  >=5-arg call [args axis, outgoing_args=24]. The partition omitted the third door — vars=8
  with NO declared object, produced by an ordinary loop spelling. This is precisely the
  failure mode phantom-frame-slots-gcc272.md was written to warn about ("the standing
  intuition ... is invalid reasoning; hunt the live-locals form first"), and it cost this
  function an owner escalation (docs/grind/decisions.md:1985, standing ruling 2026-07-28,
  option (c) INCOMPLETE-owner-accepted GRANTED). That disposition is now MOOT — the function
  matches in clean C. Recommend the operator note this on the decisions.md entry.

- [s2] **13-VARIANT STRUCTURAL SWEEP** (tmp/grind/file_LoadSectors/s2/sweep.py, table in
  s2/measurements.md). Frame equations measured for: base do-while (40/0/16), s16 local pair
  (48 via regs=7 — WRONG composition, 58 insns), s16 param pair (same), u16 local pair (48
  via regs=7, 56 insns), **for-loop (48, vars=8, 50 insns — THE MATCH)**, while-countdown
  (40), end-pointer walk (40), named intermediates off/len (48 via regs=7, 53 insns),
  block-scoped `i` inside the `if` (40 — scoping alone does NOT create the slot; the `for`
  induction expansion does), HImode-bitwise-on-real-data (48 via regs=7, 56 insns), DImode
  offset local (48 vars=8 but 52 insns — 2 junk insns), real 5-arg call (48 via args=24, 56
  insns), real 4-arg call with a double 4th arg (48 via args=24, 56 insns).

- [s2] **REAL-CALL ARGS-24 AXIS KILLED BY MEASUREMENT** (previously only argued): both
  v_real_5arg_call and v_real_double_arg do reach args=24/frame=48, and both cost +6
  instructions — a REAL >4-word call must store its 5th word at sp+16..23 and the target body
  has zero references there. Only a DELETED call reaches args=24 for free, which is the
  refused family (b). Moot now, but the axis is closed with numbers.

- [s2] **REGS=7 IS A FRAME-48 DECOY.** Five narrow-type variants hit frame 48 through regs=7
  (an extra callee-save), not through vars or args. That composition puts the saves at the
  wrong offsets AND costs 3-8 extra instructions; the target's 6 saves at sp+0x18..0x2C pin
  regs=24. Any future frame-48 hunt should read the full `vars/regs/args` triple, never the
  frame total alone.
