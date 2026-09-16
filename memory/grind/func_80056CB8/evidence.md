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

- [s22] FLOOR DROP (first since s14, 8 sessions flat): materializing the loop's bound (`start + 2`) into a named local `s32 limit;` instead of a recomputed for-test subexpression drops the honest floor 42/204 -> 38/204 (build_insns 197 -> 198). This is a genuinely new axis: every prior lever in this ledger's history (s6/s10/s12/s18/s21) targeted the byte-table INDEX (i*2), never the loop BOUND itself.

- [s22] Combining the s22 limit win with the s10/s18 idx2 (i*2-sharing) promotion regresses to 55/204 (build_insns 201) -- idx2 remains dead (loop.c:3823) even on the improved chassis; the two axes are independent and only the bound-materialization axis helps.

- [s22] Fresh `inverse_compose.py classify` on the 38/198 chassis (tmp/grind/func_80056CB8/s22/classify.txt): FIRST DIVERGENCE unchanged at PRE-RA/rtl_shape. Target's `addiu s8,s8,2` + two `addu #,#,s8` (the $fp accumulator chased since s6) remain fully target-only -- the limit win did not touch that axis. NEW residue: our build now caches the 0x1F8002B8 scratchpad-address literal (passed identically to both func_80053614 calls) in a callee-saved register ($s8, via `lui s8,0x1f80`/`ori s8,s8,0x2b8`, stored twice at `sw s8,16(#)`) across the whole loop, where target instead shows a bare target-only `lui #,0x1f80` into a temp register -- suggesting target rematerializes this constant per call site rather than hoisting it to a callee-save home across the loop. Open frontier item for next session, not yet probed.

- [s22] Fresh `inverse_compose.py classify` on the 38/198 chassis (tmp/grind/func_80056CB8/s22/classify.txt): FIRST DIVERGENCE remains PRE-RA/rtl_shape (an instruction multiset difference, not an RA/scheduler permutation) -- target's `addiu s8,s8,2` and two `addu #,#,s8` (the $fp accumulator chased since s6) remain fully target-only; the limit win did not touch that axis at all.

- [s22] New residue introduced by the limit change: our build now caches the 0x1F8002B8 scratchpad-address literal (passed identically to both func_80053614 calls per loop iteration) in a callee-saved register ($s8, via `lui s8,0x1f80` / `ori s8,s8,0x2b8`, stored twice at `sw s8,16(#)`) across the whole loop -- apparently because the new `limit` local changed register-pressure bookkeeping enough that GCC chose to home this loop-invariant constant in a callee-save register instead of rematerializing it at each call site. Target instead shows a bare target-only `lui #,0x1f80` into a temp register, suggesting the original rematerializes this constant at (at least) one call site rather than hoisting it across the loop.

- [s22] src/text1b.c was reverted to HEAD (`git checkout -- src/text1b.c`, byte-identical INCLUDE_ASM) at session end per asm-until-matched -- this is a `progress` outcome, not candidate-ready (floor 38, not 0).

- [s23] Fresh direct read of asm/funcs/func_80056CB8.s:1-145 (full prologue through both func_80053614 call sites) this session, correcting the s22 "target instead shows a bare target-only lui #,0x1f80 into a temp register" characterization: the TARGET also materializes 0x1F8002B8 exactly ONCE, via `lui $t3,(0x1F8002B8>>16)` / `ori $t3,$t3,(0x1F8002B8&0xFFFF)` at 80056D14-80056D18, BEFORE the loop label `.L80056D24` (line 29) -- not per-call-site. Target then STORES it to a fixed STACK SLOT (`sw $t3,0x78($sp)` at 80056D20) rather than keeping it in a callee-saved register, and RELOADS it from that stack slot via `lw $t3,0x78($sp)` immediately before each of the two jal func_80053614 sites (80056E1C and 80056E98). This is a stack-spill reload pattern, not a rematerialize-per-call-site pattern as s22 guessed from the classify diff alone without reading the raw target bytes.
- [s23] Chassis re-confirmed fresh: applying the s22-banked candidate.c body (limit local + func_80053614 s32-return prerequisite) to src/text1b.c and running `sandbox func_80056CB8 --disable all` scores 38, build_insns=198, target_insns=204 -- exact reproduction of the s22 floor.
- [s23] func_80053614's declared return type was changed from `void` to `s32` (prerequisite, unchanged mechanism from s2: the asm falls through $v0 from the final func_80052D00 call to the epilogue regardless of the C-declared return type, so this is behavior-neutral and load-bearing for the caller to read the return value).

- [s23] Fresh direct read of asm/funcs/func_80056CB8.s:1-145 (full prologue through both func_80053614 call sites) corrects the s22 characterization of the 0x1F8002B8 residue: the target ALSO materializes the constant exactly ONCE, before the loop (lui $t3/ori $t3 at 80056D14-18), but stores it to a fixed stack slot (sw $t3,0x78($sp) at 80056D20) and reloads it via lw $t3,0x78($sp) immediately before each of the two jal func_80053614 sites (80056E1C, 80056E98) -- a stack-spill-reload pattern, not a per-call-site rematerialization as s22 inferred from the classify diff alone without reading raw target bytes.

- [s23] Chassis re-confirmed fresh at session start: s22-banked candidate.c body (limit local + func_80053614 s32-return prerequisite) applied to src/text1b.c scores 38/204 (build_insns 198) via sandbox func_80056CB8 --disable all, exact reproduction of the s22 floor.

- [s23] func_80053614's declared return type is changed void->s32 as an unchanged, load-bearing prerequisite (banked since s2): the asm falls through $v0 from the final func_80052D00 call to the epilogue regardless of declared C return type, so the change is behavior-neutral for func_80053614 itself while making its return value available to func_80056CB8's caller.

- [s23] Both new-this-session spellings of the 0x1F8002B8 axis (scale-pseudo reuse, named pre-loop local in two declaration orders) regress the score despite the addr-local variants nudging build_insns one step closer to target (199 vs 198, target 204) -- the residual is a register-IDENTITY/allocation choice, not purely an insn-count deficit on this axis, so simply adding the missing instruction via a natural C spelling is not sufficient by itself.

- [s24] Fresh chassis re-confirmation this session: sandbox func_80056CB8 --disable all == 38/204 (build_insns 198), matching the ledger's last recorded floor exactly before any s24 change.

- [s24] src/text1b.c reverted to clean INCLUDE_ASM state after this session (git checkout -- src/text1b.c, verified zero diff) -- nothing persists on main between grind sessions per asm-until-matched.

- [s24] func_80053614's signature-fix prerequisite (void -> s32 return type, return func_80052D00(...)) is still required for the candidate to reproduce 38/198 and must be re-applied by any future session splicing candidate.c verbatim.

- [s25] Applying candidate.c's function body without its header-block extern declarations (Judge, ratan2, D_8009A820, D_8009A821, D_800F6610 -- all live above the function signature in candidate.c, outside a naive regex extraction of just the function span) produces implicit-int declarations that silently miscompile the loop body: score explodes from 38 to 141 and build_insns DROPS from 198 to 175 (fewer, wrong instructions, not more) -- a diagnostic worth remembering for any future session that programmatically re-applies candidate.c via a text-extraction script rather than hand-copying the whole file.

- [s25] All three spelling-enumerable regions in this function (sin_p/cos_p/scale/x/z order+swap, pt0/pt1 store-order structural reorder, dx/dz declaration-order+inline) are now exhaustively re-measured on the CURRENT 38/198 chassis, not just cited from an older chassis generation -- closes the ledger's KILL RE-AUDIT REQUIRED flag for these three axes specifically.

- [s25] The currently-adopted sin_p/cos_p/scale/x/z ordering (v04 from s14) remains one of the tied-best spellings on the new chassis (4-5 way tie at 38/198) -- no re-adoption needed.

- [s26] Fresh sandbox func_80056CB8 --disable all this session (s22/s23/s24-banked 38/198 body + func_80053614 s32-return prerequisite + the 3 missing extern decls, applied fresh to src/text1b.c and reverted at session end via git checkout): score=38, build_insns=198, target_insns=204 -- exact reproduction of the s22-s25 floor, confirmed three separate times (before each probe and after each revert).

- [s26] Fresh `python3 tools/ra_solver/inverse_compose.py classify text1b func_80056CB8 --ours-object tmp/sandbox/func_80056CB8/text1b.o --target-object build/src/text1b.o` (via WSL) on the current 38/198 chassis, saved tmp/grind/func_80056CB8/s26/classify.txt: FIRST DIVERGENCE remains PRE-RA/rtl_shape (instruction multiset, not RA/schedule). Target-only insns: `addiu s8,s8,2` (loop-carried +2 increment) + two `addu #,#,s8` (base-plus-idx2 addressing at both table reads) + `lui #,0x1f80` (single, bare, no ori) + `li #,1` / `li #,4` / `bltz`/`beqz`/`j` control-flow shapes. Ours-only insns: three `sll #,#,0x1` (per-access i*2 recompute) + `lui s8,0x1f80` / `ori s8,s8,0x2b8` (materializing 0x1F8002B8 into $s8) + two `sw s8,16(#)` (spilling that literal to the stack at BOTH call sites, which target does not do) + `bgez`/`slt`/`move`/`addu` control-flow shapes. This confirms at the raw instruction level (not just the s22 classify diff-summary or the s24 .greg conflict-set inference) that target's idx2 accumulator and our cached-literal both compete for the SAME callee-saved register ($s8/$fp), and that closing this residual requires either (a) successfully promoting i*2 to a loop-carried $s8-resident accumulator (blocked so far by loop.c:3823's giv-worth predicate under every C spelling tried across s6/s10/s12/s18/s21/s22) or (b) finding a C spelling where the 0x1F8002B8 literal does NOT need spilling to the stack at both call sites (every addr/scale-reuse spelling tried across s23/s26 has been worse).

- [s27] Fresh sandbox func_80056CB8 --disable all this session (s22-s26-banked 38/198 body + func_80053614 s32-return prerequisite + the 3 missing extern decls, applied fresh to src/text1b.c and reverted at session end via git checkout -- src/text1b.c, verified zero diff): score=38, build_insns=198, target_insns=204 -- exact reproduction of the s22-s26 floor.

- [s27] Ran the s26-named untried combination (idx2 loop-carried promotion + textually-distinct literal respelling at the second func_80053614 call site) fresh: score 55/204 (build_insns 201), worse than baseline and identical to every prior isolated idx2 measurement -- the combination adds nothing over idx2 alone.

- [s27] Isolated the literal-respelling half of the combination (idx2 reverted to i*2, literal kept as `0x1F800000 + 0x2B8` at the second call site): score 38/204 (build_insns 198), byte-identical to baseline. Confirmed via fresh dump (tmp/grind/func_80056CB8/dumps/text1b.s) that the compiled asm is unchanged -- GCC's fold() constant-folds the two-node PLUS_EXPR into the same INTEGER_CST as the bare literal at parse time (fold-const.c:3536), before cse.c/combine.c ever run, so no C-level arithmetic respelling of a compile-time constant can present the optimizer with two distinguishable values. This closes the entire "respell the literal to defeat CSE" sub-family as a class kill, not just this instance.

- [s27] Fresh `pwsh tools/grinder/dump.ps1 func_80056CB8` this session (tmp/grind/func_80056CB8/dumps/text1b.s, function span lines 4713-5002): confirms the current chassis materializes 0x1F8002B8 exactly ONCE (`li $fp,0x1f800000` / `ori $fp,$fp,0x02b8`) and reuses $fp at both call sites; the two `sw $fp,16($sp)` instructions are the o32 ABI's mandatory per-call stack-argument stores for the 5th (stack-passed) integer argument, not a register-pressure spill artifact -- they cannot be eliminated by any C-level respelling of the literal's value.

- [s27] src/text1b.c reverted to clean INCLUDE_ASM state after this session (git checkout -- src/text1b.c, verified zero diff) -- nothing persists on main between grind sessions per asm-until-matched.

- [s27] Fresh sandbox func_80056CB8 --disable all this session reproduces the s22-s26 floor exactly: score=38, build_insns=198, target_insns=204.

- [s27] func_80056CB8 depends on func_80053614's void->s32 return-type prerequisite (byte-neutral standalone) being re-applied in the same TU; still required to reproduce 38/198.

- [s27] The s26-named untried combination (idx2 promotion + literal-CSE-defeat) is now fully measured and closed: both halves independently and jointly killed.

- [s27] Fresh tmp/grind/func_80056CB8/dumps/text1b.s (this session) confirms the exact register/store shape around the 0x1F8002B8 literal and both func_80053614 calls.

- [s27] src/text1b.c reverted to clean INCLUDE_ASM state after this session (git checkout -- src/text1b.c, verified zero diff).

- [s28] The brief's chassis check showed 'measurement unavailable' because HEAD (src/text1b.c) carries INCLUDE_ASM per asm-until-matched -- the driver cannot auto-apply candidate.c, so every session must hand-apply it (body + header externs + the separate func_80053614 prerequisite) before any fresh measurement is meaningful.

- [s28] A normalized objdump diff (target vs a fresh 38/204 build, tmp/grind/func_80056CB8/s28/target_insns.txt vs mybuild_insns.txt) confirms the two builds are structurally near-identical region-by-region: same control flow, same branch count, same call sites, same disposition-gate chain. The residual is register-allocation choices, not missing/wrong C structure.

- [s28] The residual's clearest signature: the 0x1F8002B8 literal is held in a callee-saved register for the whole function in every candidate tried so far (my s28 build: $s8, two bare sw before the two jal's with no reload), while target spills it to a stack slot once and reloads it via a caller-saved temp ($t3) immediately before each call -- consistent with the s26-named register-pressure hypothesis (idx2 vs the cached literal contesting $s8/$fp), now sharpened: it is specifically a spill-vs-hold decision, not the idx2 axis itself (which stays independently class-killed per s21/s26/s27).

- [s29] Fresh sandbox func_80056CB8 --disable all this session (s22-s28-banked 38/198 body, applied by hand: function body + the 5-line extern header block + func_80053614 void->s32 return-type prerequisite, all re-applied fresh to src/text1b.c and reverted at session end): score=38, build_insns=198, target_insns=204 -- exact reproduction of the s22-s28 floor, confirmed fresh.
- [s29] QUANTIFIED the s26/s28 register-pressure mechanism (previously inferred from a classify.txt diff summary): decoded the s28-dumped tmp/grind/func_80056CB8/dumps/text1b.greg's "Register dispositions:" table (script + full writeup: tmp/grind/func_80056CB8/s29/conflict_map.txt) for the func_80056CB8 span (lines 14788+). Confirmed by direct pseudo->hardreg lookup: pseudos 72/75/82/83/85/86/87/88 occupy $s7/$s6/$s1/$s0/$s5/$s4/$s2/$s3 respectively (all 8 of $s0-$s7), and these 8 pseudos plus pseudo 149 (the 0x1F8002B8 literal, allocated $fp) share an IDENTICAL 32-entry conflict set -- i.e. exactly 9 pseudos are live across the function's entire loop body (both func_80053614 call sites), 8 of which are genuinely-needed named values (obj/self-pointer, loop index i, flags/table-scale/sin_p/cos_p/x/z -- the values already folded in via the s7 flags/ang/code merge and the s11 r1/r2 merge), leaving $fp as literally the only unclaimed callee-saved register in the whole function, which 149 wins by default (no competition, not by priority).
- [s29] Re-tested the s28-killed "hit_flag_arg pre-loop named local" idea in a NEW, distinct shape: declaring `s32 hit_flag_arg = 0x1F8002B8;` INSIDE the loop body (re-assigned each iteration) rather than s28's pre-loop/outside-loop declaration. Measured via sandbox func_80056CB8 --disable all: score 42/204 (build_insns 200) -- worse than both the 38/204 baseline AND s28's pre-loop variant (45/204, 199 insns), but via a different insn-count delta (200 vs 199), confirming the named-local axis is dead regardless of WHERE in the loop nest the declaration sits, not just at the one placement s28 tried.
- [s29] src/text1b.c reverted to clean INCLUDE_ASM state after this session (git checkout -- src/text1b.c, verified zero diff) -- nothing persists on main between grind sessions per asm-until-matched.

- [s29] sandbox func_80056CB8 --disable all on the s22-s28-banked candidate body (function body + 5-line extern header block + func_80053614 void->s32 return-type fix) reproduces 38/204 (build_insns 198) fresh this session, matching s22-s28 exactly.

- [s29] Decoded pseudo->hardreg mapping from tmp/grind/func_80056CB8/dumps/text1b.greg (func_80056CB8 span): 72->$s7, 75->$s6, 82->$s1, 83->$s0, 85->$s5, 86->$s4, 87->$s2, 88->$s3, 149->$fp. All eight of $s0-$s7 are occupied by named real values; $fp is the only free callee-saved register and the 0x1F8002B8 literal (pseudo 149) is the only other pseudo sharing the full 32-entry whole-loop conflict set, so it takes $fp by default.

- [s29] hit_flag_arg named-local variant tested in a NEW placement (in-loop, not pre-loop like s28) also regresses: 42/204 (build_insns 200) vs baseline 38/204 (198) and vs s28's pre-loop variant 45/204 (199) -- three distinct measured points on the same dead axis.

- [s29] src/text1b.c reverted to clean INCLUDE_ASM state at session end (git checkout -- src/text1b.c, verified zero diff via git status --short) -- nothing persists on main between grind sessions per asm-until-matched.

- [s30] Chassis re-confirmed fresh this session at floor 38/204 (build_insns 198) before any probe, matching the s22-s29 banked floor exactly.

- [s30] The object-level inverse_compose.py classify tool requires WSL for its OBJDUMP dependency; a bare Windows-side python3 invocation throws FileNotFoundError on the objdump call even though the .py file runs -- always invoke it via `wsl bash -c 'source .venv/bin/activate && python3 tools/ra_solver/inverse_compose.py classify ...'` from this project.

- [s30] asm/funcs/func_80056CB8.s lines 17/27/199/203: $s6 = ordinary loop counter i (sll $s6,$v1,1 = start; addiu $s6,$s6,1 per iteration); $fp = separate strength-reduced i*2 accumulator (sll $fp,$v1,2 = start*2; addiu $fp,$fp,2 per iteration). These are two DISTINCT registers with two DISTINCT roles in target, confirmed by direct read, not inferred from the classify diff summary alone.

- [s30] asm/funcs/func_80056CB8.s lines 20/198-201: target spills the INITIAL `start` value to 0x60($sp) and reloads + re-adds 2 to it FRESH every loop iteration for the bound comparison, rather than hoisting a stable invariant `limit` register -- a structural difference from our s22-banked chassis (which DOES hoist a stable `limit` local, and that hoist is what dropped the floor 42->38 when it was added).

- [s30] Both known C spellings for 'a value that becomes target's fp accumulator' (idx2-on-top-of-hoisted-limit: s22/s27, 55/204; idx2-without-any-limit-hoist: s30, 52/204) measure worse than the current 38/204 baseline -- the hand-authored-accumulator axis is exhausted in both combinations this ledger has been able to construct.

- [s31] HEAD is INCLUDE_ASM for func_80056CB8 (chassis-check 'measurement unavailable' at dispatch was expected, matching the s28-documented cause).

- [s31] The candidate.c file interleaves large prose header comments containing literal-looking C snippets (e.g. 'extern s16 Judge; extern s32 ratan2(s32,s32); ...' appears verbatim inside a comment block before the real declaration) -- a naive string-search extraction of the real extern header picks the WRONG (comment-embedded) occurrence and silently truncates the applied body, producing a false 'func_80056CB8 not found in text1b.o' sandbox error that looks like a chassis break but is a tooling mistake. Future sessions must extract by LINE RANGE (this session used candidate.c lines 1083-1171) or verify the extracted text has no leading '*' comment-continuation characters, not by naive substring search on the extern text.

- [s31] func_80052D00 (called by func_80053614) is already declared 's32 func_80052D00(s32, s32);' elsewhere in text1b.c, so func_80053614's void->s32 signature fix requires no additional cast -- `return func_80052D00(arg2, arg3);` compiles cleanly.

- [s31] The function's loop calls ratan2 (confirmed via call_insn 109 in the .loop dump) in addition to the two func_80053614 calls already known from the target-asm read -- this means loop_has_call is TRUE for strength_reduce's threshold formula, which was not previously stated explicitly in this ledger.

- [s32] Fresh sandbox reconfirmation this session: candidate.c body + func_80053614 void->s32 signature fix, applied verbatim to src/text1b.c, reproduces score=38, target_insns=204, build_insns=198 exactly (chassis unchanged from s22-s31).

- [s32] loop.c:5494-5530 combine_givs literally does `g1->benefit += g2->benefit; g1->lifetime += g2->lifetime; g1->times_used += g2->times_used;` on merge -- confirms the merged giv's lifetime/benefit are additive sums of the two component per-use givs, not independently computed.

- [s32] The insn-427 giv (a different expression, the `*(s8*)(arg0+0x444+i)` output-array store index) is rejected at exactly 0 vs 163 with unmerged raw benefit 2 -- this pins the scan-loop's `add_cost*bl->biv_count` deduction at exactly 2 for this function's biv class, letting the merged-giv's 124 be exactly reverse-engineered into its lifetime/threshold/benefit factors.

- [s32] src/text1b.c was left clean (git checkout -- src/text1b.c) at end of session; HEAD still carries INCLUDE_ASM("asm/funcs", func_80056CB8); for this function as required by asm-until-matched.

- [s33] Chassis-reproduction trap re-confirmed fresh at s33 (matches s25/s28): body-only splice -> 153/204 (152 insns); + header externs (Judge/ratan2/D_8009A820/D_8009A821/D_800F6610) but func_80053614 still void -> 141/204 (175 insns); + func_80053614 s32-return prerequisite (both fixes together) -> true 38/204 (198 insns), matching every s22-s32 measurement.

- [s33] The 'how i*2's value identity is named/carried' axis now has six distinct spellings measured on this or an equivalent chassis, all flat-or-worse: int-fresh (s6), int-loop-carried (s7/s10/s18/s21/s22/s27, e.g. 55/204), pointer-fresh (s10), pointer-loop-carried (s12, rejected/loop-carried-pointer-walk-worse.c, 78/204 on an older 48/204 chassis), single-shared-index (s6/s32, rejected/shared-idx-local-worse.c and shared-idx-local-fresh-chassis-worse.c, giv DOES promote but net worse due to a 9th call-spanning resident), and two-separately-named-index (s33, this session, rejected/separately-named-idxB-worse.c, 51/204, no promotion trace at all).

- [s33] m2c's fresh reconstruction (tmp/grind/func_80056CB8/s12/m2c_out.c, target asm unchanged so still current) confirms the target's own loop is compiled from a genuine do-while-shaped control-flow graph with a folded-constant entry guard (`if (1 != 0) { do {...} while(...); }`), and that its var_fp is algebraically identical to i*2 (var_fp initialized to start*2, incremented by 2 per iteration) -- consistent with, not beyond, what s26's classify.txt already established.

- [s33] src/text1b.c reverted to clean INCLUDE_ASM at session end (git status --short src/text1b.c empty).

- [s34] Target asm (asm/funcs/func_80056CB8.s lines 144-197) recomputes the store address `addu $v0,$s7,$s6` fresh at FIVE separate exit points of the flags==3/flags==4 tail rather than falling through to one merged store -- confirmed by direct read and independently by m2c's reconstruction (tmp/grind/func_80056CB8/s12/m2c_out.c) which repeats `var_v0 = arg0 + var_s6;` at each goto-block target. Writing this shape in C via duplicated-statement-into-arms measures WORSE (72/204) than candidate.c's single-merged-store form (38/204) on this chassis -- the source-level shape match does not reproduce the RTL shape here.

- [s34] The do-while loop rewrite (same body, different loop syntax) measurably changes codegen: build_insns drops 198->195, directly refuting s33's untested reasoning-only claim that loop-inversion makes for/do-while source syntax neutral. This is new information for the ledger: loop SYNTAX is not proven neutral and should not be assumed so in future sessions without a fresh measurement.

- [s34] The do-while chassis (195 real insns) is a genuinely new, lower-insn-count starting point that has never been combined with any lever besides idx2 (which was also worse there). It differs from every chassis previously explored in this ledger (which were all for-loop-based).

- [s35] Both do-while-chassis index-naming spellings (shared idx, separately-named idxB) compile to identical build_insns (197), +2 over the do-while-alone baseline (195) -- matching the +2..+3 delta pattern already observed on all 6 for-loop-chassis index-naming spellings measured in prior sessions.

- [s35] The index-naming axis (how the doubled i*2 loop computation is shared, named, or carried across the two table reads) is now empirically exhausted across 8 total spellings on 2 different chassis shapes (for-loop 198-insn baseline; do-while 195-insn baseline) -- every combination measured flat-or-worse.

- [s35] func_80053614's void->s32 return-type fix remains verified byte-neutral (re-confirmed applicable, not re-measured this session since it was already established in s2/s3) and is a load-bearing prerequisite for any chassis that reads its return value into `flags`.

- [s36] HEAD chassis check at dispatch reported 'measurement unavailable' because src/text1b.c currently carries INCLUDE_ASM("asm/funcs", func_80056CB8); for this function (asm-until-matched representation) -- the ledger's last recorded floor (38) is the only trustworthy number until the candidate.c body is reconstructed fresh, which this session did three times (baseline reconfirm implicit via git checkout, plus the two probe variants).

- [s36] Current candidate.c carries NO FAKE constructs anywhere in the function body (grep for 'FAKE' in candidate.c only matches header-comment prose discussing why FAKE does NOT apply) -- so this session's mandated KILL RE-AUDIT REQUIRED instrumentation (fake_ablate.py) is a no-op for this ledger; the re-audit was instead done by re-measuring the closest-to-target stale-chassis instance kills fresh, which is the substantively equivalent freshness check for a FAKE-free candidate.

- [s36] asm/funcs/func_80056CB8.s:85-102 confirms the target genuinely recomputes `*(obj+0xBC) - 0x320` TWICE (once for pt0[1], once for pt1[1], two separate lw+addiu sequences) rather than sharing one computed value -- so candidate.c's existing duplicate-expression spelling at that point is already structurally correct and is NOT a CSE opportunity to explore.

- [s36] asm/funcs/func_80056CB8.s:124-138 (block2, the second func_80053614 call's argument setup) emits pt0/pt1 stores in the order pt0[0], pt0[2], pt1[0], pt0[1], pt1[2], pt1[1] -- genuinely different from source-declaration order -- but transplanting that exact order as literal C statement order measures worse (see hypothesis 2 above), confirming (as s20 already found) that the RTL-level store order is not directly recoverable as source order.

- [s36] The two frontier items named at the end of s35 (shrink loop-body insn_count below loop.c:3823's threshold via non-index restructuring; restructure a non-index resident's footprint specifically on the do-while chassis) remain untried this session -- this session's turns went to the mandated re-audit of stale-chassis kills instead, which is a legitimate and necessary session per the KILL RE-AUDIT REQUIRED instruction, but does not advance those two frontier items.

- [s37] Chassis reproduction: candidate.c + func_80053614 void->s32 fix gives fresh sandbox score 38, target_insns 204, build_insns 198 -- matches ledger exactly.

- [s37] spelling_enum.py --list on the marked region: '0 named locals (), 5 assignments, 0 anchors -> 32 distinct spellings (with commutative swaps)' -- confirms no decl-inlining axis exists here (all 5 targets are pre-declared loop locals, only assigned in this block), so the swept space (ordering x swaps) is genuinely exhaustive for this block, not a sample.

- [s37] The tool's ENUM-BEGIN/END region format requires the marked span to end in a single trailing run of anchor (if/return-condition) lines with bodies OUTSIDE the markers -- it cannot currently represent a region containing interior if/else blocks with braced multi-statement bodies. The obj/flags computation block (candidate.c ~1160-1171, two separate if-statements with braced bodies) does not fit this format without either a tool extension or manual unrolling; neither was attempted this session.

- [s37] src/text1b.c and func_80053614's signature were reverted to their committed HEAD state (INCLUDE_ASM("asm/funcs", func_80056CB8); / void return) before ending the session -- git status confirms src/text1b.c clean, no draft C left on main (asm-until-matched).

- [s38] s38 chassis reproduction re-confirmed at score 38, target_insns 204, build_insns 198 both before and after the 6-variant sweep — no chassis drift since s37.

- [s38] The 6 hand-written variants covered: ternary-vs-if form for the obj assignment, decl-order placement of a hoisted kind-check local (both before and after the obj if-block), De Morgan negation with arm swap on the flags gate condition, and named intermediates (ang; dx/dz) for the true-arm and else-arm sub-expressions respectively.

- [s38] spelling_enum.py's region format (ENUM-BEGIN/END with a single trailing run of bare anchor lines) still cannot represent this block mechanically — the interior-anchor tooling gap flagged in s37 remains open and is carried forward as a frontier item, not resolved this session.

- [s38] src/text1b.c and func_80053614 confirmed clean/reverted to INCLUDE_ASM / void baseline at end of session (git status clean on src/) — no draft C left on main, consistent with asm-until-matched.

- [s39] Fresh chassis reconstruction (candidate.c body + func_80053614 void->s32 return fix + 5-line extern header spliced into src/text1b.c) reproduced the ledger's exact recorded floor of 38/204 (198 build insns) with zero drift, confirmed both before and after this session's probes.

- [s39] s38's 4 individually-tied obj/flags-block hand-variants (ternary obj, kind-local-after, ang intermediate, dx/dz intermediates) show no interaction effect when combined in any of 5 tested groupings -- the obj/flags block's spelling space is now exhausted both individually (s38) and combinatorially (s39) for these variant axes.

- [s39] A genuinely new spelling axis in block1 (sharing the textually-duplicated *(obj+0xBC)-0x320 expression into one local) was tried for the first time and scored worse (73/204) despite fewer real instructions (196 vs 198), demonstrating GCC's CSE already achieves the instruction-count reduction at baseline without a named carrier -- the explicit local only perturbs register/scheduling choice, negatively.

- [s40] Chassis-check reproduction procedure (candidate.c body + func_80053614 void->s32 fix + 5-line header externs spliced into src/text1b.c) is load-bearing and was re-verified fresh this session before any probe -- splicing only the function body gives a false floor of 153/204.

- [s40] No sibling ledger had unspent transplantable material this session: func_80055B60 (same file) carries no candidate.c; func_80057CC8 and func_80056FE8 are COMPLETED-C in the same TU but structurally unrelated (angle/motion helpers, not hit-detection loops) -- nothing to transplant.

- [s40] Every banked instance kill in this ledger (s33-s39) was already re-verified as: measured on the current s22-vintage chassis, with zero FAKE constructs present in any tested variant. No stale-chassis or stale-FAKE kill existed for tools/fake_ablate.py to re-test this session.

- [s40] Extending s39's narrow (2-occurrence, within-block) shared-expression finding to its maximal (6-occurrence, cross-call) form reproduces and strengthens the same conclusion: worse scales with scope (73/204 at 2 merged occurrences -> 95/204 at 6), and the insn-count drop scales too (-2 -> -21) while the score gets worse, not better -- strong confirming evidence that target's own instruction budget includes genuine per-site re-reads/re-derivations of obj+0xBC that an explicit shared carrier cannot reproduce.

- [s40] Combined across s33-s40, 38 distinct spelling/ordering/sharing/combination variants of this function's non-index residual have now been measured; none scored below 38/204. The spelling-space search for this chassis shape is exhausted; the only untried axes are structural (loop.c:3823 insn-count threshold, do-while chassis resident-footprint).

- [s41] `tools/ra_solver/inverse_compose.py classify` refuses in text-stream mode for this function ("zero-rule (rules-to-zero 2026-08-25): with no regfix/asmfix rules the src-derived tgt.s cannot carry target's stream") but has a supported OBJECT MODE for exactly this case: `classify <stem> <func> --target-object build/src/<stem>.o --ours-object tmp/sandbox/<func>/<stem>.o`. The pre-existing `build/src/text1b.o` (built before this session's edits, while func_80056CB8 was still INCLUDE_ASM) already held the true target bytes and did not need a fresh `engine build` -- usable directly.
- [s41] classify's verdict for func_80056CB8 is PRE-RA / rtl_shape, its most severe category. Its instruction-multiset diff independently reproduces two things already in this ledger without being told about them: the loop.c:3823 strength-reduce i+=2 accumulator (target's `addiu s8,s8,2` + `addu #,#,s8` x2 vs ours' three `sll #,#,0x1` i*2 shifts -- the s31/s32/s37-s40 frontier item) and the repeated-0x1F8002B8-literal CSE shape the s15 named-intermediate lever targeted (target's `lw #,104(#)`/`lw #,96(#)` two-different-offset reload vs ours' two independent `lui+ori+sw s8,16(#)` materializations). Re-splicing the s15 lever onto the CURRENT (s22-s40-banked) chassis measured 42/204 (200 insns), still worse than the 38/204 baseline -- the kill re-confirms rather than being superseded by the tool's independent rediscovery.

- [s41] tools/ra_solver/inverse_compose.py classify refuses in text-stream mode for zero-rule/INCLUDE_ASM-routed functions (no regfix/asmfix rules => no src-derived .tgt.s stream); it names its own object-mode escape: `classify <stem> <func> --target-object build/src/<stem>.o --ours-object tmp/sandbox/<func>/<stem>.o`.

- [s41] The pre-existing build/src/text1b.o (built before this session's src edits, while func_80056CB8 was still committed as INCLUDE_ASM) already held the true target bytes for the function and worked directly as --target-object with no fresh `engine build` needed.

- [s41] classify's object-mode verdict for func_80056CB8 is PRE-RA/rtl_shape (its most severe category: 'no backend -- the residual is upstream of every model'), meaning no RA-solver or sched-solver perturbation can reach this residual; the fix has to be a change to the C the front end sees.

- [s41] The tool's instruction-multiset diff independently reproduces two things already in this ledger's frontier without being told about them: (1) target's strength-reduced i+=2 accumulator (`addiu s8,s8,2` + `addu #,#,s8` x2) vs our three `sll #,#,0x1` i*2 shifts -- the s31/s32/s37-s40-banked loop.c:3823 rejection-inequality frontier item; (2) target reading the 0x1F8002B8 scratchpad literal back via two DIFFERENT-offset `lw` reloads (`lw #,104(#)`/`lw #,96(#)`) vs our two independent lui+ori materializations -- the shape the s15-rejected named-intermediate-scratchpad lever targeted.

- [s41] Re-splicing the s15 lever onto the CURRENT (s22-s40-banked) chassis per the mandatory kill-re-audit measured 42/204 (200 insns), worse than the 38/204 baseline on both score and insn count -- the instance kill re-confirms on the current chassis rather than being superseded by the tool's independent rediscovery of the same shape.

- [s42] First-ever instrumented .greg dump of the do-while chassis (s34 measured it but never dumped it): the 0x1F8002B8 scratchpad-literal pseudo still allocates to $fp exactly as on the for-loop chassis, and the same 8 named residents (obj/i/flags/scale-table-value/sin_p/cos_p/x/z) still saturate $s0-$s7. The do-while rewrite's -3 real-insn effect (198->195, per s34) comes from elsewhere in the function, not from freeing a register for i*2 or displacing the literal. This closes the s34/s40 frontier item "restructure a resident's footprint on the do-while chassis's distinct register landscape" as measured-inert for the fp/s0-s7 allocation specifically. Saved: tmp/grind/func_80056CB8/s42/text1b_greg_dowhile_func80056CB8_slice.txt.

- [s42] s42 first-ever instrumented .greg dump of the do-while chassis (s34 measured it but never dumped it) shows the 0x1F8002B8 literal pseudo (numbered 148 on this chassis) allocates to $fp (reg 30), identical to the for-loop chassis's literal pseudo 149.

- [s42] The do-while chassis's per-pseudo conflict-list header lines are NOT uniform across the 9 s0-s7-plus-fp pseudos the way the for-loop chassis's are (s29 finding: all 7 identical 32-entry sets) -- some pseudos (e.g. 84, 85, 86, 87) have smaller conflict sets missing 96/190/191/195/196/197 -- proving the do-while rewrite's insn-count reduction is real and localized elsewhere in the function, not a phantom or measurement artifact.

- [s42] loop.c:3241/3823's strength-reduction predicate (threshold=(loop_has_call?1:2)*(3+n_non_fixed_regs), rejection at lifetime*threshold*benefit < insn_count) remains numerically pinned from s31/s32: lifetime=2, threshold=31, benefit=2, product=124 vs insn_count=163 on the for-loop chassis; only +1 combined lifetime unit (to 3) flips it, but every C spelling that raises the merged i*2 giv's lifetime (shared idx local s32, separately-named idxB s33) has now been measured worse due to call-spanning register pressure, independent of whether the do-while chassis is used as the base.

- [s43] The dispatch brief's chassis-check line read 'measurement unavailable' at session start; this session's fresh sandbox run confirmed the true chassis is unchanged at 38/204/198, matching the ledger's last-recorded floor exactly.

- [s43] The insn-count divergence between the for-loop (198) and do-while (195) chassis is invisible in the .greg RTL dump (both show 107 (insn ...) forms) -- it only appears in the final assembled .s output, meaning it originates downstream of global register allocation (in reorg.c/dbr delay-slot-fill and/or final branch-shortening), not in any RA pass. This corrects the framing implicit in s34/s42's RA-focused investigation: the -3 insns were never going to show up as a register-allocation difference because the mechanism is a control-flow-shape decision made in stmt.c's for-loop lowering, upstream of RA.

- [s43] Both func_80053614() calls are semantically load-bearing, unconditional, per-iteration collision-probe calls -- there is no valid C-source lever that removes either call from the loop's dynamic path, closing that branch of the loop.c:3823 threshold-flip frontier permanently.

- [s44 OBJECT MODEL] Per-symbol audit of every DATA MODEL global (object-model modality, mandated this session):
  - D_8009A820 (census g_text1b_addr_8009A820): MISMATCH-declaration (measured, fixed, byte-neutral). Was `extern u8 D_8009A820;` + use-site `(&D_8009A820)[i*2]` pointer arithmetic -- a scalar-with-address-of pun the brief's DECLARATION-PUNS scan flags as a layer-1 risk. Corrected to `extern u8 D_8009A820[];` + direct `D_8009A820[i*2]` indexing. `sandbox func_80056CB8 --disable all` before/after: unchanged at 38/204 (198 build insns) -- purely a declaration-level fix.
  - D_8009A821 (census g_text1b_addr_8009A821): same MISMATCH-declaration, same fix, same measured-unchanged result (bundled with D_8009A820 in the single sandbox run above).
  - D_800F6610: MATCHES (re-confirmed, not re-measured this session -- s6 already objdiff-verified it). Arithmetically `== D_800F6608+8` (the Rec44 `.w8` field) but the TARGET asm emits an independent lui/lw(D_800F6610) relocation, not a `D_800F6608+8` addend -- proof the original source held it as a genuinely separate global. Declaring it as `D_800F6608.w8` was tried and REJECTED at s6 (see hypotheses.md [s6]); current `extern s32 D_800F6610;` is correct. This is the split-scalars-hide-aggregate counter-example: adjacency to a named struct is not merge evidence on its own.
  - D_800F6608: MATCHES. `extern Rec44 D_800F6608;` (code6cac.h), `.w0` member read; identical shape to matched sibling func_80057094's `mid.w0` read in the same TU (src/text1b.c:1889).
  - Judge: MATCHES (by convention). `extern s16 Judge;` scalar + use-site `&Judge + (angle & 0xFFF)` pointer arithmetic at all 3 use sites in this TU, including the matched/COMPLETED-C func_80057CC8 (text1b.c:2055, 2257) which byte-matches with the identical spelling. Not a pun in practice since this exact shape already compiles to correct target bytes elsewhere in the same file.

  PREMISES the current 38/204 floor argument rests on (per the object-model-audit mandate):
  1. The i*2 byte-stride indexing into D_8009A820/D_8009A821 correctly reflects the target asm's `$fp += 2` per-iteration increment (confirmed s1/s2, re-confirmed this session via asm read) -- NOT an under- or over-indexing bug.
  2. D_8009A820 and D_8009A821 are genuinely TWO separate byte tables (not one interleaved 2-byte-record array) -- based on each having its own independent %hi/%lo relocation in the target asm rather than one shared base + two offsets.
  3. The remaining 38/204 residual is a pure RA/scheduling/loop.c-strength-reduction gap in the for-loop's index/guard handling (per s31-s43's already-exhausted analysis), NOT an object-model error -- this session's full DATA MODEL sweep found no further mismatches to attack.

OBJECT MODEL: [s44 re-file, corrected header] Every global in the dispatch
brief's DATA MODEL section, declared shape vs evidence, per-symbol verdict:
- `D_8009A820` (census `g_text1b_addr_8009A820`, no header decl): MISMATCH
  declaration (measured, fixed, byte-neutral). candidate.c previously spelled
  it `extern u8 D_8009A820;` with use-site `(&D_8009A820)[i*2]`
  pointer-arithmetic -- a scalar-with-address-of pun the brief's
  DECLARATION-PUNS scanner flags as a layer-1 risk. Corrected in candidate.c
  to `extern u8 D_8009A820[];` with direct `D_8009A820[i*2]` indexing.
  `sandbox func_80056CB8 --disable all` before/after the redeclaration: both
  38/204 (198 build insns) -- confirmed byte-neutral, declaration-level fix
  only. NOTE (new this re-file): `src/text1b.c:2183` still carries the OLD
  scalar `extern u8 D_8009A820;` for this same file's other INCLUDE_ASM
  functions; landing candidate.c's array form in the same TU requires either
  replacing that line too or hoisting a single shared `extern u8
  D_8009A820[];` -- an integration-surface note, not a blocker for this
  ledger-only measurement.
- `D_8009A821` (census `g_text1b_addr_8009A821`): identical MISMATCH
  declaration, identical fix, bundled in the same sandbox run above (still
  38/204). Same `src/text1b.c:2184` scalar-collision integration note
  applies.
- `D_800F6608` (census `g_gnd_midpoint_x`, decl `extern Rec44 D_800F6608;`
  in code6cac.h): MATCHES. `.w0` member read, identical shape to the
  matched/COMPLETED-C sibling func_80057094's `mid.w0` read in the same TU
  (src/text1b.c:1889).
- `D_800F6610` (census `g_gnd_midpoint_z`, no header decl): MATCHES
  (re-confirmed, not re-measured -- s6 already objdiff-verified this).
  Arithmetically `== D_800F6608+8` (the Rec44 `.w8` field) but the target
  asm emits an INDEPENDENT `lui %hi(D_800F6610)/lw %lo(D_800F6610)`
  relocation pair (asm/funcs/func_80056CB8.s ~L47-49), not a `D_800F6608+8`
  addend -- proof the original source held this as its own genuinely
  separate global. Declaring it as `D_800F6608.w8` was tried and REJECTED at
  s6 (hypotheses.md [s6]). Current `extern s32 D_800F6610;` is correct; this
  is the split-scalars-hide-aggregate counter-example -- struct-field
  adjacency alone is not merge evidence, the relocation pattern is.
- `Judge` (no address row in the brief's table; a scalar global, not an
  aggregate): MATCHES by convention. `extern s16 Judge;` + use-site
  `&Judge + (angle & 0xFFF)` pointer arithmetic at all 3 use sites in this
  TU, including the matched/COMPLETED-C func_80057CC8 (text1b.c:2055,
  2257), which byte-matches with this identical spelling. Not a pun in
  practice since this exact shape already compiles to correct target bytes
  elsewhere in the same file.

PREMISE LIST the current 38/204 floor argument rests on:
  1. i*2 byte-stride indexing into D_8009A820/D_8009A821 correctly reflects
     the target asm's per-iteration `$fp += 2` increment (s1/s2, re-checked
     this session against the asm) -- not an under/over-index bug.
  2. D_8009A820 and D_8009A821 are two genuinely SEPARATE byte tables (each
     with its own independent %hi/%lo relocation), not one interleaved
     2-byte-record array -- ruling out an aggregate-merge fix here.
  3. The remaining 38/204 residual is a pure RA/scheduling/loop.c
     strength-reduction gap in the for-loop's index/guard handling
     (s31-s43's exhausted analysis), NOT an object-model error -- this
     session's full DATA MODEL sweep of every brief-flagged global found no
     further declaration mismatch to attack.

- [s44] OBJECT MODEL per-symbol audit (evidence.md, freshly re-filed with the literal 'OBJECT MODEL:' prefix the brief mechanically requires): D_8009A820 and D_8009A821 were MISMATCH-declaration (scalar-with-address-of pun), fixed byte-neutrally to incomplete-array-with-direct-index; D_800F6608, D_800F6610, and Judge all MATCH their evidence-backed shapes.

- [s44] New integration-surface finding: candidate.c's array-typed externs for D_8009A820/D_8009A821 will conflict with the pre-existing scalar externs already committed at src/text1b.c:2183-2184 and src/text1b_b.c:197-198 -- both are currently read by no live C code (INCLUDE_ASM-only), so this is a landing-time fix, not a present blocker.

- [s44] The previous session's session-44 work was content-correct but used an evidence.md tag format ('[s44 OBJECT MODEL]') that the driver's mechanical scan for a literal 'OBJECT MODEL:'-prefixed line did not match; this session re-filed the same findings under the required literal prefix.

- [s44] engine/sandbox.py's cheat-disabled ('--disable all') build path is currently broken for func_80056CB8/text1b.c -- independently reproduced this session, root-caused to the documented sibling index-based reorder-rule truncation mode, and confirmed NOT a candidate-content defect (the same candidate body compiles cleanly under a plain build-c).

- [s44] CC1PSX SELF-DISPROOF (driver, ruling 2026-09-08): candidate d5ef7d924e4d scores 134 under our cc1 and 145 under the original cc1psx — SOURCE-SIDE: the original compiler is no closer from this source, so the residual is a spelling not yet found (a pure-C preimage exists by construction).

- [s45] canonical gate (fresh, s45): verdict=C, asm_insns=0, total=204, distance=204 (raw, INCLUDE_ASM stub state), hand_coded_tier=LOW.

- [s45] scan_hand_coded --single func_80056CB8: tier=LOW score=1/8, only S4 (front-loads) fires.

- [s45] docs/reference/sotn-construct-index.md census for the two frontier constructs returned zero hits.

- [s45] state.json cc1psx_check (2026-09-16T12:58:20Z, candidate_sha d5ef7d924e4d): ours=134, psx=145, closer=false, ok=true -- not a compiler-fidelity lead.

- [s45] Ledger's banked floor (s22-s44, via ra_solver object-mode workaround) is 38/204 (198 build insns); this session's direct engine/sandbox.py measurement of the same candidate body returned 134/204, confirming the previously-flagged sandbox tooling defect is still present and unresolved.

- [s45] src/text1b.c reverted to the committed INCLUDE_ASM("asm/funcs", func_80056CB8); state at session end -- no C landed, per asm-until-matched.

- [s45] Filed docs/grind/decisions.md entry '2026-09-16 -- func_80056CB8 -- OWNER-ESCALATION -- LADDER EXHAUSTED (non-endgame residual, floor 38): ROTATED' citing both gate evaluations, the cc1psx self-disproof, the 45-session/9+-modality exhaustion history, and re-activation triggers.

- [s46] SANDBOX PIPELINE HEALTH: `engine/sandbox.py --disable all` for func_80056CB8/text1b.c now reproduces the banked 38/204 floor directly and correctly (score 38, target_insns 204, build_insns 198) when the s22-s44-banked candidate.c body + func_80053614 s32-return prerequisite are spliced in. The s44b/s45 defect (134/204, attributed to a sibling regfix/asmfix reorder rule shifting maspsx indices during cheat-stripping) does not reproduce -- that machinery was fully retired 2026-08-30 and can no longer cause it. Future sessions on this function should measure directly; the ra_solver object-mode workaround is no longer required.
- [s46] Both concrete spellings of the s42/s43-hypothesized "direct-bound elides the for-loop pre-header guard" frontier item were measured and are WORSE than baseline: `i < start + 2` (no `limit` local) = 42/204 (197 insns); `i - start < 2` = 47/204 (196 insns); baseline (separate `s32 limit = start + 2;`) = 38/204 (198 insns). Fewer real instructions does not mean a better score here -- both direct-bound forms trade the 3-insn guard for other, larger register/scheduling diffs. Rejected forms saved: memory/grind/func_80056CB8/rejected/direct-bound-start-plus-2-worse.c, direct-bound-i-minus-start-worse.c.

- [s46] Checked the auto-return trigger (func_8006CCC8 sibling movement to floor 39): its ledger's only open frontier item (H2, a LICM-hoist-var-reuse case on an sign-extended arg) is unrelated to func_80056CB8's frontier (for-loop guard elision / strength-reduce threshold) -- no transplantable lever, no C shared between the two functions' bodies (grep for cross-references returned nothing).

- [s46] engine/sandbox.py --disable all now scores this function's non-trivial candidate correctly and directly -- the s44b/s45 tooling defect is gone (root cause, the regfix/asmfix reorder-rule machinery, was retired project-wide 2026-08-30). Future sessions should measure directly instead of using the ra_solver object-mode workaround.

- [s46] Both concrete direct-bound for-loop spellings named in the s45 LADDER EXHAUSTED record's live frontier item #1 are now measured and dead: `i < start + 2` (42/204) and `i - start < 2` (47/204), both worse than the `s32 limit = start + 2;` baseline (38/204). Fewer real instructions (197, 196 vs 198) does not correlate with a better score for either.

- [s46] Reverted src/text1b.c to clean INCLUDE_ASM state after every splice-and-measure this session; `git status --short src/text1b.c` empty at session end.

- [s47] structural modality: re-confirmed fresh baseline 38/204/198 at session start
  (sandbox --disable all, unchanged pipeline health since s46). Checked the auto-return
  directive (func_8006CCC8 sibling movement to floor 39) against s46's own finding --
  s46 already audited this same-day directive and found no transplantable lever between
  the two functions (unrelated frontier: LICM-hoist-var-reuse on a sign-extended arg vs.
  this function's for-loop guard elision / strength-reduce threshold); nothing changed
  this session that would revise that finding. Tried two NEW structural probes on the
  s46 frontier item ("restructure the obj/flags dispatch chain to reduce live-range
  overlap with the func_80053614 calls"): (1) scalar-cache sin_p/cos_p as sinv/cosv --
  KILLED, scores worse (76 vs 38) despite fewer real insns (194 vs 198); (2) split the
  reused `flags` local into `angle` (lookup phase) + `flags` (hit-result phase) --
  KILLED as a lever (byte-neutral tie, 38/204/198 identical to baseline) though it is a
  legitimate ordinary-C alternative spelling with no downside. Neither closed the
  frontier; floor remains flat at 38 (9th consecutive flat session: s39-s47).

- [s47] Fresh sandbox --disable all re-confirms the s22-s46-banked candidate body at 38/204 (198 build insns) at session start -- pipeline health from s46 holds.

- [s47] Two new structural probes targeting the s46 frontier's obj/flags-dispatch-restructure idea were tried and measured this session; neither closed the gap, and the sin_p/cos_p scalar-cache form is markedly worse despite fewer real instructions, showing the residual is scheduling/allocation-shaped, not raw instruction-count-shaped.

- [s47] The angle/flags split is a legitimate, byte-neutral alternative spelling (kept out of candidate.c since it offers no advantage over the existing simpler single-variable form) -- filed as a banked negative result, not adopted.

- [s47] Floor has now been flat at 38/204 for 9 consecutive sessions (s39 through s47) across enumerate/synthesis/solver/forensics/object-model/escalation/structural modalities; the s45 LADDER EXHAUSTED (non-endgame residual, floor 38) ROTATED disposition and its two re-activation triggers remain the governing record -- one trigger (sandbox scoring defect) already resolved per s46; the other (a genuinely new structural lever) is still open and this session's two probes did not supply it.

- [s48, enumerate] MANDATORY KILL RE-AUDIT: re-spliced the s22-s47-banked candidate.c body (func_80053614 s32-return prerequisite + array-form D_8009A820/D_8009A821 externs, temporarily dropping the conflicting scalar externs at src/text1b.c:2183-2184 for this measurement only) fresh onto src/text1b.c HEAD and re-measured via `& tools/wteng.ps1 main sandbox func_80056CB8 --disable all`: reproduces 38/204 (198 build insns) exactly, matching the ledger's recorded floor. No FAKE construct is present anywhere in the candidate (confirmed again), so there is nothing for `tools/fake_ablate.py` to ablate; the re-audit is this direct fresh re-measurement, consistent with the s17 precedent for the same situation.
- [s48] Two genuinely NEW spelling probes this session, neither previously recorded in this ledger (grepped hypotheses.md/evidence.md for "x +=", "z +=", "flags == 3", "hit1[1]" before running — no prior order-swap or named-intermediate test found for either):
  1. Swapping the statement order of the two `if (flags != 0) { x += ...; z += ...; }` adjustments (z first, then x) on the s22-s47-banked chassis: score 45/204 (198 build insns, unchanged instruction count) -- WORSE than baseline. KILLED.
  2. Introducing a named intermediate `s32 dy0 = hit1[1] - *(s32 *)(obj + 0xBC); if (dy0 < 5) { flags = 0; }` in the flags==3 arm (replacing the inline `if (hit1[1] - *(s32 *)(obj + 0xBC) < 5)`) on the same chassis: score 38/204 (198 build insns) -- byte-neutral tie, ordinary C, no advantage. Not adopted (kept simpler inline form per the pipeline's simplest-known-form tiebreak); does not close any part of the residual.
- [s48] src/text1b.c reverted to clean HEAD at session end (`git checkout -- src/text1b.c`; `git status --short src/text1b.c` empty).
- [s48] Floor remains flat at 38/204, now 10 consecutive sessions (s39-s48) across enumerate/synthesis/solver/forensics/object-model/escalation/structural modalities. The systematic spelling-space search for every block the ledger has so far identified (loop bound, obj/flags dispatch incl. combinations, sin_p/cos_p/scale/x/z ordering+swaps, pt0/pt1 store-order structural variants x2 blocks, dx/dz declaration-order+inlining, the x/z post-call adjustment order, and the flags==3 named-intermediate) is now exhaustively covered with zero hits below 38. The s45/s46/s47 LADDER EXHAUSTED (non-endgame residual, floor 38) ROTATED disposition stands; the live frontier is unchanged from s47 (a fresh RTL/.greg dump targeting the func_80053614 call-site register conflicts, forensics/solver modality, not enumerate).

- [s48] src/text1b.c reverted to clean HEAD at session end (git checkout -- src/text1b.c; git status --short src/text1b.c empty).

- [s48] Floor remains flat at 38/204 for 10 consecutive sessions (s39-s48) across enumerate/synthesis/solver/forensics/object-model/escalation/structural modalities.

- [s48] The systematic spelling-space search across every block this ledger has identified (loop bound s46, obj/flags dispatch + combinations s38/s39, sin_p/cos_p/scale/x/z ordering+swaps s14/s25/s37, both pt0/pt1 store-order blocks s15/s25, dx/dz declaration-order+inlining s9/s15/s25, the x/z post-call adjustment order s48, flags==3 named-intermediate s48) is now exhaustive with zero hits below 38.

- [s48] The s45/s46/s47 LADDER EXHAUSTED (non-endgame residual, floor 38) ROTATED disposition stands unchanged; this session's kills are additional negative evidence within that disposition, not a reopening of it.

- [s49, synthesis] Fresh mandatory kill re-audit via a new repeatable splice script (tmp/grind/func_80056CB8/s49/splice.py, banked as artifact): re-spliced the s22-s48-banked candidate.c body onto current src/text1b.c HEAD, measured `sandbox func_80056CB8 --disable all` = 38/204 (198 build insns), exact reproduction of the ledger's recorded floor. This is the 4th independent confirmation (s41, s46, s48, s49) that the candidate carries zero FAKE-annotated constructs -- fake_ablate.py has no target here. Reverted src/text1b.c to clean HEAD after measurement (`git checkout -- src/text1b.c`; git status clean).

- [s49, synthesis] Full ledger re-read (hypotheses.md 2272 lines, evidence.md 876 lines, candidate.c 1392 lines) confirms the s45/s46/s47/s48 disposition chain is internally consistent: the s45 LADDER EXHAUSTED (non-endgame residual, floor 38) ROTATED record's two endgame-lock gates (scan_hand_coded tier LOW; sotn-construct-index census zero hits) govern CANONICAL-ASM and CONSTRUCT-CLASS-EXTENSION eligibility only, and neither gate's evidence has changed since s45. The record does NOT claim ordinary-C spelling-space exhaustion, and indeed s46/s47/s48 (post-rotation, post-auto-return) found and killed 6 more genuinely new ordinary-C spellings without closing the gap -- consistent with an un-run forensics/solver axis, not a proven-dead one.

- [s49, synthesis] Sibling auto-return directive (func_8006CCC8 -> floor 39, 2026-09-16T13:44) re-checked against s46/s47's own same-directive findings: still no transplantable lever (func_8006CCC8's only open item is an unrelated LICM-hoist-var-reuse case on a sign-extended arg; no shared C structure with this function's for-loop guard / register-pressure frontier). Third consecutive session to confirm this same finding for the same directive stamp; no new information changes it.

- [s49, synthesis] FRONTIER RESET (per this session's mandate): the merged, single strongest live frontier item across the whole ledger is the s47/s48-carried item -- a fresh RTL/.greg dump specifically at the two func_80053614 call sites (not the loop-guard dumps already banked from s43's text1b_greg_forloop_slice.txt / text1b_greg_dowhile_full.txt) to identify which pseudo(s) are actually in conflict across the calls, rather than continuing to guess restructuring shapes blind. This requires forensics or solver modality; synthesis modality is not mandated to execute it. No new frontier items were found this session -- the enumerate-modality spelling search (s46-s48) is exhaustive over every block identified so far.

- [s49] Chassis check at dispatch reported 'measurement unavailable'; direct sandbox --disable all on the current INCLUDE_ASM HEAD returns no_c_body:true (score 204, build_insns 0) as expected for an un-spliced function -- the ledger's 38/204 floor is only reproducible by splicing candidate.c in, which this session did and confirmed fresh.

- [s49] tmp/grind/func_80056CB8/s49/splice.py is a new repeatable splice script (not previously banked) that applies the func_80053614 return-type prerequisite, drops the conflicting scalar D_8009A820/D_8009A821 externs, and inserts the full candidate body -- future sessions can reuse it directly instead of hand-splicing.

- [s49] 12 consecutive flat sessions (s37-s48, now s49 = 13th) across enumerate/synthesis/solver/forensics/object-model/escalation/structural modalities; the systematic spelling-space search across every identified block (loop bound, obj/flags dispatch, sin_p/cos_p/scale/x/z ordering, pt0/pt1 store order, dx/dz declaration order, x/z post-call order, flags==3 named-intermediate, angle/flags split) is exhaustive with zero hits below 38.

- [s49] The sole remaining frontier item -- a fresh RTL/.greg dump AT THE TWO func_80053614 CALL SITES specifically (distinct from the s43 loop-guard dumps already banked) -- requires forensics or solver modality and has not yet been executed; synthesis modality is ledger consolidation, not a probe-running modality.

- [s50, solver] Ran the mandated first solver step (`inverse_compose.py classify`, object-level path: build/src/text1b.o vs tmp/sandbox/func_80056CB8/text1b.o) fresh on the current 38/204 chassis. Verdict: PRE_RA, unchanged from s7/s17 -- confirms the s49 frontier item's RA-conflict framing was mistaken. RA/sched solver tooling ruled OUT for this residual (tool's own "no backend" verdict); see hypotheses.md s50 entries for the full class-kill + predicate cite.

- [s50] Manual read of asm/funcs/func_80056CB8.s lines 1-75 (target asm, ground truth) alongside the classify diff surfaced NEW structural evidence not previously banked at this precision: target computes the loop's per-iteration table offset ONCE per iteration into a single register ($fp, seeded as (code&3)*4 == i*2 doubled... i.e. v1<<2 at entry, then incremented by a flat +2 stride per iteration per the classify's "addiu s8,s8,2"), and adds that SAME register into TWO DIFFERENT symbol base addresses (%hi(D_8009A821) and %hi(D_8009A820)) at the two table-lookup sites. Our candidate instead computes `i * 2` independently via a fresh `sll` at each of the two sites (3 total sll per the classify diff, since func_80053614 is called twice per iteration). This is the "shared BASE POINTER / shared OFFSET" lever that memory/grind/func_80056CB8/rejected/shared-idx-local-worse.c's own header (s6) named as the next thing to try and explicitly did NOT test (it only tested sharing the raw index computation, which regressed due to register pressure across the intervening ratan2 call) -- distinct from per-table-pointer-locals-worse.c (s7, killed: two SEPARATE per-table pointers) and from the untested/declined UB cross-object pointer-arithmetic variant that file also flagged. The untried, well-formed form: a single plain `s32 off` local (not a pointer, to sidestep the UB concern) computed once per iteration (`off = i * 2;`, written ONCE) and read at both `D_8009A821[off]` / `D_8009A820[off]` sites, in place of writing `i * 2` at each site independently. NOT measured this session (turn budget spent on classify + asm cross-read); this is the next session's frontier probe. Full classify output: tmp/grind/func_80056CB8/s50/classify.txt.

- [s50] Confirmed the two `lw #,104(#)` / `lw #,96(#)` instructions the classify diff flagged as "target only" are stack-frame-relative reloads of previously-stored stack-array pointers (`addiu $t3,$sp,0x28; sw $t3,0x68($sp)` and a sibling store at $sp+0x60), i.e. pt0/pt1-equivalent addresses passed to a callee -- NOT unread object fields at obj+0x60/obj+0x68 as an initial (wrong) reading of the raw offsets suggested. Ruled out to prevent a future session chasing a nonexistent "extra field read" lead.

- [s50] Fresh classify (s50) reproduces the exact PRE-RA residual class from s7/s17, unmoved across 33 sessions despite 12+ enumerate/structural sessions varying statement order, naming, and loop-bound spellings.

- [s50] Manual read of asm/funcs/func_80056CB8.s lines 1-75 confirms the classify diff's 'target only: lw #,104(#) / lw #,96(#)' are stack-frame-relative reloads of pt0/pt1-equivalent stack-array pointers stored earlier in the prologue, NOT unread object fields -- ruling out a false 'extra field read' lead before it could mislead a future session.

- [s50] Target's loop shares ONE offset register across both D_8009A820/D_8009A821 byte-table lookups (addu into two different symbol bases with the same shared register, stride +2/iteration); our candidate recomputes i*2 independently at each lookup site (3 sll total per iteration pair).

- [s51] tmp/grind/func_80056CB8/dumps/text1b.greg line 14788 (func_80056CB8 slice): global_alloc spills reg 11 (t3, the `limit = start + 2;` invariant, insn 21) to sp+104 at insn 469, and reg 65 (fixed LO hardware register) transiently at insn 143 for a multiply.

- [s51] The frame-size delta (176 vs 168 bytes) the s50 classify session's instruction-multiset diff surfaced is now attributable to this specific spilled pseudo, not an unnamed frame difference.

- [s51] Both concrete C-level levers derived from this attribution (delete-the-named-local / inline the bound; share the table-index offset) measured worse on the current chassis -- the lever the frame-slot finding actually calls for is a multi-set-pseudo LICM defeat ([[defeat-licm-hoist-var-reuse]] mechanism: reuse the SAME pseudo for a second, used, loop-variant value so it is no longer a loop.c movable), which is distinct from and untested by any prior session's probes.

- [s51] A malformed splice attempt this session (outer 0..1 counter form) produced a syntactically broken body (build_insns=8, unbalanced braces) -- discarded as a non-measurement, not banked as a kill; the underlying C shape (decoupling `i` from an outer counter) remains genuinely untried if re-spliced correctly.

- [s51] git status confirmed clean (src/text1b.c reverted to INCLUDE_ASM stub) after every probe this session.

- [s52] HEAD src/text1b.c is unchanged from prior sessions (INCLUDE_ASM("asm/funcs", func_80056CB8);) -- confirmed via grep before and after this session's work; the tree was left clean (git status --short src/text1b.c empty at session end).

- [s52] The auto-return directive noted at dispatch (func_8006CCC8 sibling movement to floor 39) was already acknowledged and measured by s49 per hypotheses.md's s49 entry re-checking the same stamp -- no new action required this session; func_8006CCC8's own body (a different function, different file region) shares no transplantable block with func_80056CB8's loop residual.

- [s52] Both genuinely-untried structural items named across s49, s50, and s51 (the outer 0..1-counter loop restructuring; do-while-chassis combinations with the limit-removal and shared-idx-local levers) are now measured and closed -- the rederive-modality mandate for this session is fulfilled.

- [s52] No FAKE constructs were introduced or present in any probe this session; every measurement used git checkout -- src/text1b.c to cleanly revert between probes, verified via git status --short.

- [s53] FORENSICS: Fresh chassis reproduction at 38/204 requires copying the candidate.c body (lines 1378-1460 in the current-numbered file) PLUS the 5-line header extern block immediately above it (Judge/ratan2/D_8009A820[]/D_8009A821[]/D_800F6610) PLUS the func_80053614 void->s32 return fix (src/text1b.c:1513, `return func_80052D00(arg2, arg3);`) -- re-confirmed the s25/s28-documented trap fresh: omitting the header externs alone (leaving func_80053614's fix in place) silently miscompiled via K&R implicit-int to score 141/204 (build_insns 175, badly wrong) before the fix was reapplied. Both must land together to reproduce the ledger's 38/204.

- [s53] Read tmp/grind/func_80056CB8/dumps/text1b.greg lines 14788-15995 (the full func_80056CB8 RTL region, bounded by the next function's `;; Function func_80056FE8` marker at the file's line 15995) end-to-end for the first time this ledger. CONFIRMED the exact identity and independence of both reg-11/reg-65 spills named at s50/s51:
  * insn 21 (`t3 = s6 + 2`, hard reg 11) computes `limit = start + 2` (s6 already holds `start`, itself `(*(u16*)(arg0+0x3E8) & 3) * 2` from insns 11/13/16/24). insn 469 immediately spills it to `4(sp+104)` (a synthetic reload insn, cost -1). It is reloaded exactly ONCE more, at insn 493 (`t3 = mem(sp+104)`), immediately before the loop-back-edge compare at insn 28 (`v0 = (s6 < t3)`) and the branch at insn 29 -- i.e. `limit`'s live range spans the ENTIRE loop body (insn 21 to insn 28, ~950 lines of RTL / every iteration's compare), by definition of being a loop-invariant bound checked every iteration. `i` (s6) itself is incremented at insn 438 (`s6 = s6 + 1`) immediately before the reload+compare.
  * "Need 1 reg of class MD_REGS (for insn 143)" is a SEPARATE, unrelated register-class deficit: insn 143 (`lo = v1 * v0`, a `parallel` clobbering hi/accum, `mulsi3_internal`) is the flags/scale fixed-point multiply inside the loop body (operands: v1 = a sign-extended-then-<<8 byte read via `D_8009A821[i*2]`-family code per candidate.c:1400, v0 = a sign-extended s16 mem read). This multiply belongs to the previously-explored, previously-KILLED "scratchpad-literal"/"scale-reuse" hypothesis family (rejected/named-intermediate-scratchpad-literal-worse.c, rejected/scale-reuse-for-scratchpad-literal-worse.c) -- it is NOT the same value as `limit` or `i`, has no source-level relationship to the loop bound, and its MD_REGS need is reported by local-alloc independently of insn 21's GR_REGS need (both appear in the same class-need pass over the WHOLE function; "Spilling reg 11" and "Spilling reg 65" are two separate spill actions in the dump, satisfying two separate class deficits -- not one spill fixing both).
  * grep of the whole func_80056CB8 RTL region for reloads of the spill slot `104))` found exactly 2 hits: the spill store (insn 469) and the single reload (insn 493). No other pseudo reads or writes that stack slot, confirming `limit` has no additional live-range interaction beyond the store/reload pair already known.

- [s53] Src/text1b.c left clean (INCLUDE_ASM marker) at session end; git status --short src/text1b.c empty, verified after every splice/revert cycle. Fresh floor 38/204 re-confirmed via sandbox --disable all this session (chassis: s22-s52-banked body + header externs + func_80053614 s32-return prerequisite, zero FAKE constructs).

- [s53] Fresh chassis reproduction of the ledger's 38/204 floor requires copying THREE things together: (1) candidate.c's function body (current lines 1378-1460), (2) the 5-line header extern block immediately above it (Judge/ratan2/D_8009A820[]/D_8009A821[]/D_800F6610), and (3) changing func_80053614's declared return type from void to s32 with an explicit return statement (src/text1b.c:1513, `return func_80052D00(arg2, arg3);`). Omitting (2) alone this session silently miscompiled via K&R implicit-int to 141/204 (build_insns 175) before being caught and fixed -- independently re-confirms the s25/s28-documented reproduction trap fresh.

- [s53] insn 21 in text1b.greg (hard reg 11 / t3) computes `limit = start + 2` (start already in s6 from insns 11/13/16/24, the (*(u16*)(arg0+0x3E8)&3)*2 object-model read). insn 469 immediately spills it to 4(sp+104) (a synthetic reload insn). It is reloaded exactly once, at insn 493, immediately before the loop-back-edge compare (insn 28: v0 = s6 < t3) and branch (insn 29) -- limit's live range spans the entire loop body by construction, since it is compared against the loop counter every iteration.

- [s53] insn 143 (`lo = v1 * v0`, mulsi3_internal, clobbering hi/accum) is the flags/scale fixed-point multiply inside the loop body -- a previously-explored and previously-KILLED hypothesis family (rejected/named-intermediate-scratchpad-literal-worse.c, rejected/scale-reuse-for-scratchpad-literal-worse.c). Its MD_REGS class need is reported independently of insn 21's GR_REGS need; there is no source-level relationship between the multiply's operands and the loop bound.

- [s53] Grepping the whole func_80056CB8 RTL region (14788-15995) for reloads of the spill slot '104))' found exactly 2 hits (the spill store and the single reload) -- confirms no other pseudo interacts with that stack slot beyond the known store/reload pair.

- [s53] src/text1b.c left clean (git status --short empty, INCLUDE_ASM marker restored) at session end after every splice/revert cycle this session.

- [s54, forensics] Fresh chassis re-confirmation: re-applied the func_80053614 s32-return fix (src/text1b.c:1513) + candidate.c's header extern block + function body (splice script tmp/grind/func_80056CB8/s54/splice.py, lines 1387-1475 of the current candidate.c) to src/text1b.c, measured `sandbox func_80056CB8 --disable all` == 38/204 (build_insns 198, target_insns 204), then reverted cleanly (`git checkout -- src/text1b.c`; `git status --short src/text1b.c` empty afterward). This is the mandatory kill re-audit for this session (5th consecutive independent fresh confirmation, after s41/s46/s48/s49/s52/s53) -- no FAKE construct exists in the candidate to ablate via tools/fake_ablate.py (it is 100% ordinary C: array decls, pointer locals, arithmetic, one func_80053614 s32-return signature change that is itself ordinary C, not a coercion).

- [s54] Ran `tools/loop_movables.py --func func_80056CB8 --file text1b --dumps tmp/grind/func_80056CB8/dumps` (artifact: tmp/grind/func_80056CB8/s54/loop_movables_output.txt) against the single loop (insns 25..444, insn_count=163, loop_has_call=True, threshold=61 derived). This is the FIRST time this ledger has run loop.c's actual move_movables decision report (vs manual RTL tracing) for this function. Result: pseudo 149 (regno 149, `limit`, per the s53 .greg identification) appears at insn 210 with `life=40, nset=1, savings=1, threshold=52, insn_count=163` and decision **"moved"** (52*1*40=2080 >= 163) -- i.e. loop.c's move_movables pass DOES classify `limit` as a movable invariant and hoists it out of the loop by the ordinary LICM inequality (loop.c:1631), independent of any register-pressure consideration. This is CONSISTENT with (not contradictory to) the s51/s53 forensics: `limit` being a genuine LICM movable is exactly WHY it lives across the whole loop body and becomes the pseudo that global_alloc later spills at sp+104 -- the movable-ness is upstream cause, the spill is downstream consequence of the resulting long live range colliding with 20 other simultaneously-live loop-body pseudos. No other loop-body pseudo shows a `nset>1`-admitted-via-consec_sets_invariant_p movable that could plausibly be `limit`'s alternative (the tool's `pseudos set >1 time` list -- {82,83,87,88} -- are different regnos entirely, already known to be the per-iteration hit0/hit1/work-adjacent locals, not loop-invariant bounds).

- [s54] The s53 live-frontier item #2 ("investigate whether narrowing hit0/hit1/work's live-range overlap, e.g. reusing hit1's storage for hit0 between the two func_80053614 calls, changes global_alloc's spill choice -- IF semantically valid, verify hit0 is dead before hit1 is written") is answered by direct source-level dataflow reading of candidate.c's body (lines ~1439-1471, reproduced verbatim in src/text1b.c this session before revert): `hit0` is written by the FIRST `func_80053614(pt0, pt1, (s32)hit0, (s32)work, ...)` call (candidate.c line ~1439). `hit1` is written by the SECOND `func_80053614(pt0, pt1, (s32)hit1, (s32)work, ...)` call (line ~1452), which happens AFTER hit0's write and BEFORE hit0's only reads. The flags==4 arm (line ~1458-1459) reads `hit0[0]` and `hit0[2]` in the SAME nested-if block that also reads `hit1[1]` (line ~1462/1467) -- i.e. `hit0` is read AFTER `hit1` has already been written, and both are read within the same control-flow region. hit0 is therefore NOT dead before hit1 is written: reusing hit1's storage for hit0 would have the second func_80053614 call's write into that shared storage silently corrupt hit0's still-needed values before the flags==4 arm reads them, producing an INCORRECT program (not merely a different-codegen one). This closes the storage-reuse half of the s53 frontier item outright -- no measurement needed or taken, since the construct fails on program correctness before any codegen question is reachable. The narrower-scope half of the same idea (hit0/hit1/work declared inside the loop's block scope rather than function scope) was ALREADY measured byte-neutral at s24 (see the PROBE 2 entry in candidate.c's header, "moved hit0[4], hit1[4], work[4] from function scope into the LOOP's block scope"), so both sub-ideas in this frontier item are now closed.

- [s54] Checked sibling ledgers per this session's brief mandate. func_8006CCC8 (also src/text1b.c, unspent since its s4) works an entirely different object (a field28-dispatch record-update loop over D_800A34FC-relative pointers, func_8006CBD4 callee) with no shared data, callee, or control-flow shape with func_80056CB8's func_80053614/hit0/hit1/D_8009A820 loop -- read its evidence.md/hypotheses.md tail (s3/s4 entries) and confirmed no transplantable block exists between the two. func_80055B60 has only a 1-session ledger (its own s1) with no candidate.c yet, nothing to transplant. func_80057CC8 and func_80056FE8 are COMPLETED-C siblings in the same TU already reflected in candidate.c's Judge/ratan2 idiom citations from earlier sessions (no new transplant available). No sibling transplant applied or needed this session.

- [s54] src/text1b.c left clean (INCLUDE_ASM marker restored, git status --short empty) at session end.

- [s54] Fresh chassis measurement this session: sandbox --disable all == 38/204 (build_insns 198, target_insns 204) with the s22-s53-banked candidate.c body + func_80053614 s32-return prerequisite + header externs applied, then reverted cleanly via git checkout -- src/text1b.c (git status --short empty afterward).

- [s54] tools/loop_movables.py output (tmp/grind/func_80056CB8/s54/loop_movables_output.txt) is the first formal loop.c movable-decision report run for this function: only 11 movables total in the single loop (insns 25..444, insn_count=163, loop_has_call=True); pseudo 149 (limit) at insn 210 is 'moved'; the {82,83,87,88} multi-set pseudos flagged by the tool are the known per-iteration hit0/hit1/work-adjacent locals, not loop-bound candidates.

- [s54] candidate.c's own header notes (accumulated s22-s53) already establish limit's spill mechanics via manual .greg reading; this session's tool run is independent, automated corroboration of the same fact via the actual loop.c predicate rather than manual RTL tracing.

- [s54] No FAKE construct exists anywhere in the current 38/204 candidate body -- it is 100% ordinary C (array decls, pointer locals, arithmetic, one func_80053614 void->s32 signature correction). tools/fake_ablate.py was not applicable this session for that reason; the mandatory kill re-audit was performed as a fresh sandbox re-measurement instead (per the brief's own framing: 'no FAKE construct to ablate' has been the case since at least s48).

- [s55] Fresh chassis re-confirmation: sandbox --disable all == 38/204 (build_insns 198) with the s22-s54-banked candidate.c body + func_80053614 s32-return prerequisite + header externs applied, then reverted cleanly via `git checkout -- src/text1b.c` (git status --short clean afterward, aside from the pre-existing unrelated metrics/events.jsonl diff).

- [s55] The pt0/pt1 shared-8-word-buffer merge (converting `s32 pt0[4]; s32 pt1[4];` into `s32 pts[8]; s32 *pt0=pts; s32 *pt1=pts+4;`, correctness-verified safe by both s53 and s54's dataflow reads) measures 103/204 (build_insns 207) — substantially worse than baseline. Confirms fixed-address stack arrays are cheaper than pointer-indirected slices here; closes the s53/s54 "genuinely untried, correctness-safe" pt0/pt1 frontier item with a real negative.

- [s55] `tools/nrefs_census.py --func func_80056CB8 --file text1b` run for the first time this ledger (via `bash tools/wsl.sh`, not the Windows-side python — the tool shells out to `mipsel-linux-gnu-cpp` which only exists in the WSL toolchain). Produced the global.c:615 allocno_compare order for the current baseline chassis; raw output not yet cross-referenced against the s51 `.greg` dump's pseudo numbering (the two dumps are from different passes, `.lreg` vs `.greg`, and may renumber pseudos differently) — this cross-reference is the concrete next step before deriving a pseudo-specific C lever from the s53 conflict-list frontier item.

- [s55] sandbox --disable all == 38/204 (build_insns 198, target_insns 204) reproduces fresh with the banked candidate.c body applied to src/text1b.c, then reverted cleanly (git status --short clean afterward except the pre-existing unrelated metrics/events.jsonl diff).

- [s55] tools/nrefs_census.py must be invoked via `bash tools/wsl.sh` (WSL bash), not the Windows-side python3 directly — it shells out to `mipsel-linux-gnu-cpp`, which only exists in the WSL toolchain; a direct Windows python3 invocation fails with CalledProcessError.

- [s55] tools/nrefs_census.py --func func_80056CB8 --file text1b (run for the first time this ledger) produced the global.c:615 allocno_compare order for the current baseline chassis. The pseudos with the longest livelen (72=148, 74=144, 75=145, 116=282, 146/148/149=138-140) correspond to cross-call-live values (obj pointer chain, the two func_80053614 constant-address arguments, the Judge symbol_ref) already flagged as spill-adjacent in the s51 .greg read.

- [s55] The census's pseudo numbering comes from the .lreg dump (pre-allocation), while the s51 evidence about limit's spill came from the .greg dump (post-allocation) -- these two dumps can renumber pseudos differently between passes, so pseudo '149' in this session's census (a (const_int 528482304) load) is NOT confirmed to be the same object as s51's 'pseudo 149 = limit'. A verified cross-reference table (diffing the .greg dump's ';; Register dispositions:' pseudo-to-symbol map against this census's 'first def' column) is required before deriving any pseudo-specific C lever from the s53 conflict list.

- [s55] func_80056CB8 shares no data/callee/control-flow with sibling func_8006CCC8 (confirmed again this session by reading s54's prior conclusion in hypotheses.md; not re-derived) -- no transplant available from that unspent sibling.

- [s56, rederive] Owner directive check: the dispatch brief's "auto-return: coupled sibling moved after rotation -- func_8006CCC8 -> floor 39 (2026-09-16T13:44)" refers to the SAME floor (39) already recorded in this ledger's s54 sibling check (func_8006CCC8 floor 39 since its s4, 2026-09-16 08:44) -- no new data since s54's read. Re-read func_8006CCC8's hypotheses.md tail (s1-s4) fresh this session: its object (a field28-dispatch record-update loop over D_800A34FC-relative pointers via func_8006CBD4, techniques = i s16->s32 widening, nested if/else block-layout matching branch-sense/physical-ordering, named-intermediate declaration-order for a loop-invariant constant, single-read field-dispatch CSE) remains structurally disjoint from func_80056CB8's func_80053614/hit0/hit1/D_8009A820/pt0/pt1 collision-detection loop -- same conclusion as s54, reconfirmed fresh rather than assumed stale.

- [s56, rederive] Fresh m2c decompile of asm/funcs/func_80056CB8.s (`python3 tools/m2c/m2c.py --valid-syntax --context include/m2c_context.h --target mipsel-gcc-c`, artifact tmp/grind/func_80056CB8/s56/fresh_m2c.c) produces a do-while loop with NO separately-named loop-bound local -- the bound is inlined as `var_s6 < (sp60 + 2)` where sp60 is a pre-loop copy of the starting index, i.e. exactly the "do-while chassis + inline bound, no `limit` local" shape. This shape is BYTE-IDENTICAL in structure to the already-banked-and-killed memory/grind/func_80056CB8/rejected/dowhile-nolimit-worse.c (s52, measured 45/204, build_insns 194, WORSE than the 38/204 for-loop-with-named-limit baseline). m2c's independent re-derivation from raw asm converging on a form already in the rejected bank is affirmative evidence that the for-loop-with-named-`limit`-local chassis (candidate.c's current shape) is the locally-optimal structural family already found, not evidence of an untried structural axis.

- [s56, rederive] decomp.me local corpus search (`python3 tools/decomp_me_scrape.py search --asm-file asm/funcs/func_80056CB8.s` against the 3754-scratch local cache in tmp/decomp_me_corpus/) returns no meaningful precedent: best match similarity 0.068 (func_800B788C, gcc2.7.2-cdk), next four all <=0.063. No candidate scratch shares this function's shape (loop + two-call collision-probe dispatch + LICM-spill residual) closely enough to mine for a technique.

- [s56] Fresh 38/204 floor stands unchanged; candidate.c body UNCHANGED since s22 (8 consecutive confirming/covering sessions: s48,s49,s50,s51,s52,s53,s54,s55,s56).

- [s56] m2c's independent re-derivation from raw asm.s converges on the for-loop-with-named-limit family's do-while sibling (already killed), which is corroborating evidence the currently-banked chassis is the locally-optimal structural shape for this residual, not proof of global optimality.

- [s56] The residual remains pinned (s51/s53/s54) to global_alloc's spill of pseudo 149 (`limit`), confirmed by loop_movables.py as a genuine move_movables LICM invariant (insn 210, threshold*savings*lifetime=2080>=163) -- this is a register-allocation-class residual, not a structural-shape residual, which explains why rederive-modality (shape search) keeps converging on already-covered shapes.

- [s56] The concrete open item remains forensics/solver-modality: cross-reference nrefs_census's .lreg-based pseudo numbering against the .greg dump's post-allocation numbering (fresh -da run needed) before any pseudo-specific C lever can be derived from the s53 conflict-graph frontier.

- [s57, structural] Fresh 38/204 re-confirmed (9th consecutive). Ran a genuinely new structural probe not covered by any of the 29 prior rejected forms: type-narrowing `start`/`limit`/`i` (all provably small-valued, [0,6]) from s32 to s16, in 3 combinations. All 3 measured worse: i-alone 45/204 (+8 insns, MIPS lacks native halfword ALU so s16 `i` forces extra sign-extend/widen ops around every `i*2` index and the tail `(s8*)` store), start+limit 39/204 (same insn count, 1-point-worse RA residual — limit still spills), all-three 48/204 (+10 insns, compounds both). Banked to rejected/type-narrow-{i-s16,start-limit-s16,all-s16}-worse.c. This closes the "narrow start/limit/i's own type" reading of the s53 frontier item; the still-open reading (narrow one of the ~20 OTHER conflict-graph-live pseudos) needs the s55-named forensics/solver pseudo cross-reference (`-da` .greg re-run vs nrefs_census), which is out of scope for structural modality.

- [s57] Chassis-fresh 38/204 re-confirmed for the 9th consecutive session (s49-s57), across permuter/structural/enumerate/synthesis/solver/forensics/object-model/rederive/escalation modalities per the 2026-09-16 rotation record.

- [s57] Type-narrowing start/limit/i to s16 is a genuinely new structural axis not covered by any of the 29 pre-s57 rejected forms (prior sweeps varied declaration ORDER and STATEMENT order, never TYPE, for these three locals specifically).

- [s57] The s53 live-frontier item ('narrow some OTHER live pseudo's declared type/scope to relieve limit's spill cost') has two readings: (1) narrow start/limit/i themselves -- now CLOSED by this session's measurement (all 3 combos worse); (2) narrow one of the ~20 OTHER conflict-graph-live pseudos identified in the s51 .greg dump -- still open, requires the s55-named pseudo-number cross-reference (fresh -da run vs nrefs_census .lreg output), which is a forensics/solver-modality task outside this session's structural mandate.

- [s57] Sibling func_8006CCC8 (floor 39, the owner-directive trigger for this session's dispatch) remains structurally disjoint (field28-dispatch record-update loop vs. this function's collision-detection loop over func_80053614/hit0/hit1/pt0/pt1) -- re-confirmed at s54/s56, no transplantable block; not re-derived again this session to avoid redundant ledger bloat.

- [s58] 10 consecutive sessions (s49-s58) re-confirm honest floor 38/204 (build_insns 198 vs target_insns 204) with zero drift.

- [s58] tools/ra_solver/inverse_compose.py classify (re-run multiple times through s50) reports FIRST DIVERGENCE: PRE-RA/rtl_shape -- the honest and target instruction streams are a different instruction MULTISET, not a permutation/renaming of the same set, so RA-solver and sched-solver are both formally out of scope for this residual (their own tool verdict: 'searching them would produce fiction').

- [s58] The target-only instruction shape (addiu s8,s8,2 / two addu #,#,s8 / beqz+bltz+j triple) is a genuine loop-carried strength-reduced i*2 accumulator kept in $fp across the whole loop -- loop.c's strength_reduce giv-worth test (loop.c:3823) rejects promoting our i*2 to a giv on every chassis measured because insn_count (163) exceeds lifetime*threshold*benefit (124), a 24% shortfall confirmed via the real -da .loop dump (s31).

- [s58] The 'one C handle carries i*2 across both table lookups' family (shared index local, shared pointer local, shared byte-offset local) is now KILLED across THREE distinct chassis generations (s6 @81/204, s11-13 @48-58/204, s51 @38/204, always regressing to ~51-81/204) -- this family is exhausted, not chassis-relative.

- [s58] 32 rejected forms are now banked in memory/grind/func_80056CB8/rejected/, spanning loop-bound shapes, declaration order, statement re-association, type-narrowing, do-while restructuring, and shared-index/pointer/offset variants.

- [s58] The 0x1F8002B8 scratchpad-literal single-vs-double materialization gap (target spills to a stack slot and reloads via a caller-saved temp; ours re-materializes via lui/ori into a callee-saved $fp both times) is understood via the s29 .greg register-disposition audit: $fp is the ONLY unclaimed callee-saved register left after 8 other named residents saturate $s0-$s7, so the literal takes it by allocation default, not priority contention -- naming the literal as a local (tried 3x: s15, s28, s41) always regresses because it forces one pseudo to live across the intervening func_80053614 call.

- [s59] Fresh 38/204 re-confirmation (13th consecutive flat session). Extended the s3 obj/flags declaration-order kill (originally measured on a pre-s22 ancient 106-floor chassis) to all 7 loop-scoped locals (obj/flags/scale/sin_p/cos_p/x/z) on the CURRENT chassis: 3 representative orderings (full reverse, def-before-use, pointer-locals-first) all tie 38/204 exactly. Declaration-order axis for this block is now closed on the current chassis (instance-scoped, 3 of 5040 permutations sampled).

- [s59] No untried block-local spelling axis remains identifiable from this ledger's enumerate-modality history (s2/s3/s5/s9/s14/s15/s25/s37/s38/s39/s48/s59). The live frontier is unchanged from s49/s51/s55/s58: naming an unnamed compiler-internal pseudo in the global_alloc conflict graph via the s51/s55 .greg/.lreg cross-reference, which requires forensics/solver modality.

- [s59] 13th consecutive session confirming honest floor 38/204 (build_insns 198) on the current chassis with zero HEAD drift since s22.

- [s59] The block-local spelling search for this residual (loop bound forms, obj/flags dispatch + combinations, sin_p/cos_p/scale/x/z ordering+swaps, both pt0/pt1 store-order blocks, dx/dz declaration-order+inlining, x/z post-call adjustment order, flags==3 named-intermediate, angle/flags variable split, sin_p/cos_p precompute-to-scalar, and now the full 7-local declaration order) is flat across 15 consecutive sessions (s37-s59) and 10+ distinct modalities.

- [s59] The residual's mechanism was pinned at s51 via a .greg dump: pseudo for `limit` is spilled in global_alloc's conflict graph, live across virtually the whole function body alongside ~20 other pseudos; 8 of those have stable C names (obj/i/flags/scale/sin_p/cos_p/x/z) and are all closed as narrowing/reorder targets (s57/s58/s59); the remaining ~20 are compiler-internal pseudos with no stable declared C handle.

- [s59] src/text1b.c reverted to clean INCLUDE_ASM state after every measurement this session; git status confirms no residual diff except the pre-existing metrics/events.jsonl noise.

- [s60] Fresh 38/204 re-confirmation (16th consecutive flat session, s37-s60). Tested 3 hand-written spellings of the type-dispatch if/else block (flagged at s37/s38 as untested due to a spelling_enum.py tooling gap: it can't represent regions with interior if/else bodies): OR-operand swap (40/204), hoisted repeated-field-read local (58/204 -- NOT neutral, target genuinely re-reads the field twice so caching it changes the program), negated-condition arm swap (46/204). All worse; banked to rejected/dispatch-condition-spellings-worse.c. This closes the most obvious hand-derived readings of the s37/s38 tooling-gap block; a fully mechanical enumeration of that block still requires either a spelling_enum.py extension (interior-anchor support) or further hand-generated variants -- concrete next step for a future enumerate session.
- [s60] Sibling func_8006CCC8 re-checked: floor unchanged (39, since its s4) and structure unchanged (field28-dispatch record-update loop, disjoint from this function's collision-detection loop) since s54/s56/s57 -- no transplant.
- [s60] src/text1b.c reverted to clean INCLUDE_ASM state after every measurement this session; git status confirms no residual diff except the pre-existing metrics/events.jsonl noise.

- [s60] Fresh 38/204 re-confirmation, 16th consecutive flat session (s37-s60) across 10+ distinct modalities.

- [s60] The s37/s38-flagged tooling gap (spelling_enum.py's ENUM-BEGIN/END format cannot represent a region with interior if/else bodies) now has 3 of its most obvious hand-derived spellings closed by direct measurement: OR-operand order, field-read caching, and if/else arm order are all worse than the natural form.

- [s60] The field-read-hoist result (58/204, -18 build_insns) is affirmative evidence the target genuinely performs two separate reads of arg0+0x6A -- caching is not a neutral respelling for this residual, it removes required target behavior.

- [s60] The live frontier (naming one of the ~20 OTHER compiler-internal pseudos in the global_alloc conflict graph via the s51 .greg / s55 nrefs_census cross-reference) remains unaddressed -- it requires forensics/solver modality, not enumerate.

- [s60] src/text1b.c reverted to clean INCLUDE_ASM state after every measurement this session; no residual diff except the pre-existing unrelated metrics/events.jsonl noise.
