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

## s2 (structural, 2026-07-28) — floor unchanged at 10; the phantom-slot axis is MEASURED DEAD for this function

- [s2] Baseline re-verified with candidate.c applied to src/text1b.c: sandbox
  --disable all = 10, build 56/56 insns (rules_dropped 12, cheat_asm_stripped 323).
- [s2] **Frontier item 1 SUPERSEDED, not run as written.** The phantom slot is NOT
  assign_stack_temp during expansion — reproduced the tslLineG5Init trigger
  standalone (vars=8) and the lreg dump shows the orphan pseudo verbatim
  ("Register 77 used 2 times across 2 insns in block 0; dies in 0 places;
  ST_REGS or none") = the reload alter_reg stale-ref orphan already nailed by
  func_80037540 H14 (gdb-confirmed there). No expansion-site map is needed;
  the map's question is answered by the orphan's entry condition instead.
- [s2] **ENTRY CONDITION, BISECTED** (tmp/grind/func_800481E8/s2/minrepro.py +
  bisect2.py; 14 cases). The orphan fires iff combine DELETES an emitted 2-insn
  HImode→SImode sign-extension chain (ashift16/ashiftrt16 with an intermediate
  pseudo) AFTER flow counted refs. Triggers measured (all vars=8, orphan in
  lreg): T0 faithful tslLineG5Init; **T3 = same trigger with s16 locals loaded
  from u16 MEM via cast — OUR load shape, so lh loads are NOT required**;
  B2 wide bitwise + raw `&1` second use; **B3 = ONE variable, `if (v1 < 640)` +
  `if (v1 & 1)` — minimal known trigger, and the compare is literally our
  a3v < 0x280 shape**; B5 trigger coexisting with a LIVE emitted sll/sra of the
  same value; B7 HImode ADD low-bit consumer (bitwise NOT required); B8 `== 1`
  consumer. Non-triggers (vars=0): single consumer (T1); u16/zero-extension
  everywhere (T2, B6 — zero-ext is 1-insn andi or free, no intermediate pseudo
  exists to orphan); constant-&-only (T4); two extended-compare uses (T5);
  extended second use (B1); one ext use + raw use (B4 — clean lh conversion).
- [s2] **THE KILL: every deletion route is VISIBLE in the emitted bytes.**
  Either (a) the sll/sra chain is NOT emitted (high-bits-dead consumer), or
  (b) the lhu converts to lh, or (c) consumer insns (andi/branch/nor) appear
  that the target lacks. func_800481E8's target contains all 4 lhu loads, all
  4 sll/sra pairs LIVE (consumers: 3 call args, slti 0x280, sw of the 5th
  arg — every one needs correct high bits), and NO low-bit-only use of any
  halfword value. Therefore no honest spelling of THIS function's semantics
  can host a combine-deleted extension → no orphan → vars pinned at 0.
- [s2] Hand grid over the candidate chassis (probe.py, 11 variants): every
  s32-holder / redundant-(s16)-re-ext / staged-HI-view / zero-ext-recast
  spelling of the stream values gives vars=0; the holder spellings additionally
  convert lhu→lh (byte-break). Banked rejected/s32-holder-reext-no-orphan-lh-conversion.c.
  V5 (tree-fold) and V11 ((s16)arg1*4 prologue) are byte-identical controls at
  vars=0; V8 (`*p & ~3u` round) emits li/and — byte-break, dead.
- [s2] **BYTE-IDENTICAL ALTERNATIVE SPELLINGS BANKED** (equivalent-spelling
  class, cc1-output-identical to candidate): V6 `s32 a1w = *(u16*)p;` with call
  arg `(s32)(s16)a1w`; V7 `u32 a1u = *(u16*)p;` with arg `(s32)(s16)(u16)a1u`.
  Useful chassis variety for any future ruling-driven work.
- [s2] **QUANTITATIVE SWEEP — 52,043-iteration instrumented permuter campaign**
  (workspace tmp/grind/func_800481E8/s2/perm; compile.sh logs cc1 vars= for
  EVERY candidate and captures every vars>0 source; perm_pad_var_decl zeroed;
  honest --stack-diffs scorer, base 266). vars distribution over 42,374 logged
  compiles: 33,415×0, 8,718×8, 219×16, 12×24, **6×32**. All 8,644 captured
  vars>0 sources classified (classify_hits.py): 7,127 volatile-coercion, 1,457
  address-escape, **60 "clean" — ALL at vars=8 ONLY, all ≥33 non-sp body diffs,
  all semantically broken permuter mutations** (char locals, constant-folded
  strides, same-address double reads). ALL 6 vars=32 forms are volatile cheats
  (4-5 volatile decls each). ZERO clean forms reach vars∈[25,32]; ZERO clean
  forms near byte-identity. Only honest-score find: `volatile int new_var;`
  dead-pad (score 202) = the known cheat family, banked
  rejected/volatile-dead-pad-permuter-find.c. Campaign harvested + STOPPED
  (9 procs killed, 0 left).
- [s2] **FIVE-WAY FRAME TAXONOMY CLOSED for func_800481E8** (mirrors
  func_80037540 H21): frame 0x48 needs vars∈[25,32]; routes: (i) written
  aggregate → adds stores the 56-insn byte-matched target lacks; (ii) volatile
  → cheat, score-inert; (iii) address-escape → cheat + extra insns; (iv) stale-
  ref phantom → measured dead THIS session (no deletable extension exists);
  (v) unwritten tail → forbidden family (no dead stores in target, so the
  2026-07-01 carve-out does not apply). The frame axis has no sanctioned route
  left; this matches the pending file_LoadSectors owner question (same family,
  parked owner-gated 2026-07-27) and the endgame-lock-disposition standing
  policy surface.
- [s2] PROJECT-WIDE REUSABLE FINDING: B3 is the smallest known phantom trigger
  — a target containing a signed halfword compare PLUS a low-bit test of the
  same halfword value can host phantom frame slots in pure C. Relevant to the
  in-binary census (28 functions with untouched frame slack, func_80037540 s5).
- [s2] Instrument caveat: /tmp is unreliable in fresh WSL sessions here
  ("Failed to start the systemd user session" runs) — compile hooks must use
  repo-local temp files (mkperm.sh was patched accordingly).

- [s2] Baseline re-verified with candidate.c applied to src/text1b.c: sandbox --disable all = 10, 56/56 insns, rules_dropped 12, cheat_asm_stripped 323

- [s2] Five-way frame taxonomy CLOSED for func_800481E8: (i) written aggregate adds stores the byte-matched target lacks; (ii) volatile = cheat (score-inert, sandbox strips it); (iii) address-escape = cheat + extra insns; (iv) stale-ref phantom measured dead this session; (v) unwritten tail forbidden (target has no dead stores, 2026-07-01 carve-out inapplicable)

- [s2] Campaign telemetry: base 266 (honest stack-diffs scorer), 52,043 iters, 1 find (volatile dead-pad, 202, 6.7min), harvested + STOPPED, 9 procs killed, 0 orphaned

- [s2] vars distribution over 42,374 logged compiles: 33,415x0 / 8,718x8 / 219x16 / 12x24 / 6x32; every vars=32 form is the volatile-coercion family

- [s2] Byte-identical alternative spellings banked for the stream reads (V6: s32 raw holder + (s32)(s16) at call; V7: u32 holder + (s32)(s16)(u16)): equivalent-spelling class for future ruling-driven work

- [s2] Project-wide reusable: B3 is the smallest known phantom trigger (signed halfword compare + low-bit test of same value) — applicable to the 28-function untouched-frame-slack census

- [s2] Instrument caveat: /tmp unreliable in fresh WSL sessions (systemd user session failure); compile hooks must use repo-local temps

- [s2] Same owner-question family as file_LoadSectors (parked owner-gated 2026-07-27) and the endgame-lock-disposition standing policy (2026-07-20)
