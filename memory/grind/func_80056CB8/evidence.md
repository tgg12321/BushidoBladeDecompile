# Evidence — func_80056CB8 (src/text1b.c)

## [s1] recon 2026-09-16

**Function status:** NOT YET STARTED. `src/text1b.c` carries
`INCLUDE_ASM("asm/funcs", func_80056CB8);` (asm-until-matched state, no
cheats). `sandbox --disable all` reports `no_c_body: true`, `score: 204`
(== `target_insns`) — the "distance" is simply the raw target instruction
count, not a diff against any candidate C. This is a from-scratch
reconstruction, not a residual-chasing session.

**Signature (confirmed via caller + asm):** `void func_80056CB8(void *arg0)`
(possibly a typed character/fighter struct pointer). Caller evidence:
`memory/grind/func_80055B60/evidence.md:13` region R9 — `func_80055B60`
calls it as `jal func_80056CB8(s2)` (single arg, plain jal, return value
unused at the call site). Asm prologue: `$a0` is moved into `$s7` and held
live across the whole function (`addu $s7,$a0,$zero` at
`asm/funcs/func_80056CB8.s:3`), consistent with a single struct-pointer
parameter used throughout.

**OBJECT MODEL:** (per the brief's mandatory recon requirement — every
flagged DATA MODEL symbol addressed)

- `D_8009A820` (byte, MATCHES): already `extern u8 D_8009A820;` at
  `src/text1b.c:2183`. No header decl, but the in-TU convention for this
  exact class of byte table is already established just above in the same
  file: `func_80056FE8`'s body (`src/text1b.c:1867-1874`) reads sibling
  tables `D_8009A830`/`D_8009A838`/`D_8009A840` via
  `*(u8 *)(((s32)(&D_8009A83X)) + idx)` pointer-cast arithmetic rather than
  a declared array. Our target's asm does the identical thing:
  `lui $at,%hi(D_8009A821); addu $at,$at,$fp; lbu $v0,%lo(D_8009A821)($at)`
  (`asm/funcs/func_80056CB8.s:29-32`) — a `%hi/%lo`-relocated base address
  plus a register offset, i.e. exactly `*(u8 *)((s32)&D_8009A821 + fp)`.
  No declaration change needed; write the read in this file's own idiom.
  Score not separately measurable in isolation (whole-function distance is
  204 either way at this stage) — logged as MATCHES-unmeasured (isolating
  this one symbol from a 204-insn from-scratch body isn't meaningful; the
  measurement that matters is the eventual full-body sandbox score).
- `D_8009A821` (byte, MATCHES): identical situation, same idiom, same file
  region (`src/text1b.c:2184`), same asm shape
  (`asm/funcs/func_80056CB8.s:56-59` — the second array read at
  `.L80056DA8`).
- `D_800F6608` (Rec44 struct, MATCHES + naming correction): already
  `extern Rec44 D_800F6608;` in `include/code6cac.h:283`,
  `w0`/`w4`/`w8`/`wC` are `s32` members at struct offsets 0/4/8/0xC
  (`code6cac.h:265-274`). The census's separate symbol `D_800F6610` is
  `D_800F6608 + 0x8` — i.e. it is NOT a distinct global, it is
  `D_800F6608.w8`. Confirmed both arithmetically (`0x800F6610 -
  0x800F6608 == 8`) and by exact sibling precedent:
  `func_80057094` (`src/text1b.c:1889`, matched, in the SAME file) reads
  `D_800F6608.w0 - *(s32*)(arg0+0xF4)` and `D_800F6608.w8 -
  *(s32*)(arg0+0xFC)` — the identical midpoint-anchor `ratan2` idiom our
  target's asm uses at `.L80056D6C` (`asm/funcs/func_80056CB8.s:50-57`:
  `lw $v1,%lo(D_800F6608)($v1)` for w0, `lw $v0,%lo(D_800F6610)($v0)` for
  w8, then `jal ratan2` with `subu`-computed deltas against
  `0xF4($s1)`/`0xFC($s1)`). **This IS the DATA MODEL declaration-fix
  hypothesis** (hypothesis #1 below) — write `D_800F6608.w8`, never
  declare a new `D_800F6610` symbol.
- `Judge` (s16, MATCHES): already `extern s16 Judge;` declared twice in
  this TU (`src/text1b.c:1967`, `:2169`). It is a sine/cosine-style lookup
  table accessed via raw pointer arithmetic on `&Judge`, NOT array
  indexing syntax — and this file already has the exact idiom matched and
  in production at `func_80057CC8` (`src/text1b.c:2045-2046`):
  `*(&Judge + (ang_mid & 0xFFF))` / `*(&Judge + (((s16)ang_mid + 0x400) &
  0xFFF))`. Our target's asm computes the identical shape twice
  (`.L80056D94`-area, `asm/funcs/func_80056CB8.s:61-90`): a 12-bit-masked
  angle word shifted left 1 (`andi $v0,$s0,0xFFF; sll $v0,$v0,1`) added to
  `%hi/%lo(Judge)`, then `lh` — i.e. `*(&Judge + (angle & 0xFFF))` — done
  TWICE, once for the base angle (`$s5`, offset 0) and once for angle+0x400
  (`$s4`, offset `+0x400` before the same mask), mirroring
  `func_80057CC8`'s `ang_mid` / `ang_mid+0x400` pair exactly. **Hypothesis
  #2 below: reuse this exact idiom verbatim for the two Judge reads.**

**Structural map of the asm (204 insns, `asm/funcs/func_80056CB8.s`):**
- Prologue: saves `$s0-$s7,$fp,$ra` (0xA8-byte frame), `$s7 = arg0`.
- `v1 = (lhu arg0[0x3E8]) & 3` (a 0..3 mode/counter field); `s6 = v1<<1`
  (loop start counter, saved to `0x60($sp)`), `fp = v1<<2` (byte-table
  index base, `%hi/%lo` relocation offset for the `D_8009A820/21` reads).
  Sets up three saved stack pointers (`0x68/0x70/0x78($sp)`) to
  `sp+0x28`, `sp+0x58`, and the scratchpad address `0x1F8002B8` — the
  scratchpad literal is almost certainly an argument to `func_80053614`
  (see below), not a scratchpad POKE by this function itself (no
  `mtc2`/`swc2`/cop2 opcodes anywhere in the 218-line file — this is
  ordinary ALU/mult/branch code, ZERO GTE-band signals, `scan_hand_coded`
  tier LOW per the canonical gate, so this is an ordinary ratan2/table
  lookup ALU function, not canonical-asm territory).
- **Outer loop `.L80056D24 .. .L80056FB4`** runs exactly TWICE
  (`s6` starts at `v1<<1`, increments by 1 each iter, loop test is
  `s6 < initial_s6 + 2`; `fp` increments by 2 each iter, NOT 4 — so the
  two `D_8009A820/21` byte-table reads inside the loop use a *different*
  index stride (`fp`, +2/iter) than the byte-table's own natural element
  size implies; this needs a probe next session, not assumed).
- Inside the loop: reads `arg0[0x6A]` (`s16`, compared against 0x13/6 to
  pick between `arg0->field0x1CA` [dereferenced through `$s1`, itself set
  from either `arg0` directly or `*(s32*)arg0` gated by a `D_8009A821`
  byte's bit `0x1000`] and a `ratan2(D_800F6608.w0 - s1[0xF4], D_800F6608.w8
  - s1[0xFC])` call) to build an angle word `s0`.
  Two Judge-table lookups (`s5`/`s4`, angle and angle+0x400) feed two
  `mult` ops against a `D_8009A820`-derived scaled byte, producing values
  written into a `0x28/0x2C`-based local stack struct (two `sra 12`
  fixed-point scalings against `s1[0xB8]`/`s1[0xBC]`, both offset by
  `-0x320`).
  Calls **`func_80053614`** TWICE per iteration (already matched, in this
  TU: `void func_80053614(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32
  arg4)`, `src/text1b.c:1513`) with a 5-slot stack-built argument block
  (`0x10/0x18/0x1C/0x20/0x28/0x2C/0x30($sp)` written before each call) —
  this is a coordinate/rect probe helper, return value accumulated into a
  bitmask `s0` (`or`, then `+1`).
  A conditional block (only when the first `func_80053614` call fails —
  `beqz $s0`) does an angle-blend fixed-point computation reading
  `0x0($s5)`/`0x0($s4)` a second time with a `*24` (`x*5-x, <<2, +x`
  =`*31`? — actually `(x<<5 - x)<<2 + x = (31x)<<2+x = 125x`... needs a
  careful re-derivation next session, NOT assumed) scale factor.
  After the two `func_80053614` calls, a distance/threshold gate chain
  (`0x38/0x40($sp)` vs `arg0[0xB8]/[0xC0]`, squared and summed, compared
  against `0x3D0900`) followed by a second gate (`arg0[0xBC]` vs
  `arg0[0x4C]`, range `[0,0x3E8]` either direction) picks a small integer
  `s0 in {0,4,5}` written to `arg0[0x444 + s6]` (byte array indexed by the
  outer loop counter, NOT `fp`).
- Epilogue: standard restore, `jr $ra`.

**Callee inventory:** `ratan2` (external, s32 atan2-style, used elsewhere
in this TU), `func_80053614` (matched, in-TU, 5 args). No unmatched
callees — nothing blocks a from-scratch C reconstruction on the callee
side.

**No canonical-asm signal.** `canonical func_80056CB8` → verdict `C`,
`hand_coded_tier: LOW` (no S1/S2/S6 signal). `distance 204` is
whole-function size, not hand-asm evidence
([[canonical-gate-distance-not-evidence]]). This is an ordinary — if
large — pure-C reconstruction target. No GTE/cop2 opcodes, no redundant
masks (packed-multiply-cluster S8 signal absent), no hardcoded-register
tells.

- [s1] sandbox --disable all: no_c_body=true, score=204=target_insns (function has never had a C draft; distance is raw size, not a residual)

- [s1] canonical: verdict C, hand_coded_tier LOW, no S1/S2/S6 signal - ordinary pure-C target, not canonical-asm territory

- [s1] signature confirmed via caller (func_80055B60 region R9, memory/grind/func_80055B60/evidence.md:13): void func_80056CB8(void *arg0), called as func_80056CB8(s2), return value unused

- [s1] D_8009A820/D_8009A821 byte tables already extern u8-declared in this TU (src/text1b.c:2183-2184) with an established cast-pointer-arithmetic access idiom used two functions earlier in the same file (func_80056FE8, src/text1b.c:1867-1874) for the sibling tables D_8009A830/38/40

- [s1] D_800F6608 (Rec44) already declared in include/code6cac.h:265-283; D_800F6610 is D_800F6608.w8, not a separate symbol (see hypothesis 1)

- [s1] Judge already declared extern s16 Judge (src/text1b.c:1967,2169) with a matched pointer-arithmetic table-lookup idiom in func_80057CC8 (src/text1b.c:2045-2046) directly reusable for this function's two lookups

- [s1] callee func_80053614 is matched/in-TU (src/text1b.c:1513, 5 args: s32*,s32*,s32,s32,s32), called twice per outer-loop iteration with a stack-built 7-word argument block; ratan2 is the other callee, external, used elsewhere in this TU - no callee blocks a from-scratch draft

- [s1] outer loop runs exactly twice (s6 starts at (arg0[0x3E8]&3)<<1, increments by 1, loop test s6 < initial+2); fp (byte-table index base) increments by 2/iter - open structural question, see hypothesis 3

- [s1] zero GTE/cop2 opcodes, zero redundant pre-shift masks (packed-multiply-cluster S8 signal absent) anywhere in the 204-insn body - ordinary ALU/mult/branch/call code

## [s2] structural 2026-09-16

**First C draft applied.** The s1 ledger's inherited
`authored-draft-2026-08-18.c` (from a PRE-asm-until-matched authoring
session that never got applied to src/ — the function was still
`INCLUDE_ASM` going into this session) was applied to `src/text1b.c`
verbatim, plus one prerequisite fix: `func_80053614`'s declared return
type in this TU was `void` but the target's two calls to it in
`func_80056CB8` consume `$v0`. Changed to `s32 func_80053614(...)` with
`return func_80052D00(arg2, arg3);` as its last statement.
**Verified byte-neutral**: `sandbox func_80053614 --disable all` still
scores `0/32` after the change (func_80053614's own asm falls through
$v0 to the epilogue regardless of the C-level return type, so this was
never actually a behavior change — just fixing a signature that
happened not to matter for the void-caller case).

`sandbox func_80056CB8 --disable all` progression this session:
- 204 (no_c_body) -> 126 (first draft applied, `work[2]`)
- 126 -> 106 (`work` widened to `s32 work[4]`, resolving
  authored-notes-2026-08-18.md [S7]: `work` is 16 bytes not 8. Confirmed
  via the `.frame` dump: `vars=104, frame=168(0xA8)` now EXACTLY matches
  target's frame size, computed independently from the asm's saved-reg
  offsets (0x80-0xA7) + spill slots (0x60-0x7F) + 5 local blocks
  (0x18-0x5F) = 0xA8 total.)

**Structural match confirmed via side-by-side dump comparison**
(`tmp/grind/func_80056CB8/dumps/text1b.s` vs `asm/funcs/func_80056CB8.s`):
every region — the obj-select branch, the type-gated angle source
(facing angle vs ratan2 fallback), both Judge-table lookups, the pt0/pt1
fill including the DOUBLE reads of `obj+0xB8` and `obj+0xC0` (our C
already double-reads these the same way target does, at the same
points), the `*0x7D` (125) fallback expansion via
`sll5;subu;sll2;addu;sra8` (identical to target, confirming the
authored-notes [S3] guess was correct — GCC's own constant-multiply
strength reduction reproduces the exact target shape from
`(v * 0x7D) >> 8`), both `func_80053614` calls, and the disposition gate
chain (`code` in {0,3,4,5}) all match target's mnemonic sequence and
branch structure region-by-region. `hand_coded_tier` stays LOW; no
GTE/cop2 signal appeared anywhere in the build.

**Residual is register allocation, not structure.** Mapping the s0-s7
role assignment: target has `flags`/`r1` sharing `$s0`, `obj=$s1`,
`x=$s2`, `z=$s3`, `cos_p=$s4`, `sin_p=$s5`, loop-`i`=`$s6`, `arg0=$s7`.
Our build has `obj=$s0`, `flags`/`z` sharing `$s1`, `x=$s2` (MATCHES),
`cos_p=$s3`, loop-`i`=`$s4`, `sin_p=$s5` (MATCHES), `r1=$s6`,
`arg0=$s7` (MATCHES). Only 3 of 8 roles land in the same hard register;
the rest are systematically shifted. This produces per-instruction byte
diffs even where the mnemonic sequence is identical (different `sw
$sN,...` register field), which is almost certainly the bulk of the
106-instruction sandbox score.

**KILLED: statement-order swap does not affect this allocation** — see
hypotheses.md. Tried computing `obj = arg0;` before `flags = ...;`
(the reverse of the authored draft's order, which computed `flags`
first) hypothesizing that GCC's pseudo-register numbering (and thus
hard-register assignment) follows source assignment order closely
enough that swapping which local is assigned-to first would flip which
of `obj`/`flags` gets the lower-numbered pseudo and thus (plausibly) a
different hard register. Measured: ZERO change in the dumped register
assignment (`obj` stayed `$s0`, `flags` stayed `$s1`) and ZERO change in
sandbox score (stayed 106). This is an INSTANCE kill on this specific
statement-order lever for this specific pair of variables — it does
not prove no C-level lever exists, only that this one didn't move
these two pseudos' hard-register assignment on this chassis.

- [s2] func_80053614 (matched, in-TU) return type void->s32 fix is byte-neutral (sandbox 0/32 before and after) - was a pure signature correction, not a behavior change
- [s2] first full draft (authored-draft-2026-08-18.c + the func_80053614 fix) drops honest floor 204 -> 126
- [s2] widening `work` from 8 to 16 bytes (`s32 work[4]`) drops floor 126 -> 106 and makes the function's `.frame` size (168/0xA8) match target exactly - resolves authored-notes [S7]
- [s2] side-by-side dump comparison confirms full structural match (branch shape, double-reads, *0x7D expansion, disposition gate chain) - residual is register-allocation only, not missing/wrong structure
- [s2] KILLED (instance): swapping the `obj`/`flags` first-assignment order does not change either variable's hard-register assignment or the sandbox score - global allocation for this shape is not driven by this statement-order lever

- [s2] func_80056CB8 had no candidate.c and was still INCLUDE_ASM going into this session, despite an unapplied authored-draft-2026-08-18.c sitting in the ledger from a pre-asm-until-matched authoring pass -- applying it was the session's first and highest-value move.

- [s2] func_80053614 (matched, in-TU, called twice by func_80056CB8) was declared void in src/text1b.c but the target's calls to it consume $v0; changing its return type to s32 with an explicit return statement is verified byte-neutral (sandbox func_80053614 --disable all: 0/32 both before and after) because its own asm falls through $v0 to the epilogue regardless of declared type.

- [s2] Side-by-side comparison of tmp/grind/func_80056CB8/dumps/text1b.s against asm/funcs/func_80056CB8.s shows the candidate's instruction sequence, branch structure, double-reads of obj+0xB8/obj+0xC0, the *0x7D fallback expansion (sll5;subu;sll2;addu;sra8), both func_80053614 call sites, and the full disposition gate chain all match target region-by-region -- the remaining 106-instruction sandbox score is attributable to register allocation (which s0-s7 slot each pseudo lands in), not missing or incorrectly-shaped C structure.

- [s2] Frame size now matches target exactly: 168 bytes (0xA8), vars=104, with pt0/pt1/hit0/hit1/work at the same relative offsets (0x18/0x28/0x38/0x48/0x58) as the target's implied layout.

- [s2] hand_coded_tier stays LOW; zero GTE/cop2 instructions anywhere in the candidate or the target asm -- this remains an ordinary pure-C register-allocation problem, not a canonical-asm candidate.

- [s3] Dossier confirms src/text1b.c still carries INCLUDE_ASM("asm/funcs", func_80056CB8); for this function -- no C has landed on main; the candidate.c body only exists in the ledger.

- [s3] Read tmp/grind/func_80056CB8/dumps/text1b.greg (regenerated this session via `pwsh tools/grinder/dump.ps1 func_80056CB8`), function block starting line 14788: the 23-pseudo register-disposition list maps obj=pseudo82->$s0(16), flags=pseudo83->$s1(17, shares with a later pseudo consistent with z), x=pseudo88->$s2(18, MATCHES target), cos_p=pseudo87->$s3(19), i(loop)=pseudo74->$s4(20), sin_p=pseudo86->$s5(21, MATCHES target), r1=pseudo90->$s6(22), arg0=pseudo72->$s7(23, MATCHES target) -- pinning the candidate.c header's prose register-ring description to exact pseudo numbers for future sessions.

- [s3] Three independent source-order permutation probes across s2 (statement-order swap of obj/flags) and s3 (declaration-order swap of obj/flags; declaration-order move of r1 next to flags) all measured ZERO effect on score or register assignment -- source order among these three locals is not the lever; the next axis is live-range/conflict-graph shape (register-alloc-pure-c Lever A/B), not reordering.

- [s3] func_80053614's return-type fix (void -> s32) is confirmed byte-neutral standalone (sandbox func_80053614 --disable all: 0/32 both before and after) and is a hard prerequisite for func_80056CB8's candidate to reach 106 rather than 147 -- its asm falls through $v0 to the epilogue regardless of declared return type, so the fix only affects callers that now read the return value.

- [s3] Working tree left clean at session end (src/text1b.c reverted to committed INCLUDE_ASM state via `git checkout`); only memory/grind/func_80056CB8/{candidate.c,hypotheses.md} were modified and are part of this session's ledger update.

## [s4] permuter 2026-09-16

**Chassis check confirmed floor 106.** Applied candidate.c (unchanged from s2/s3) plus
the func_80053614 void->s32 prerequisite fix to src/text1b.c; `sandbox func_80056CB8
--disable all` reproduced score 106 (build_insns 201, target_insns 204) before any
permuter work started -- no chassis drift since s3.

**Built a fresh single-function permuter workspace** at
`tmp/grind/func_80056CB8/s4/perm_ws` (this function had never had a permuter campaign
before this session -- the CHASSIS RULE 20k-iteration re-seed restriction does not
apply). base.c is a preprocessed (`mipsel-linux-gnu-cpp`) TU-prefix of src/text1b.c
truncated right after func_80056CB8's closing brace (Python brace-matcher, see
`tmp/grind/func_80056CB8/s4/build_ws.sh`); target.o is `asm/funcs/func_80056CB8.s`
assembled standalone at offset 0 via `tools/decomp-permuter/prelude.inc` (gp=64 line
stripped for r3000); compile.sh mirrors the Makefile's exact per-file pipeline (cc1
-O2 -G0 ... -mel -msoft-float | prologue_fix.py | maspsx.py --expand-div
--aspsx-version=2.34 [+ sdata/expand-lb/multu config lists] | multu_pad.py |
mipsel-linux-gnu-as), then isolates the `.ent func_80056CB8` .. `.end func_80056CB8`
region via awk before final assembly. Validated: base insns 201 / target insns 204,
exactly matching the engine sandbox's build_insns/target_insns.

**Campaign result: NEGATIVE, banked as an instance kill.** Launched with
`-j 6 --stop-on-zero`, base permuter-weighted score 5235. Ran 44,294 iterations over
~24 minutes (two overlapping in-session `wait` polling loops, both harvested cleanly;
no orphan -- `harvest --dir tmp/grind/func_80056CB8/s4/perm_ws --stop` confirmed the
campaign dead afterward). Best score found: 4103/5235 (21.6% reduction from base),
plateaued at 4103 for the back half of the run with no further novel-and-better find.
The 4103 form itself (`output-4103-1/diff.txt`) is a type-broken mutation (unused
`u8 *new_var` alias, uncast `*(obj + 0xC0)` dereference) -- not a valid intermediate
lever, just permuter noise. No output across the full run approached score 0 or the
residual's actual scale. See hypotheses.md [s4] for the full mechanism writeup: the
permuter's mutation set (semantic-changing random edits) is poorly matched to a
residual that is PURELY hard-register assignment on an already-structurally-matching
body -- most mutations regress the weighted score by changing instruction count
rather than exploring register-allocation-neutral rephrasings.

- [s4] Chassis re-confirmed at floor 106 before permuting (candidate.c + func_80053614 fix, no drift since s3)
- [s4] First-ever permuter campaign for this function: tmp/grind/func_80056CB8/s4/perm_ws, 44,294 iterations, base score 5235, best found 4103 (21.6% reduction), plateaued, zero near-zero or valid-closing finds
- [s4] Campaign telemetry logged via tools/permuter_campaign.py (permuter-launch/permuter-harvest events, metrics/events.jsonl); campaign stopped and harvested in-session, no orphan
- [s4] Working tree left clean at session end (src/text1b.c reverted to committed INCLUDE_ASM state); only memory/grind/func_80056CB8/{evidence.md,hypotheses.md} and tmp/grind/func_80056CB8/s4/* (scratch artifacts) were touched this session -- candidate.c unchanged from s2/s3 (permuter found no improvement to bank)

- [s4] Chassis re-confirmed floor 106 (build_insns 201 / target_insns 204) before permuting -- no drift since s3.

- [s4] First-ever permuter campaign for func_80056CB8: 44,294 iterations, base permuter-weighted score 5235, best found 4103 (21.6% reduction), plateaued, zero near-zero or structurally-valid finds.

- [s4] The plateaued 4103 form is type-broken (unused alias pointer + dropped pointer cast), not a usable intermediate lever -- inspected directly in output-4103-1/diff.txt.

- [s4] Campaign stopped and harvested in-session (no orphan); working tree left clean at session end (src/text1b.c reverted to committed INCLUDE_ASM state).
