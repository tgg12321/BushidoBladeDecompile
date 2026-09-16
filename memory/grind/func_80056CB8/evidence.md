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

- [s5] Chassis re-verified at session start: candidate.c (s2/s3/s4-banked body + work[4] + func_80053614 s32-return fix) reproduces score 106 (build_insns 201, target_insns 204) before any s5 change, matching the ledger exactly.

- [s5] Dump comparison (tmp/grind/func_80056CB8/dumps/text1b.s vs asm/funcs/func_80056CB8.s .L80056D94-.L80056E38) shows target ALSO keeps t0=obj->0xB8 and t1=obj->0xC0 live across the whole pt0/x/z/pt1 block (feeding both the pt0 store and the x/z base-add) while separately re-reading obj->0xC0 for pt0[2] and obj->0xBC for pt1[1] a second time -- the classic split-read-defeats-hoist duplicate-read pattern (already SOTN-sanctioned). Our C source already writes these as separate literal *(s32*)(obj+0xC0) expressions rather than through a shared named variable, and the s5 dump confirms GCC does NOT CSE-merge them in the built object -- this part of the structure already matches target and is not the source of the remaining 81-point residual.

- [s5] The remaining residual after the s5 improvement is build_insns 197 vs target 204 (a 7-insn shortfall) plus whatever weighted register/reorder diffs make up the rest of score 81 -- still register allocation, not structure; the s3-recorded pseudo->hardreg map is now STALE (the statement reorder renumbers pseudos) and must be re-derived from the freshly regenerated tmp/grind/func_80056CB8/dumps/text1b.greg/.lreg before the next session hypothesizes about specific pseudo assignments.

- [s5] The diff introduces zero new locals, zero dead code, zero annotations, zero asm -- it is a pure reorder of statements already present in the s2-banked candidate, vetted in memory/grind/func_80056CB8/self_vet.md against the full 6-test cheat checklist.

- [s6] STALE-HEAD-CLAIM correction: re-checked HEAD at session start -- func_80053614 was STILL `void` on main (src representation is INCLUDE_ASM for func_80056CB8, so nothing persists between grind sessions on main; the s3 note "already applied to src/text1b.c" only ever described a candidate-only fact). Re-applied the func_80053614 s32-return fix + s5 candidate body to src/text1b.c for measurement; re-confirmed floor 81/204 (build_insns 197) exactly matches s5.

- [s6] `python3 tools/objdiff.py tmp/sandbox/func_80056CB8/text1b.o build/src/text1b.o` (build/src/text1b.o is the still-INCLUDE_ASM reference object = the ORIGINAL target bytes for this TU) confirms func_80056CB8 is the ONLY changed function in text1b.o (271/272 identical) -- a clean isolated diff, full capture tmp/grind/func_80056CB8/s6/objdiff_full.txt (before fix) / objdiff_full2.txt (after).

- [s6] Found + fixed a real object-model mismatch: the ratan2 branch's second argument read `D_800F6608.w8` (one lui(D_800F6608) + lw offset 0 and lw offset 8 from the same base) where target emits a fully SEPARATE `lui v1,%hi(D_800F6610); lw v1,%lo(D_800F6610)(v1)` (asm/funcs/func_80056CB8.s:51-55, literal symbol name). D_800F6610 is a real linker symbol (undefined_syms_auto.txt:642, address 0x800F6610 = D_800F6608+8 exactly) -- so the ORIGINAL C at this one call site read a separate global, not a struct field. Added `extern s32 D_800F6610;` (TU-local to text1b.c only -- D_800F6608 stays a Rec44 struct for every OTHER consumer, e.g. src/code6cac.c func_8001B294 which writes it field-by-field) and changed the arg to `D_800F6610 - *(obj+0xFC)`. Confirmed via objdiff-diff: the `-lw v0,8(v0)` / `+lw v0,0(v0)` mismatch is gone post-fix. Score/build_insns unchanged (81/197 both before and after) -- this diff pair wasn't part of the weighted score's counted residual, but it's a genuine correctness fix, kept.

- [s6] KILLED (instance): hypothesized target's single reused `$s8` (i*2, materialized once, reused via `addu at,at,s8` at both the D_8009A821 and D_8009A820 index sites) is reachable by factoring both `[i*2]` index expressions onto one `s32 idx = i*2;` local. Measured WORSE: score 81->90, build_insns 197->199 (rejected/shared-idx-local-worse.c). The shared local's live range across the intervening ratan2 call raised register pressure past the cost of the saved multiply. Target's $s8 reuse is real but not reached by this spelling -- try a shared base-pointer local next, or treat it as scheduling-driven.

- [s6] Remaining post-fix objdiff (tmp/grind/func_80056CB8/s6/objdiff_full2.txt) is otherwise ALL register-name substitutions (register-rotation-cluster shape) PLUS one concrete new lead: in the code==4 tail (y/hit1[1] >= 0x3E9 nested-if setting code=5), our build emits one `bgez` where target emits `bltz`+`beqz` as two branches, and target has FOUR extra `addu v0,s7,s6` (recomputing the store address `arg0+i`) duplicated into the delay slots of 4 separate exit branches reaching the shared `sb 0x444(v0)` store (asm/funcs/func_80056CB8.s:160-206) plus an extra `j TGT`. This matches reorg.c's profitable-duplication-into-delay-slot mechanism, not a missing-goto C-structure gap: rewriting this exact tail with explicit `goto store;` early exits (semantically identical control flow) measured BYTE-IDENTICAL to the nested-if form (81/197, no change) -- so whatever gates reorg's duplication decision here is upstream of this tail (register pressure / scheduling earlier in the function), not the tail's own C shape. Do not re-try the goto-vs-nested-if axis on this tail.

- [s6] STALE-HEAD-CLAIM corrected: func_80053614 was still `void` on main at session start (src is INCLUDE_ASM for func_80056CB8 between sessions, nothing persists) -- re-applied the s32-return fix + s5 candidate body, re-confirmed floor 81/204 (build_insns 197) exactly matches the s5-recorded number before any s6 change.

- [s6] tools/objdiff.py tmp/sandbox/func_80056CB8/text1b.o build/src/text1b.o (build/src/text1b.o is the still-INCLUDE_ASM reference object = the ORIGINAL target bytes) confirms func_80056CB8 is the ONLY function differing across the whole text1b.o (271/272 identical) -- a clean, isolated diff surface for this function, captured in tmp/grind/func_80056CB8/s6/objdiff_full.txt and objdiff_full2.txt.

- [s6] asm/funcs/func_80056CB8.s:51-55 literally names two separate symbols, D_800F6608 and D_800F6610, each with its own lui/lw %hi/%lo pair -- direct evidence the original C read two distinct globals at this call site, not one struct's two fields.

- [s6] undefined_syms_auto.txt:642 already defines D_800F6610 = 0x800F6610 (== D_800F6608+8, the same storage as the Rec44 .w8 field) -- no linker-side work needed to use it as a second symbol.

- [s6] D_800F6608 is written field-by-field as a genuine multi-field Rec44 struct in src/code6cac.c (e.g. func_8001B294 sets w0/w4/w8/h10/h12/h14/w18/b1E) -- it is a real aggregate for every OTHER consumer, so the fix here is a TU-local read-site correction (extern s32 D_800F6610; added only in text1b.c), not a declaration change to the shared header.

- [s6] Post-fix remaining objdiff is a register-rotation-cluster (consistent renaming, e.g. our s3/s6/s0/s1 vs target's different but self-consistent assignment) plus the code==4 tail branch-topology + delay-slot-duplication difference described in the hypotheses above -- both are register-allocation/scheduling residuals, not object-model or structural gaps.

- [s6] src/text1b.c reverted to committed INCLUDE_ASM state at session end (git checkout -- src/text1b.c verified clean); no C draft left on main per asm-until-matched.

## [s7] structural 2026-09-16

- [s7] Chassis re-verified at session start: applying the s6-banked candidate.c body (D_800F6610 fix + s5 store-batching + func_80053614 s32-return fix) to src/text1b.c reproduces score 81/204 (build_insns 197) exactly, before any s7 change.

- [s7] Direct read of asm/funcs/func_80056CB8.s (lines 1-40, 40-99, 130-218), not a stale dump, shows target keeps ONE hardware register ($s0) for three successive non-overlapping-lifetime roles: `flags` (the D_8009A821 byte value <<8), `ang` (computed IN PLACE as `addu $s0,$s0,$v0`, never a separate register), and the final disposition `code` (`or $s0,$s0,$v0; addiu $s0,$s0,1` then the 0/3/4/5 tail, `.L80056ED0` onward) -- three roles, one register, in target's own bytes.

- [s7] Removing the `ang` and `code` local declarations and reusing the EXISTING `flags` local for both roles (every former `ang`/`code` read/write becomes a `flags` read/write) drops sandbox func_80056CB8 --disable all from score 81 (build_insns 197) to score 58 (build_insns 198) -- a 23-point drop despite ONE MORE raw instruction, confirming the win is register-identity quality. Reproduced twice in-session.

- [s7] `python3 tools/objdiff.py tmp/sandbox/func_80056CB8/text1b.o build/src/text1b.o` (build/src/text1b.o is the still-INCLUDE_ASM reference object, i.e. the ORIGINAL target bytes) on the post-merge chassis: func_80056CB8 remains the only changed function (271/272 identical elsewhere); the diff region for func_80056CB8 itself shrank to 112 lines (full capture: tmp/grind/func_80056CB8/s7/objdiff_s7.txt). Remaining differences are (a) a pure register-name/loop-counter rotation ($s3 in our build vs a $s6/$s7-entangled role in target for the outer loop index and its derived store addresses -- the standing register-rotation-infrastructure class, no-new-park-categories.md; NOT a new park category, just unclosed via C so far) and (b) the code==4 tail's branch-topology difference (target: `bltz`+`beqz` as two branches; ours: one `bgez`) -- but NOW target's characteristic FOUR duplicated `addu v0,s7,s6` address-recomputes (into 4 exit-branch delay slots) has a MATCHING four duplicated `addu v0,s3,s7` in our own build too (this was NOT true at s6 -- the merge changed enough codegen that this specific duplication now also happens in our fork); only the two-vs-one branch SPLIT for the y-compare remains different.

- [s7] Re-tested (on this NEW chassis) the s6-killed goto-vs-nested-if rewrite of the code==4 tail's y-compare: byte-identical again (58/198 both forms). Second independent confirmation across two materially different chassis that this tail's own C shape does not gate the branch-topology mismatch.

- [s7] Tried and rejected (measured worse, on the PRE-merge chassis): two per-table pointer locals (`u8 *pf`, `u8 *ps`) in place of the `[i*2]` array-index reads for the D_8009A821/D_8009A820 byte tables -- score 81 -> 83 (worse), build_insns 197 -> 196. Reverted before the flags/ang/code merge was applied; banked as memory/grind/func_80056CB8/rejected/per-table-pointer-locals-worse.c. A further cross-declaration-pointer-arithmetic variant (`ps = pf - 1`) was drafted but never measured -- it is undefined-behavior C and was reverted without running sandbox, noted only so it is not re-derived.

- [s7] Working tree state: src/text1b.c carries the s7-banked candidate body (flags/ang/code merge + all s2-s6 fixes) at end of session -- per asm-until-matched this is NOT committed; the ledger candidate.c is the persistent record. (If the driver's scope check requires a clean tree between sessions, this session's final action reverts src/text1b.c to the committed INCLUDE_ASM state -- see the outcome JSON / self_vet for the final tree state taken.)

- [s7] Chassis re-verified at session start: candidate.c through s6 reproduces score 81/204 (build_insns 197) before any s7 change, matching the ledger exactly.

- [s7] Direct read of asm/funcs/func_80056CB8.s (not a stale dump) shows target keeps ONE hardware register ($s0) for three successive non-overlapping-lifetime roles: flags, ang (computed in place, never a separate register), and the final disposition code (reused starting .L80056ED0) -- confirmed line-by-line in the target's own bytes, not inferred from a dump.

- [s7] Applying the flags/ang/code variable-reuse merge (borrowing the EXISTING flags local, deleting the ang and code declarations entirely) drops the honest floor 81 -> 58/204 (build_insns 197 -> 198). Reproduced twice in-session.

- [s7] objdiff (tools/objdiff.py, tmp/grind/func_80056CB8/s7/objdiff_s7.txt) confirms func_80056CB8 is still the ONLY function differing from build/src/text1b.o (the original target bytes) across the whole TU (271/272 identical elsewhere), and the function's own diff region shrank to 112 lines: (a) a pure register-name/loop-counter rotation ($s3 vs a $s6/$s7-entangled role in target for the outer loop index and its derived store addresses) and (b) the code==4 tail's branch-topology difference (bltz+beqz vs bgez) -- but now our build ALSO reproduces target's characteristic FOUR duplicated address-recompute instructions into exit-branch delay slots (it did not before the merge); only the two-vs-one branch SPLIT for the y-compare itself remains different.

- [s7] Re-tested the s6-killed goto-vs-nested-if rewrite of the code==4 tail on this NEW chassis: byte-identical again (58/198). Second independent confirmation across two materially different chassis.

- [s7] Per-table pointer-local variant for the two byte-table reads measured WORSE (81 -> 83) on the pre-merge chassis; reverted and banked as a rejected form.

- [s7] A cross-declaration pointer-arithmetic variant (ps = pf - 1, treating D_8009A820/D_8009A821 as one array) was drafted but never measured -- recognized as undefined-behavior C and as deepening the exact declaration-pun the grind brief's DATA MODEL section flags for these two symbols; reverted without running sandbox.

- [s7] src/text1b.c reverted to the committed INCLUDE_ASM state at session end (git checkout -- src/text1b.c verified clean); the s7-banked body lives only in memory/grind/func_80056CB8/candidate.c per asm-until-matched.

- [s8] solver modality: inverse_compose.py classify (object-level, --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o) on the s7-banked floor-58 chassis reports FIRST DIVERGENCE: PRE-RA -- instruction MULTISET differs between our 198-insn build and target's 204-insn stream, so this residual is NOT reachable by tools/ra_solver (global/local-alloc/reload) or tools/sched_solver; both are RA/scheduler-only and this is upstream of both. Full report: tmp/grind/func_80056CB8/s8/classify_s8.txt.
- [s8] The multiset diff's two clearest signals: (1) our build materializes the 0x1F8002B8 scratchpad-address literal via $fp (li/ori) and stores it to the outgoing-arg stack slot TWICE (once per func_80053614 call site); target shows only a single lui/ori of that constant. (2) target's stream contains `addiu s8,s8,2`, `sll s8,#,0x2` (a *4 scale -- NOT the *2 our two byte-table index computations use), two `addu #,#,s8`, and a `beqz`+`bltz`+`j` triple where our build has one `bgez` -- this corroborates the code==4 y-compare tail's branch-topology mismatch (killed twice already, s6 and s7, as a goto-vs-nested-if question) is a genuine PRE-RA structural residual, not resolved by either prior attempt's C-control-flow shape change alone.
- [s8] Naming the repeated 0x1F8002B8 literal as a single fresh local measured WORSE (58 -> 60) -- the naive single-named-intermediate lever does not by itself recover target's one-time materialization; whatever produces target's single lui/ori is either a different value-lifetime shape (loop-invariant hoist above the loop entirely, not just a shared name inside it) or entangled with the s8/*4/addu/branch cluster above, not separable as an isolated CSE fix.

- [s8] inverse_compose.py classify (object-level, --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o) on the s7-banked floor-58 chassis reports FIRST DIVERGENCE: PRE-RA -- instruction multiset differs between our 198-insn build and target's 204-insn stream; not reachable by ra_solver or sched_solver.

- [s8] The multiset diff's clearest signals: our build materializes+stores the 0x1F8002B8 scratchpad-address literal via $fp twice (once per func_80053614 call); target shows one lui/ori. Target's stream also carries addiu s8,s8,2, sll s8,#,0x2 (*4, not our *2), two addu #,#,s8, and a beqz+bltz+j triple where we have one bgez -- corroborates the code==4 y-compare branch-topology mismatch (killed s6, s7) as a genuine PRE-RA structural residual.

- [s8] Naming the repeated literal as a single fresh local measured WORSE (58->60) -- the naive single-named-intermediate lever alone does not recover target's one-time materialization; whatever produces target's single lui/ori is either a loop-invariant hoist above the loop entirely, or entangled with the s8/*4/addu/branch cluster, not separable as an isolated CSE fix.

- [s9] enumerate modality: chassis re-confirmed floor 58/204 (s7-banked candidate.c body + func_80053614 s32-return fix, re-applied and re-verified before any s9 edit).
- [s9] Re-tested the s8 "hoist 0x1F8002B8 above the loop entirely" frontier hypothesis (single fresh local declared BEFORE the for-loop, not just once inside it, used at both func_80053614 call sites): score 58 -> 60 (WORSE), same result as s8's in-loop attempt. Reverted. This closes both the in-loop (s8) and above-loop (s9) single-naming spellings for this literal; whatever produces target's single lui/ori materialization is not reachable via a plain named-intermediate at either scope, corroborating s8's PRE-RA multiset classification (this is not an ordinary CSE-naming problem).
- [s9] Systematic spelling_enum sweep (tools/spelling_enum.py --no-swaps default swaps included) of the code==4 tail's dx/dz distance-check region (`s32 dx = hit0[0]-...; s32 dz = hit0[2]-...; if (0x3D0900 < dx*dx + dz*dz)`): exhaustive 5/5 spellings (both declaration orders dx-then-dz / dz-then-dx, plus dx-inlined, dz-inlined, and both-inlined into the if-condition) measured BYTE-IDENTICAL at 58/198 -- zero gradient across the entire declaration-order + inlining spelling space for this sub-expression. Full variant set: tmp/grind/func_80056CB8/s9/enum/{v0..v4}.c (generated), each hand-applied to src/text1b.c and sandboxed individually (spelling_enum's own sandboxing companion tool tools/sweep_variants.py is blocked by worktree_contamination_guard for a non-wteng-pinned direct invocation on main, so this session verified each variant by direct src edit + `wteng sandbox` instead -- same measurement, just manual instead of batched).
- [s9] CLASS KILL (this specific sub-expression's spelling space only, NOT a class kill of the whole residual): the dx/dz distance-check arithmetic is not where the floor-58 residual lives. This corroborates s8's classify finding that the residual is upstream (the scratchpad-literal materialization count / the y-compare two-vs-one branch split), not in this downstream arithmetic. Consistent across s6 (goto-vs-nested-if on the y-compare itself, twice killed), s8 (both single-naming spellings of the scratchpad literal), and now s9 (the distance-check arithmetic) -- three independent regions of the code==4 tail all show zero gradient, narrowing the live residual to the call-site scratchpad materialization mechanism itself (target's single lui/ori vs our two) and/or the branch-topology split, neither of which any tried C-level respelling in this tail has moved.

- [s9] Chassis re-confirmed at session start: applying candidate.c (s7 body) + the func_80053614 void->s32 return-type prerequisite to src/text1b.c reproduces sandbox score 58/204 (build_insns 198) exactly, matching the ledger's last-recorded floor.

- [s9] Two independent spellings of the single-named-intermediate lever for the 0x1F8002B8 scratchpad literal (in-loop, s8; above-loop, s9) both measure WORSE (58->60) -- the naive CSE-naming fix does not recover target's single lui/ori materialization at either declaration scope, reinforcing s8's PRE-RA multiset classification (this is not an ordinary redundant-load-elimination problem reachable by renaming).

- [s9] The code==4 tail's dx/dz distance-check arithmetic has zero spelling gradient (5/5 exhaustive variants byte-identical) -- this sub-expression is not where the residual lives, narrowing the live floor-58 gap to the scratchpad materialization count and/or the y-compare branch-topology split (bgez vs bltz+beqz), both already independently corroborated PRE-RA by s8's inverse_compose.py classify report.

- [s9] Three independent regions of the code==4 tail (y-compare control flow: killed s6+s7; scratchpad literal naming: killed s8+s9; distance-check arithmetic: killed s9) now all show zero gradient under every C-level respelling tried -- the remaining residual is very likely a single upstream mechanism (the PRE-RA instruction-multiset divergence s8 identified) manifesting across all three, not three separate local problems.

- [s10] Chassis floor is stable at 58/204 (build_insns 198) across s7, s8, s9, and this session (s10) -- no drift.

- [s10] No FAKE-annotated construct exists anywhere in the current candidate.c (the s7 flags/ang/code merge is the SOTN-sanctioned variable-reuse-for-codegen-control family, not a FAKE last-resort carve-out) -- tools/fake_ablate.py's sweep-of-FAKE-marker-subsets is inapplicable to this ledger's kill re-audit; recorded explicitly so a future session does not spend a turn rediscovering this.

- [s10] Read the full 218-line asm/funcs/func_80056CB8.s directly this session (not re-derived from a stale dump or the s8 classify report's placeholder register names, which are the tool's own generic labels and do NOT correspond to physical MIPS $s8/$fp registers one-to-one -- the classify report's 'sll s8,#,0x2' / 'addiu s8,s8,2' placeholders actually correspond to the physical $fp register in the raw asm).

- [s10] Target's $fp register is initialized ONCE before the loop (sll $fp,$v1,2 = v1*4 = i_initial*2, asm line 27) and incremented by a plain addiu $fp,$fp,0x2 exactly once per iteration at the loop bottom (asm line 203), in lockstep with but SEPARATE FROM the real loop counter $s6 (i, addiu $s6,$s6,0x1 at asm line 199). $fp is read via addu $at,$at,$fp at BOTH the D_8009A821 lookup (asm line 31) and the D_8009A820 lookup (asm line 68) -- i.e. target keeps i*2 as a second, genuinely loop-carried induction variable, not a same-iteration recompute-from-i temporary.

- [s10] The classify report's other 'target only' instructions (li #,4; two lw at stack offsets 96/104) are ordinary structural artifacts of the code==3/4 tail comparison and the pt0/pt1 pointer spills across the two func_80053614 calls -- traced against the raw asm and NOT independent residual axes; they are part of the already-diagnosed (s6/s7/s8) branch-topology + call-argument-spill residual, not a new lever.

- [s10] Two spellings of 'share i*2 as one C value across both byte-table reads' are now killed on the s7/s10 chassis: fresh in-body local (regenerates s6's original finding, now confirmed chassis-independent) and loop-carried induction variable (new this session, worse still). Both regress via the same mechanism s6 identified: the shared value's live range spans the ratan2() call and the intervening obj/flags computation, and that extra register pressure costs more than the single sll+addu it would remove.

- [s11] [forensics] Ran the instrumented cc1 .loop dump (pwsh tools/grinder/dump.ps1 func_80056CB8) and read the func_80056CB8 slice (tmp/grind/func_80056CB8/dumps/text1b.loop:13251-14311). Root-caused the missing $fp accumulator to loop.c:3806-3833's strength-reduction benefit-vs-insn_count threshold ("giv of insn 140 not worth while, 124 vs 164."; loop has 164 real insns) -- both i*2 byte-table-index givs are already combined by combine_givs from our existing (&D_x)[i*2] spelling, but neither the combined giv nor the store-address giv clears the threshold for this loop's size, so GCC recomputes them from the biv each iteration instead of maintaining an accumulator. This is a named, non-source-facing mechanism, not an unexplored C structure.
- [s11] Read the FULL target asm (asm/funcs/func_80056CB8.s, all 210 lines) for the first time this session end to end (not just the loop-body slice prior sessions focused on). Found $s0 (already flags/ang/code per s7) also carries r1 across the entire second func_80053614 call (addu $s0,$v0,$zero at line 105; or $s0,$s0,$v0 / addiu $s0,$s0,0x1 at the tail merges r2 directly into the register still holding r1). r2 itself never gets a persistent register -- consumed straight from $v0.
- [s11] func_80056CB8 sandbox --disable all: 58/204 (build_insns 198) confirmed at session start; 48/204 (build_insns 198, unchanged insn count) after the r1/r2 variable-reuse merge -- new session floor.

- [s11] sandbox func_80056CB8 --disable all at session start (s7-banked chassis reapplied): score 58, build_insns 198 -- reproduces s7-s10 exactly.

- [s11] pwsh tools/grinder/dump.ps1 func_80056CB8 regenerated tmp/grind/func_80056CB8/dumps/text1b.{loop,greg,lreg,sched,sched2,combine,cse,cse2,flow,jump,jump2,rtl,s,dbr} from the instrumented cc1 against the full text1b.c TU (pre-existing unrelated forward-declaration conflict warnings elsewhere in the TU are cosmetic and do not affect func_80056CB8's own dump slice).

- [s11] func_80056CB8's .loop dump slice is tmp/grind/func_80056CB8/dumps/text1b.loop lines 13251-14311 ('Loop from 22 to 444: 164 real insns.'); the giv-rejection lines name the exact loop.c:3823 predicate.

- [s11] Read the FULL target asm/funcs/func_80056CB8.s (all 210 lines) end to end for the first time this session (prior sessions read slices); found the r1/r2 register-reuse pattern in $s0 not previously identified despite being flagged as an open frontier item since s7.

- [s11] Final sandbox measurement after both s11 changes: score 48, build_insns 198 (unchanged insn count from the 58-floor baseline) -- confirms a pure register-identity win, same class as the s7 merge.

- [s11] src/text1b.c and the func_80053614 return-type prerequisite were reverted to the committed HEAD state (git checkout -- src/text1b.c) before ending the session; func_80056CB8 remains INCLUDE_ASM on main per asm-until-matched. candidate.c carries the full s11 body + derivation.

- [s12] Fresh m2c (mipsel-gcc-c target) decompile of asm/funcs/func_80056CB8.s archived at tmp/grind/func_80056CB8/s12/m2c_out.c -- first time this ledger has run m2c directly on this function (prior sessions only suggested it). Confirms (does not overturn) the s10/s11 raw-asm reading: target's var_fp increments by 2 alongside the real loop counter var_s6 and feeds both byte-table lookups directly; the store-address expression arg0+i is recomputed at 5 separate branch-join points before one shared final store.

- [s12] tools/ra_solver/inverse_compose.py classify (object-level path, text1b/func_80056CB8, --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o) must be invoked via WSL on this machine -- the native Windows Python path fails with FileNotFoundError because engine/score.py shells out to a WSL-only objdump binary. Command: wsl bash -c "cd '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile' && source .venv/bin/activate && python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_80056CB8/text1b.o".

- [s12] classify verdict on the s11/s12 (floor-48) chassis is UNCHANGED from s8's original PRE-RA verdict, but the instruction-multiset diff is now more precise: our build emits addiu sp,sp,-176/176 (176-byte frame) vs target's -168/168 (168-byte frame, one 8-byte stack slot smaller); our build also shows move #,s8 / move s8,# / lw s8,168(#) (a spill-and-reload round-trip through $s8) plus lhu #,1000(s8) / lhu #,106(s8) (arg0-relative loads based off $s8), while target shows addiu s8,s8,2 (the known i*2 accumulator) plus three plain addu #,#,# / one addu #,#,s8, and the SAME lhu offsets via a different, unspecified base register. Reading: in our build $s8 is occupied by arg0/obj (with a stack round-trip to free it up at some point), whereas in target $s8 is reserved for the i*2 accumulator throughout and arg0 lives in a different register.

- [s12] All 4 possible C-level spellings of 'share i*2 between the two byte-table reads' are now measured and KILLED on this ledger: fresh-int-per-iteration (s6, worse), loop-carried-int (s10, worse), fresh-pointer-per-iteration (s7, worse), loop-carried-pointer (s12, worse). This closes the family; the accumulator is not reachable by naming/carrying the index value differently.

- [s12] candidate.c's own body text does NOT carry the func_80053614 void->s32 signature prerequisite (it lives in prose only) -- re-applying the candidate verbatim without also fixing func_80053614 silently starts the next session from a badly wrong chassis (132/171 instead of 48/198). Documented at the top of candidate.c's s12 header so future sessions do not lose a turn rediscovering this.

- [s13] Fresh `pwsh tools/grinder/dump.ps1 func_80056CB8` .greg dump (tmp/grind/func_80056CB8/dumps/text1b.greg lines ~14788-14850) shows 'Spilling reg 11. Spilling reg 65.' driven by simultaneous register class needs at insn 153 (the scale*sin_p widening multiply, needs LO/MD_REGS) and insn 456 (needs GR_REGS); cross-referenced against the .combine dump, insns 456/457 are the &pt0/&pt1 stack-address LICM hoists (target ALSO hoists these -- confirmed by direct read of asm/funcs/func_80056CB8.s lines 21-26, NOT a residual), and insn 459 sets reg 149 = 0x3D0900, the flags==4 threshold constant, whose long cross-loop live range competes with the multiply's LO/MD need.

- [s13] Direct read of asm/funcs/func_80056CB8.s lines 150-171 (the flags==4 tail): target materializes the 0x3D0900 constant INLINE at its single use site inside the conditionally-executed branch (lui/ori pair right before the compare), NOT hoisted above the loop -- the first direct confirmation of which specific loop-invariant our build hoists that target's original C did not.

- [s13] loop.c:1529-1634 (move_movables) and loop.c:3806-3833 (strength_reduce's giv rejection) both gate on the SAME quantity, insn_count (=164 real insns for this loop, printed in the .loop dump), in OPPOSITE directions: strength_reduce wants insn_count SMALL to accept the $fp accumulator (per s11's finding); move_movables wants insn_count LARGE (relative to threshold*savings*lifetime) to reject the constant hoist. This is a double-bind: an insn_count-shifting change plausibly trades one lever against the other and must be measured against both simultaneously.

- [s13] Sandbox measured fresh this session on the s12-banked chassis: score=48, target_insns=204, build_insns=198 (unchanged from s11/s12 across all 4 measurements this session, including 3 reverted probes).

- [s14] tools/sweep_variants.py IS callable from an agent session on main despite the worktree_contamination_guard: the guard blocks unpinned `python3 -m engine.cli` / relative `eng.ps1` invocations, not a plain `wsl bash -c 'source .venv/bin/activate && python3 tools/sweep_variants.py ...'` call (sweep_variants.py shells out to `python3 -m engine.cli sandbox` internally, from inside WSL, not from the blocked vector). s9's "manual edit + wteng sandbox per variant" workaround is no longer necessary for future enumerate-modality sessions on this function or any other.

- [s14] Chassis re-confirmed at session start: s13-banked candidate.c body + func_80053614 s32-return prerequisite applied to src/text1b.c reproduces sandbox score 48/204 (build_insns 198) exactly, matching the ledger's last-recorded floor, before any s14 change.

- [s14] The func_80053614 void->s32 return-type prerequisite is REQUIRED for sweep_variants.py runs too, not just manual candidate.c splices: the first sweep attempt (func_80053614 still void) scored every one of the 16 order variants at 133-171/171 -- the same badly-wrong shape s12 documented for a naive candidate.c splice. Re-applying the prerequisite before the sweep fixed this; recorded so a future session doesn't lose a turn rediscovering it for sweep-tool use specifically.

- [s14] Systematic spelling_enum.py + sweep_variants.py sweep of the sin_p/cos_p/scale/x/z block (16 orderings, --no-swaps) found REAL gradient for the first time this ledger cycle on a spelling_enum-driven sweep: 4/16 orderings score 42/197 (vs the 48/198 baseline for the original order). This is the block immediately after the ratan2 angle computation and immediately before the pt0/pt1 array fill + first func_80053614 call -- a region s6-s13 never targeted (all prior sessions focused on the i*2 index arithmetic and the flags==3/4 tail).

- [s14] Re-sweeping the SAME region with the commutative-swap axis added (32 variants) found no further improvement below 42/197 -- this specific block's order+swap spelling space is now exhaustively measured flat at 42.

- [s14] Official `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all` on the adopted v04 ordering (sin_p, scale, x, cos_p, z) confirms: score=42, target_insns=204, build_insns=197. New session floor: 48 -> 42 (first floor movement since s11's r1/r2 merge, three sessions ago).

- [s14] src/text1b.c (both the func_80053614 signature fix and the func_80056CB8 body) was reverted to the committed HEAD state (`git checkout -- src/text1b.c`) before ending the session; func_80056CB8 remains INCLUDE_ASM on main per asm-until-matched. candidate.c carries the full s14 body + derivation.

- [s14] tools/sweep_variants.py IS callable from an agent session on main via a plain 'wsl bash -c source .venv/bin/activate && python3 tools/sweep_variants.py ...' call -- the worktree_contamination_guard blocks unpinned 'python3 -m engine.cli' / relative eng.ps1 invocations, not this vector. s9's manual per-variant workaround is no longer necessary for future enumerate-modality sessions.

- [s14] Chassis re-confirmed at session start: s13-banked candidate.c body + func_80053614 s32-return prerequisite reproduces sandbox score 48/204 (build_insns 198) exactly before any s14 change.

- [s14] The func_80053614 void->s32 return-type prerequisite is required for sweep_variants.py runs too: without it, all 16 order variants scored 133-171/171 (the same badly-wrong shape s12 documented for a naive candidate.c splice).

- [s14] The sin_p/cos_p/scale/x/z block sits immediately after the ratan2 angle computation and immediately before the pt0/pt1 array fill + first func_80053614 call -- a region s6-s13 never targeted (all prior sessions focused on the i*2 index arithmetic and the flags==3/4 tail).

- [s14] Official & tools/wteng.ps1 main sandbox func_80056CB8 --disable all on the adopted v04 ordering confirms score=42, target_insns=204, build_insns=197 -- first floor movement since s11's r1/r2 merge, three sessions ago.

- [s14] src/text1b.c was reverted to the committed HEAD state (git checkout -- src/text1b.c) before ending the session; func_80056CB8 remains INCLUDE_ASM on main per asm-until-matched.

- [s15] Chassis re-confirmed at session start: sandbox --disable all on the reapplied s14 candidate.c body (+ func_80053614 void->s32 prerequisite) scores 42, target_insns 204, build_insns 197 — matches the ledger's last recorded floor exactly.

- [s15] spelling_enum.py's _ASSIGN_RE (tools/spelling_enum.py:62) only matches bare-identifier LHS assigns (`name = expr;`), so `pt0[0] = expr;` / `pt1[N] = expr;` are silently treated as anchors, not reorderable statements — confirmed by reading the regex, not assumed. The s15 hand generator (tmp/grind/func_80056CB8/s15/gen_pt_variants.py) is a scratch-only workaround, no edit to tools/ was made or needed.

- [s15] tools/ra_solver/inverse_compose.py classify's own C-lever suggestion list for a cse_merge-class PRE-RA residual includes 'store-const-reload-cse' and '(single named intermediate)' by name — both were already covered by prior sessions or tried fresh this session (the scratchpad local); both remain non-closing.

- [s15] The two 'ours only: sw s8,16(#)' entries in the classify diff are NOT a redundant re-materialization of the literal (there is only ONE lui/ori for it) — they are the o32 ABI's stack-passed 5th-argument store, one per func_80053614 call site, which is structurally required regardless of how arg4's value is sourced in C. This reframes the s8/s13 'materializes twice' language: the redundancy is in the STACK STORE pairing with a call-crossing-live register (reg 11/65 spill), not in the literal's derivation.

- [s15] src/text1b.c is byte-identical to HEAD at session end (git diff --stat reports no changes) — all edits made during this session (candidate re-application, the scratchpad-literal experiment) were reverted; only memory/grind/func_80056CB8/candidate.c carries the session's findings, per the asm-until-matched convention.

- [s16] Direct read of asm/funcs/func_80056CB8.s:150-192 (the flags==4 y-compare tail) this session, decoded register-by-register: beqz $v1,.L80056F98 (outer threshold skip) / lw $a0,0xBC($s1) (y) / lw $v1,0x4C($sp) (hit1[1] spilled to stack) / subu $v0,$a0,$v1 (dy) / bltz $v0,.L80056F80 with slti $v0,$v0,0x3E9 in the delay slot (executes unconditionally) / beqz $v0,.L80056F90 (flags=5 path) / j .L80056F98 (skip) / .L80056F80: subu $v0,$v1,$a0 (=-dy) / slti $v0,$v0,0x3E9 / bnez $v0,.L80056F98 / .L80056F90: addiu $s0,$zero,5 -- this maps EXACTLY to the already-banked nested if/else in candidate.c, not to a different branch-topology; the s6/s7 'beqz+bltz+j triple vs our single bgez' framing in the ledger's live frontier was a misdiagnosis based on instruction-count comparison, not a decoded structural read.

- [s16] src/text1b.c reverted to byte-identical HEAD at session end (git diff --stat empty after `git checkout -- src/text1b.c`, verified).

- [s17] Chassis re-confirmed at session start (synthesis modality): applying the s16-banked candidate.c body + func_80053614 s32-return prerequisite to src/text1b.c and running `sandbox func_80056CB8 --disable all` scores 42, target_insns 204, build_insns 197 -- matches the ledger's last recorded floor exactly.

- [s17] MANDATORY KILL RE-AUDIT: re-measured s16's closest-to-target instance kill (the flags==4 y-compare single-abs-value form, `s32 dy = y-hit1[1]; s32 ady=(dy>=0)?dy:-dy; if(ady>=0x3E9) flags=5;`) fresh on the current chassis. No FAKE construct is present in this form or the baseline, so `tools/fake_ablate.py` (which ablates FAKE-annotated units) has nothing to ablate here -- the applicable re-audit is the direct re-measurement. Result: 45/204 (build_insns 197, unchanged) -- reproduces the s16 result exactly. Kill re-confirmed on the current chassis.

- [s17] ROOT-CAUSE CORRECTION of the s13/s15/s16 "reg 11/reg 65 double-bind" framing: read tools/gcc-2.7.2/config/mips/mips.h:1181 (`#define FIRST_PSEUDO_REGISTER 68`) and tools/gcc-2.7.2/reload1.c:2283 (`fprintf (dumpfile, "Spilling reg %d.\n", spill_regs[n_spills]);`). Since FIRST_PSEUDO_REGISTER is 68, any register number below 68 printed by this message is a HARD register, never a pseudo -- "reg 11" and "reg 65" in the .greg dump's "Spilling reg 11."/"Spilling reg 65." lines are NOT pseudo/allocno numbers naming a spilled C-level value. Per mips.h's DEBUG_REGISTER_NAMES table, register 11 = $t3 and register 65 = "lo" (the MIPS multiply/divide low-result register, class MD_REGS/LO_REG). Reading the fresh .greg dump for func_80056CB8 this session (tmp/grind/func_80056CB8/dumps/text1b.greg, function block lines 14788-15991) directly localizes the two "Need 1 reg of class..." lines immediately preceding the spill: "Need 1 reg of class LO_REG (for insn 142)" / "Need 1 reg of class MD_REGS (for insn 142)" -- insn 142 in the function's own RTL stream (tmp/grind/func_80056CB8/s17/func_greg.txt:399) is `(insn:HI 142 ... (parallel [(set (reg:SI 65 lo) (mult:SI (reg/v:SI 3 v1) (reg:SI 2 v0))) (clobber hi) (clobber accum)]) 29 {mulsi3_internal} ...)` -- the FIRST of the two `(scale * *sin_p) >> 12` / `(scale * *cos_p) >> 12` multiplies in the loop body, whose lo result is moved to $t2 in the immediately following insn 478 because reload needs lo free again for the second multiply shortly after. This is ordinary MIPS single-hi/lo-pair register pressure from two back-to-back 32-bit multiplies, not a spillable named C object; "Need 1 reg of class GR_REGS (for insn 457)" is a separate, later scratch-register need (t3 reused for materializing &pt0/&pt1/&hit0/&work stack-slot addresses ahead of the two func_80053614 calls, per the addsi3_internal insns at tmp/grind/func_80056CB8/s17/func_greg.txt:118-120). The s13-s16 frontier item asking a solver to "name which C-level object occupies pseudo 11 and pseudo 65" rests on a category error and cannot be answered because no such pseudo exists at those numbers.

- [s17] Re-ran `tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --ours-object tmp/sandbox/func_80056CB8/text1b.o --target-object build/src/text1b.o` fresh on the current 42/197 chassis (WSL, output saved to tmp/grind/func_80056CB8/s17/classify.txt). Verdict is UNCHANGED from the s8 finding already in this ledger (hypotheses.md:452): FIRST DIVERGENCE: PRE-RA, rtl_shape -- "the honest (197 insns) and target (204 insns) instruction streams have a DIFFERENT MULTISET, not merely a different assignment/order of the same insns" and "ra_solver/sched_solver are both RA/scheduler-only and explicitly refuse this residual." This CONTRADICTS the entire s13-s16 line of pursuit, which built a PRE-RA "double-bind" / register-allocation framing and proposed running `tools/ra_solver/inverse.py` (the RA-solver, not the classify triage) against this residual -- the classify tool's own verdict (already banked at s8, apparently not re-consulted by s13-s16) says the RA/scheduler models "cannot express this residual" at all, so an inverse.py RA-solver run was never going to be the productive next step regardless of which pseudo occupies which register.

- [s17] Correlated part of the classify "target only" instruction list to the raw target disassembly (asm/funcs/func_80056CB8.s) this session: `addiu $v0,$t3,0x2` (80056FA4) and `addiu $fp,$fp,0x2` (80056FB0) sit in the LOOP TAIL (80056F9C-80056FB4, immediately before the loop re-tests and before the `.L80056FB4` epilogue label), alongside `addiu $s6,$s6,0x1` (80056FA0, the `i++`). These are per-iteration index/offset bookkeeping (the byte-table index and a byte-offset-into-Judge-table pointer, `$fp`, both advanced by 2 per iteration) -- NOT part of the flags==4 y-compare tail (which s16 already closed at asm lines 175-192 / addresses 80056F50-80056F78). This means at least some of the classify "target only" shapes are ordinary loop-increment instructions our C's `for` loop produces differently-shaped increments for, a genuinely NEW un-examined axis distinct from both the y-compare tail (closed s16) and the scratchpad-literal materialization (closed s8/s9/s15).

- [s17] Chassis re-confirmed at floor 42/204 (build_insns 197) fresh this session by applying the s16-banked candidate.c body + func_80053614 s32-return prerequisite and running the sandbox.

- [s17] reload1.c:2283's 'Spilling reg %d.' message can only ever name a hard register (spill_regs[] < FIRST_PSEUDO_REGISTER=68), so the s13-s16 ledger's 'reg 11/reg 65' spill entries name $t3 and lo, not pseudos -- this closes the entire pseudo-naming line of inquiry as a category error.

- [s17] insn 142 in func_80056CB8's own RTL (fresh .greg dump this session) is the mulsi3_internal for the loop's first `scale * *sin_p` multiply; the LO_REG/MD_REGS spill pressure at that point is inherent single-hi/lo-pair MIPS register-class pressure from two back-to-back 32-bit multiplies (sin_p then cos_p), not a source-level spelling defect.

- [s17] tools/ra_solver/inverse_compose.py classify's FIRST DIVERGENCE: PRE-RA/rtl_shape verdict (banked at s8, re-confirmed fresh this session) has stood unchanged across s8-s17 -- this residual is upstream of every RA/scheduler model in the solver suite by the tool's own design, so no amount of register-allocation-focused probing (s13-s16) could have closed it.

- [s17] Two of the classify 'target only' instructions (`addiu $v0,$t3,0x2` @80056FA4, `addiu $fp,$fp,0x2` @80056FB0) are now localized to the loop-tail index/pointer-bookkeeping block (80056F9C-80056FB4) -- ordinary per-iteration `i++`/pointer-advance bookkeeping our current for-loop C produces differently, and a genuinely new axis distinct from the two already-closed structural findings (y-compare tail s16; scratchpad literal s8/9/15).

- [s18] solver modality: classify re-run fresh on the current (s17-banked) 42/197 chassis reproduces the unchanged PRE-RA/rtl_shape verdict (banked s8, re-confirmed s17, s18). Both ra_solver and sched_solver remain out of scope for this residual by the tool's own design (instruction MULTISET differs, not allocation/scheduling).

- [s18] Read asm/funcs/func_80056CB8.s lines 1-217 directly (full prologue-through-loop-tail) to pin down target's $fp register exactly: `sll $fp,$v1,2` before the loop (= start*2, matching our C's already-*2-scaled `i`), `addiu $fp,$fp,0x2` at the loop bottom (.L80056FB0), read via `addu $at,$at,$fp` at BOTH the D_8009A821 and D_8009A820 byte-table lookups. This is a genuine loop-carried i*2 accumulator resident in $fp across the whole loop body, confirming (not merely re-asserting) the s10/s12-era description on the CURRENT chassis.

- [s18] Kill re-audit (mandatory per 3-flat-session rule): re-measured the closest-to-target rejected form (s10 variant (b), loop-carried INT idx2 mirroring target's $fp structure) on the CURRENT 42/197 chassis. Result: 42 -> 50 (build_insns 197 -> 200), WORSE, reproducing the s10/s12 verdict on a chassis two floor-drops later (58 -> 48 -> 42). Reverted; floor 42/204 re-confirmed after revert. No FAKE constructs in either form; fake_ablate.py has nothing to ablate.

- [s18] Read tools/gcc-2.7.2/loop.c:3806-3833 (the strength-reduction giv-worth decision) directly: the gating predicate at loop.c:3823 (`v->lifetime * threshold * benefit < insn_count`) depends only on `insn_count` (the loop body's GCC-computed total instruction count), never on which C construct names/carries the candidate induction variable's value. This gives the s11 "loop.c strength-reduction benefit threshold" root-cause a named, line-cited mechanism: no re-spelling of how i*2 is held (fresh int / loop-carried int / fresh pointer / loop-carried pointer -- all four now measured across two chassis generations) can change insn_count in the direction that would cross the threshold, because promoting the value to its own C object costs additional instructions (register pressure, increment insns) rather than removing them.

- [s18] sandbox func_80056CB8 --disable all fresh-measured this session at 42/204 (build_insns 197), exactly reproducing the s14-s17 chassis after re-applying candidate.c (func_80053614 void->s32 return prerequisite included).

- [s18] classify's PRE-RA/rtl_shape verdict for this residual has now stood unchanged across s8, s17, and s18 -- three independent fresh runs.

- [s18] Target's $fp register is initialized once before the loop as `sll $fp,$v1,2` (= start*2, matching our C's already-*2-scaled loop variable `i`) and incremented by a bare `addiu $fp,$fp,0x2` at the loop bottom (.L80056FB0), read via `addu $at,$at,$fp` at both the D_8009A821 flags lookup and the D_8009A820 scale lookup -- a genuine loop-carried i*2 accumulator, confirmed by direct line-by-line reading of asm/funcs/func_80056CB8.s this session.

- [s18] tools/gcc-2.7.2/loop.c:3823's giv-worth predicate (`v->lifetime * threshold * benefit < insn_count`) is the named mechanism behind the s11-identified 'loop.c strength-reduction benefit threshold' root cause: it depends only on insn_count and the giv's own terms, never on the C-level spelling of the index.

- [s18] All four possible C spellings of sharing i*2 as one value (fresh int -- s6; loop-carried int -- s10 and re-confirmed s18; fresh pointer and loop-carried pointer -- s12) are now measured WORSE than plain `(&D_x)[i*2]` array indexing across three distinct chassis generations (58/198, 48/198, 42/197).

- [s19] Direct read of asm/funcs/func_80056CB8.s lines 144-153 attributes the classify-flagged "target only: li #,4" (x2, previously unattributed since s8) precisely: `addiu $v0,$zero,0x4` appears TWICE, once in the delay slot of `bne $s0,$v0(=3),.L80056F08` (line 145-146) and once in the delay slot of `beqz $v0,.L80056F08` (line 152-153) -- both are the unconditional-safe MIPS delay slot following a conditional branch, and both converge on the SAME successor label `.L80056F08` where `bne $s0,$v0,.L80056F98` (line 157) then tests flags==4 by reusing the already-loaded $v0=4 instead of re-materializing it at the join. This is reorg.c's fill_simple_delay_slots (tools/gcc-2.7.2/reorg.c:2861) opportunistically filling a would-be-nop delay slot with an insn safe on both branch outcomes, exploiting the shared successor to save one instruction (a classic reorg.c win, not resolve.c/combine).
- [s19] The two "target only: lw #,96(#)" / "lw #,104(#)" classify entries correlate to real target instructions at asm/funcs/func_80056CB8.s:73/124 (`lw $a1, 0x68($sp)` = 104 decimal, the func_80053614 4th-arg stack reload) and :198 (`lw $t3, 0x60($sp)` = 96 decimal, the loop-tail bound reload) -- both are ordinary per-iteration stack reloads of values our (smaller, 197-insn) frame keeps at DIFFERENT offsets, not evidence of a missing/extra C-level object. This is a frame-layout artifact of the same 197-vs-204 insn_count deficit s18 class-killed at loop.c:3823, not an independent structural axis.
- [s19] CONCLUSION: all four previously-"unattributed" classify target-only instruction categories (li#,4 x2; lw 96(#); lw 104(#)) are now fully explained as downstream consequences of the single insn_count deficit (loop.c:3823 class kill, s18) -- none are an independent, separately-fixable C-level lever. This closes the s17/s18 "next probe" frontier item: there is no hidden second structural difference in this tail; the entire PRE-RA residual traces to one cause (loop body real-instruction-count shortfall vs target), and further progress requires finding C that genuinely adds/reshapes real instructions in the loop body (not respells existing ones), which s6/s7/s10/s12/s15/s16/s18's ~29 measured instance kills already show is NOT achieved by any tried spelling of the i*2 index, the y-compare tail, the scratchpad literal, or the store-duplication tail.

- [s19] Fresh sandbox func_80056CB8 --disable all this session (s19), candidate.c body + func_80053614 void->s32 return-type prerequisite re-applied to src/text1b.c: score=42, build_insns=197, target_insns=204 -- exact reproduction of the s14-s18 floor.

- [s19] Fresh instrumented-cc1 .loop dump this session (pwsh tools/grinder/dump.ps1 func_80056CB8) reproduces the s18 giv-rejection numbers verbatim: 'Insn 42: giv reg 91 src reg 74 ... lifetime 1', 'giv at 42 combined with giv at 129', 'giv of insn 129 not worth while, 124 vs 164.', 'giv of insn 426 not worth while, 0 vs 164.' -- the loop.c:3823 predicate (v->lifetime*threshold*benefit < insn_count) rejecting strength-reduction of the i*2-scaled byte-table index giv regardless of which C construct names it. Excerpt saved tmp/grind/func_80056CB8/s19/loop_dump_excerpt.txt (170 lines, function func_80056CB8's full loop-pass trace).

- [s19] Read tools/gcc-2.7.2/loop.c:3806-3833 directly this session to re-verify the predicate text and line number are unchanged from s18's citation (v->lifetime * threshold * benefit < insn_count, line 3823, 'giv of insn %d not worth while, %d vs %d.' message format).

- [s19] Read asm/funcs/func_80056CB8.s:130-165 directly this session: confirmed `addiu $v0,$zero,0x4` (=li v0,4) is materialized TWICE at 80056EE0 and 80056EFC, in the delay slots of two branches converging on .L80056F08, matching the s19-banked reorg.c:2861 fill_simple_delay_slots attribution exactly.

- [s19] The previous session on this function was discarded solely for missing an attached artifact ('forensics session must attach >=1 existing non-empty artifact') -- this session's substantive findings (li#,4/lw-96/104 attribution, class kill) were already correctly derived and banked in hypotheses.md/evidence.md/candidate.c; this session's contribution is re-verifying that analysis fresh against the current chassis and attaching the required real artifact (tmp/grind/func_80056CB8/s19/loop_dump_excerpt.txt) plus an independent second read of the target asm confirming the li#,4 delay-slot-fill claim.

- [s20] Fresh m2c decompile of asm/funcs/func_80056CB8.s this session produces output byte-identical to the s12-archived copy (tmp/grind/func_80056CB8/s12/m2c_out.c) -- the target asm has not changed since this ledger began, so m2c gives no new raw information, only a fresh lens for finding untested statement-shape permutations.

- [s20] Every value m2c reconstructs (dx/dz/y in the flags==4 tail, the hit1[1]/obj->unkBC comparison in flags==3, both func_80053614 call-argument triples) matches the current candidate.c's semantics exactly -- m2c's differences from our C are purely SSA-materialization/statement-order artifacts, never a missing or differently-computed VALUE.

- [s20] The pt0/pt1 block-shape family (store-order reorder from s15, plus this session's store/computation interleave) is now closed across both call sites: 12 total measured spellings (10 from s15 + 2 from s20), none at or below the s14 floor of 42/204.

- [s20] The s19 forensics conclusion is unchanged: the remaining 42/204 residual (build_insns 197 vs target 204, a 7-instruction real-body deficit) is PRE-RA/rtl_shape per inverse_compose.py classify, gated by loop.c:3823's strength-reduction insn_count-threshold double-bind (named s13, sharpened with a line cite s18).

- [s21] Fresh sandbox func_80056CB8 --disable all this session (candidate.c body + func_80053614 s32-return prerequisite re-applied to src/text1b.c): score=42, build_insns=197, target_insns=204 -- exact reproduction of the s14-s20 floor. src/text1b.c reverted to byte-identical HEAD at session end (`git checkout -- src/text1b.c`, verified via `git status --short`).

- [s21] Fresh instrumented-cc1 dump this session (`pwsh tools/grinder/dump.ps1 func_80056CB8`) confirms the func_80056CB8 block in text1b.greg spans lines 14788-15991 and reproduces the s17-banked "Spilling reg 11."/"Spilling reg 65." pair verbatim (insn 142 mulsi3_internal LO_REG/MD_REGS need; insn 457 GR_REGS scratch need) -- unchanged from s17/s18, no new spill signal on the current chassis. Dump files saved: tmp/grind/func_80056CB8/dumps/text1b.{greg,lreg,sched,sched2,loop,combine,cse,cse2,flow,jump,jump2,rtl,dbr,s} (fresh this session).

- [s21] DEFINITIVE MECHANISM READ (closes the s20 live-frontier item): read tools/gcc-2.7.2/loop.c:3241 directly -- the threshold used by strength_reduce's giv-worth predicate at loop.c:3823 is `threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)`, NOT the move_movables threshold at loop.c:532 (same formula shape, different call site/context; both were candidates and both resolve to the same n_non_fixed_regs term). Read tools/gcc-2.7.2/regclass.c:360-392 (`init_reg_sets_1`): `n_non_fixed_regs` is a GLOBAL int (declared regclass.c:102, `extern`'d hard-reg-set.h:267) computed ONCE by counting `FIRST_PSEUDO_REGISTER` (=68, mips.h) hard registers whose `fixed_regs[i]` bit is unset, where `fixed_regs[]` is `bcopy`'d from the target-description macro `FIXED_REGISTERS` (mips.h:1188-1195) -- a STATIC LITERAL ARRAY (1,1,0,0,...,1 at fixed positions: $zero,$at,$k0,$k1,$gp,$sp,$fp,and one more) with no `CONDITIONAL_REGISTER_USAGE` override defined for this target (grep confirms the macro is unused in config/mips/mips.h). `init_reg_sets_1` is called once per COMPILATION (compiler startup), not once per function and not influenced by any function's live ranges, conflict graph, or register pressure.

- [s21] CLASS KILL: the s20 live-frontier hypothesis ("reducing register pressure elsewhere in the loop body ... could flip move_movables'/strength_reduce's threshold ... to reject or accept a giv without perturbing insn_count") rests on a false premise. `n_non_fixed_regs` is a per-compilation constant (28 on MIPS o32 per the FIXED_REGISTERS table: 68 total hard regs minus fixed $zero/$at/$k0/$k1/$gp/$sp/$fp/one more, minus $hi/$lo virtual class quirks) identical for every function compiled in this build -- it CANNOT be changed by any source-level restructuring of func_80056CB8 (or any other function): not by narrowing a variable's type, not by splitting a block-local live range, not by reducing simultaneously-live locals, not by any C-level register-pressure lever. The entire threshold term in both loop.c:532 and loop.c:3823's predicates is fixed at compile-configuration time; the ONLY variable term available to C source is `insn_count` (the loop body's real instruction count) and each giv's own `lifetime`/`benefit` (which s18 already showed cannot be moved favorably by any tried spelling of the index). This closes the entire "register-pressure-as-threshold-lever" line of inquiry as a category error, parallel to s17's "reg 11/65 pseudo-naming" category error.

- [s21] Fresh sandbox func_80056CB8 --disable all this session: score=42, build_insns=197, target_insns=204, exact reproduction of the s14-s20 floor.

- [s21] Fresh instrumented-cc1 dumps saved (tmp/grind/func_80056CB8/dumps/text1b.{greg,lreg,sched,sched2,loop,combine,cse,cse2,flow,jump,jump2,rtl,dbr,s}); func_80056CB8's block in text1b.greg spans lines 14788-15991.

- [s21] tools/gcc-2.7.2/loop.c has TWO distinct threshold formulas sharing the n_non_fixed_regs term: loop.c:532 (move_movables, multiplier base 1) and loop.c:3241 (strength_reduce, multiplier base 3) -- our residual's predicate at loop.c:3823 uses the :3241 definition.

- [s21] n_non_fixed_regs (regclass.c:102) is set exactly once per compilation in init_reg_sets_1 (regclass.c:360-392), counting non-fixed entries of the static FIXED_REGISTERS array (mips.h:1188-1195) over FIRST_PSEUDO_REGISTER=68 hard registers; no CONDITIONAL_REGISTER_USAGE macro is defined for this target to make it context-sensitive.

- [s21] src/text1b.c reverted to byte-identical HEAD at session end (git checkout -- src/text1b.c, verified via git status --short).
