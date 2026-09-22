# Hypothesis ledger — func_8002C22C (PutRobShadow)

## H1 — object model: D_80102314 is record[1] of the 2-elem practice-menu table
(D_80101EC8 = record[0], stride 0x44C, per func_8002C61C's own `s1 + 0x44C`
pattern in the same TU). **CONFIRMED.** Static asm evidence: the symbol is
materialized ONCE via its own `lui %hi/addiu %lo(D_80102314)` (a direct symbol
ref, not arithmetic from D_80101EC8), then every field is read via
`lw $rX, OFF($t1)` for OFF in {0x210..0x224}/{0x234..0x248} — genuine
base+offset addressing with zero individually-named symbols at those
addresses (grep of `undefined_syms_auto.txt` confirms). Declared
`extern s32 D_80102314;` and used as `s32 *d_tbl = &D_80102314; d_tbl[OFF/4]`
— this exactly reproduces the observed addressing form. Declared FILE-LOCAL in
`src/code6cac_b.c` this session (not the header — out of this function's
`scope_allow.txt` grant; see frontier #3).

## H2 — record-0 vec3 fields (D_801020D8.. / D_801020FC..) need NO aggregate
merge; the existing individual `extern s32` scalars are already the correct
object model. **CONFIRMED.** Every one of the twelve fields is loaded via its
OWN `%hi/%lo` relocation in the asm (never base+offset from a shared struct
pointer), which is exactly what discrete scalar externs compile to. The
census's "vec3 A / record 0" language describes game-state semantics, not a
missing C aggregate — do NOT attempt a `split-scalars-hide-aggregate`-style
merge here; there is no base-register indirection to fix (contrast with
D_80102314 in H1, which DOES need base+offset addressing and got it).

## H3 — plain (non-volatile) casts on the scratchpad addresses (0x1F800000-
0x1F800074, 0x1F800360-0x1F800378) reproduce the target's control-flow-local
scratchpad accesses, since scratchpad is explicitly EXCLUDED from the
hardware-MMIO volatile carve-out (`.claude/rules/mmio-volatile-type-level.md`)
and this project has no IRQ-writer evidence to support the game-state
two-prong gate for these specific addresses (they appear to be a private
scratch buffer for this one call chain, not an interrupt-touched global).
**KILLED — instance.** Measured this session (s1): translating the archived
C shape 1:1 with every `volatile` cast stripped to a plain cast scores
`sandbox --disable all` = 211/252 (build 230 insns vs target 252) — a large
improvement over the 252-insn no-C-body floor, confirming the scratchpad
region itself does NOT need volatile to reach the right BALLPARK of codegen.
BUT `--diff` shows 27 source-level hunks, several of which show our build's
early hunks (hunks 3-5) carrying many more instructions clustered right after
the first branch than target has at the equivalent point — consistent with
(but not yet PROVEN as) loads/stores our cc1 treats as branch-independent
being scheduled/materialized earlier than target's strictly-arm-local order.
This measurement used the archived-body C 1:1 translated to plain casts, on
the current toolchain (`-mel -msoft-float`), zero FAKE constructs, zero pins.
`kill_scope: instance` — this exact spelling (plain casts, no other
scheduling-affecting restructuring) does not reach distance 0; it is NOT a
class claim that scratchpad can never be typed without volatile — the
divergence has NOT yet been pass-attributed (dumps were generated
`tmp/grind/func_8002C22C/dumps/code6cac_b.*` this session but not read in
detail — next session should grep the `.cse`/`.loop`/`.sched` dumps for the
func_8002C22C RTL block before proposing a specific lever, per the brief's
PASS ATTRIBUTION mandate). It is equally possible the apparent "hoist" is a
diff-alignment artifact of the 22-instruction count mismatch rather than a
real motion — this needs the dump read to settle before any volatile /
restructuring lever is attempted.

## H4 — the s1 211-residual's mechanism is CSE1's `cse_end_of_basic_block` block-extension
(`.claude/rules/cse-block-extension-controls-fold-span.md`), NOT loop.c LICM as H3 speculated.
**CONFIRMED (s2).** `.loop` dump for func_8002C22C (lines 6365-7445) carries zero
invariant/hoist/giv notes — the function has no loops at all (two straight-line if/else
pairs, no back-edges), so LICM is mechanically inapplicable. The `.cse` dump instead shows
each scratchpad zero-store address materialized into its own fresh pseudo at the top of the
function, and the SAME constant's pseudo is never re-materialized for later same-address
stores inside either if/else arm (`grep` for the CONST_INT values 528483168/172/176/184/188/192
across the whole function body in `.cse` finds most of them exactly once despite 3+ C-level
writes to each address) — i.e. cse1 is treating the pre-branch block and both branch arms as
one continuous region and forwarding the address computation across the join, exactly the
mechanism the rule file documents (`;; Processing block from 2 to 239` spans past the
conditional branch). Target's asm never does this (confirmed via `--diff`: target
re-materializes `lui at,0x1f80` fresh before EVERY single scratchpad store, never reusing a
base register across two different store statements).
- mechanism: `cse.c:8102-8184` `cse_end_of_basic_block` — a conditional branch whose jump-target
  label has `LABEL_NUSES==1` does not end cse1's basic block; it is followed, so quantities
  (including simple constant-address pseudos) computed before the branch stay "known" while
  processing code that only runs on one of the two paths.
- probe: read `.loop`/`.cse` dumps for the func_8002C22C RTL block (grep `;; Function
  func_8002C22C` for block bounds, grep the scratchpad CONST_INT values for recurrence count)
- result: `.loop` dump empty for this function (no loop insns at all); `.cse` dump shows the
  address-pseudo-sharing pattern the rule predicts
- verdict: CONFIRMED

## H5 — duplicating the six unconditional pre-branch scratchpad zero-init stores into BOTH
if/else arms (instead of leaving them shared, above the branch) measurably shrinks the
CSE-shared span and lowers the honest floor. **CONFIRMED (s2), partial — does not reach 0.**
- mechanism: same cse1 block-extension (H4); duplicating the statement into each arm doesn't by
  itself break the extension test (LABEL_NUSES is unchanged), but it does make each arm's
  zero-init the FIRST reference to that address within the arm's own straight-line run, which
  changes which stores end up sharing a pseudo with which.
- probe: moved `*(s32*)0x1F80036x = 0;` (x in 0,4,8,C,10,14 offsets from base 0x1F800360) from
  before `if (D_800A3824 & 1)` to the start of both the `if` and `else` bodies (verbatim
  duplicate, semantically identical since both arms always ran it before); measured
  `sandbox --disable all`
- result: 211 -> 199 (`build_insns` 230 -> 246, target 252); `--diff` afterward: 36 hunks (32
  source-level / 4 operand-only / 0 not-scored) — hunk 1 shows the SAME merge pattern now
  operating intra-arm (four `lui/ori` address pairs still batched before four stores within one
  arm) rather than cross-arm, i.e. the mechanism is unchanged, only its span shrank
- verdict: CONFIRMED
- kill_scope: N/A (this is a positive/improving result, not a kill)

## H6 — wrapping one arm's zero-init block in `do { ... } while (0);` (FAKE-annotated,
attempting to trip one of `cse_end_of_basic_block`'s three backward-scan escapes via an
inserted loop-note) closes more of the H4 residual than plain per-arm duplication (H5).
**KILLED — instance.** Measured this session (s2): applying the wrap on TOP of the H5 form
(if-arm only) scores `sandbox --disable all` = 200 — WORSE than H5's 199, with `build_insns`
unchanged at 246 (same instruction count, a worse masked operand arrangement). Reverted
immediately; saved to `memory/grind/func_8002C22C/rejected/dowhile0-zero-init-single-arm.c`.
- mechanism: `do-while-zero-exception` (owner ruling 2026-07-06) — sanctioned for ANY codegen
  effect including register allocation; hypothesized here to interact with cse.c's backward-scan
  loop-note break (`cse.c:8109-8114`, breaks on `NOTE_INSN_LOOP_END`)
- probe: wrapped the if-arm's six zero-init stores in `do{...}while(0);` with a `/* FAKE */`
  annotation naming the mechanism; measured `sandbox --disable all` before/after on the H5
  chassis
- result: 199 -> 200 (build_insns unchanged, 246); no improvement, net negative
- verdict: KILLED
- kill_scope: instance — this exact spelling (do-while(0) wrap on ONE arm's zero-init, on the
  H5 chassis, zero other constructs) does not help; it is NOT a claim that do-while(0) wraps can
  never help this residual in a different placement (e.g. wrapping a different sub-range, or
  both arms, or the whole arm body) — those are untried, not disproven.
- measured_on: s2 candidate (H5 chassis: per-arm-duplicated zero-init, D_80102314 base+offset
  decl), current toolchain (-mel -msoft-float), one FAKE construct (the do-while wrap itself,
  reverted), zero pins

## H7 (standing precedent, not newly derived this session) — volatile on the
0x1F800000-0x1F8003FF scratchpad range is a Judge-FAILed, project-BANNED construct for this
exact address range, independent of which function uses it.
**Evidence found (s2):** `src/code6cac.c:237-238` (func_80017FA0, a neighboring function in the
same cluster, now COMPLETED-C) carries a doc comment: *"This supersedes the s4 volatile form
(Judge FAIL 2026-08-20 02:54, construct BANNED: volatile on scratchpad 0x1F800000-0x1F8003FF)"*
— that function's true closing form used a goto/do-while loop restructuring (a loop.c-class fix),
NOT volatile. This directly answers s1 frontier item #2 (search for a sibling using the same
scratchpad range): the sibling exists, and its answer is "volatile was tried and Judge-FAILed",
reinforcing H3's KILLED status and `.claude/rules/mmio-volatile-type-level.md`'s explicit
scratchpad exclusion + `.claude/rules/legitimate-volatile-interrupt-touched.md`'s unmet two-prong
gate (no IRQ writer identified for these addresses). Any future session should NOT re-attempt a
volatile respelling on this address range without a genuinely NEW IRQ-writer citation — the
Judge has already ruled on this exact range once.

## Frontier (next session, structural or rederive modality)

1. **Keep pushing the CSE block-extension lever (H4/H5).** The 199-residual's hunk 1 shows the
   SAME intra-arm address-sharing pattern that H5 shrank from cross-arm to intra-arm — the next
   reduction is probably ANOTHER structural change that shrinks the span further within one arm,
   not a wholesale new mechanism. Candidates worth measuring (none tried yet): (a) interleaving
   each zero-init store immediately before its OWN first later-use in program order (rather than
   grouping all six zero-inits together at the arm's top) so each address's zero-store and
   overwrite-store are adjacent, denying cse1 a wide window to share the pseudo; (b) applying the
   SAME per-arm-duplication idea to the trailing unconditional `0x1F800368/374/378` stores after
   the first if/else (lines 976-980 of the s2 candidate), which are currently NOT duplicated into
   anything since they sit after the join — check whether restructuring them into the if/else
   arms themselves (they use branch-set variables a2/d_v1/d_a0/d_v0/d_a1 so this is a legitimate
   real restructuring, not a coercion) closes more of the residual; (c) the SECOND if/else block
   (0xA8/0xAC/0xB0 accumulate region, lines 982-1008) has NOT been examined via `--diff` in detail
   this session — read hunks past #14 (not yet reviewed) to see whether the SAME cse1 mechanism
   or a DIFFERENT one (accumulation via `+=` may hit combine.c fold behavior instead) governs that
   region.
2. Frontier item retired (s1 #2 = the sibling-precedent grep) — DONE, answered by H7: volatile is
   Judge-banned on this range, do not retry without new IRQ evidence.
3. **Promote `D_80102314` to `include/code6cac.h`** once a scope-grant session authorizes
   touching that header for this function — unchanged from s1, still out of this session's
   `scope_allow.txt` grant.

## Frontier (next session, recon or rederive modality)

1. **Pass-attribute the remaining 211-distance residual.** Dumps already
   generated (`tmp/grind/func_8002C22C/dumps/code6cac_b.{cse,cse2,loop,sched,
   sched2,flow}`, this session, `pwsh tools/grinder/dump.ps1 func_8002C22C`).
   Grep each for the func_8002C22C block (search `PutRobShadow`-adjacent
   symbol names or the 0x1F800360 literal) and confirm whether the early
   hunks are genuine motion (which pass) or an alignment artifact of the
   230-vs-252 instruction count. This determines whether H3's residual is a
   scheduling/CSE lever (ordinary C restructuring) or something needing a
   volatile-grant argument.
2. **Check for other matched/canonical BB2 functions touching the SAME
   scratchpad addresses** (0x1F800000-0x1F800074, 0x1F800360-0x1F800378) —
   a sibling that already resolved this exact address range would settle
   whether volatile was needed there and is strong precedent either way.
   `grep -rn "0x1F80036\|0x1F80037\|0x1F80000\|0x1F80005\|0x1F80006\|0x1F80007" src/*.c`
   was NOT run yet this session.
3. **Promote `D_80102314` to `include/code6cac.h`** once a scope-grant
   session (Judge ESCALATE `integration-handoff` / driver
   `add-scope-allow`) authorizes touching that header for this function —
   currently declared file-local in `src/code6cac_b.c` to stay in-scope.

## Candidate state

`memory/grind/func_8002C22C/candidate.c` (s1) = translated archived-body
shape, plain casts, `D_80102314` scalar decl, measured 211/252
(`sandbox --disable all`). Also live-staged in `src/code6cac_b.c` for the
next session to resume from (permitted — the src file is this function's own
scope surface). NOT candidate-ready: distance is 211, not 0.

## [s1] D_80102314 is record[1] of the same 2-element practice-menu table whose record[0] base is D_80101EC8 (stride 0x44C, corroborated by func_8002C61C's own `s1 + 0x44C` pattern in the same TU); declaring it as a scalar extern and indexing base+offset (`s32 *d_tbl = &D_80102314; d_tbl[OFF/4]`) reproduces the asm's own `lui/addiu D_80102314` direct-symbol-then-offset addressing.
- mechanism: static asm evidence: the symbol is materialized ONCE via its own lui %hi/addiu %lo relocation (not arithmetic from D_80101EC8), then every field is read via lw $rX,OFF($t1) with zero individually-named symbols at those addresses (grep of undefined_syms_auto.txt)
- probe: declared extern s32 D_80102314 file-local in src/code6cac_b.c, used as s32*d_tbl=&D_80102314; d_tbl[OFF/4]; measured via sandbox --disable all
- result: sandbox --disable all = 211 (from 252 no-C-body floor); --diff shows zero hunks attributable to mis-resolution of this symbol
- verdict: CONFIRMED

## [s1] The twelve record-0 vec3 fields (D_801020D8/DC/E0/E4/E8/EC and D_801020FC/80102100/04/08/0C/10) need no aggregate/struct merge; the existing individual `extern s32` scalar declarations already reproduce the target's addressing.
- mechanism: each field is loaded via its own lui %hi/lw %lo relocation pair in asm/funcs/func_8002C22C.s (lines 44-63, 77-108), never via a base register + offset from a shared struct pointer -- exactly what discrete scalar externs compile to, so no split-scalars-hide-aggregate condition applies
- probe: s1 candidate uses the plain scalar extern s32 decls as-is for all twelve fields; measured sandbox --disable all and --diff
- result: sandbox --disable all = 211; --diff shows no hunk attributable to symbol-address mismatch for these twelve fields (all divergent hunks are scheduling/hoist of scratchpad loads around the two branches, not object-model mismatch)
- verdict: CONFIRMED

## [s1] Plain (non-volatile) casts on the scratchpad addresses (0x1F800000-0x1F800074, 0x1F800360-0x1F800378) reproduce the target's control-flow-local scratchpad accesses without needing volatile, since scratchpad is explicitly excluded from the hardware-MMIO volatile carve-out and this project has no IRQ-writer evidence for the game-state two-prong gate on these addresses.
- mechanism: GCC 2.7.2's alias analysis on raw-address pointer casts (non-volatile) may prove no-aliasing across the branch and hoist/reorder loads across it, whereas the target keeps every scratchpad load/store strictly inside its owning if/else arm in source order
- probe: translated the archived pre-include-asm-body.c C shape 1:1 with every volatile cast stripped to a plain cast; measured sandbox --disable all and inspected --diff hunk classes
- result: sandbox --disable all = 211/252 (build 230 insns) -- a large improvement over the 252-insn no-C-body floor, so the scratchpad region itself does NOT need volatile to reach this ballpark. But --diff shows 33 hunks (27 source-level / 6 operand-only / 0 not-scored); the first source-level hunks (1-5) show our build clustering more instructions right after the first branch than target has at the equivalent point, consistent with (but not proven as) cc1 treating the plain casts as branch-independent and scheduling/materializing loads earlier than target's strictly-arm-local order. This exact spelling (plain casts, no other scheduling restructuring) does not reach distance 0.
- verdict: KILLED
- kill_scope: instance
- measured_on: s1 candidate (archived-body C shape translated 1:1 to plain casts, D_80102314 declared+used base+offset), current toolchain (-mel -msoft-float), zero FAKE constructs, zero pins

## [s2] The s1 211-instruction residual is caused by GCC 2.7.2's loop.c LICM hoisting scratchpad loads out of the two if/else arms.
- mechanism: loop.c loop-invariant code motion
- probe: Read tmp/grind/func_8002C22C/dumps/code6cac_b.loop for the func_8002C22C RTL block (lines 6365-7445) for invariant/hoist/giv notes.
- result: The .loop dump for this function's block contains zero invariant/hoist/giv notes. The function has no loops at all (two straight-line if/else pairs, zero back-edges), so LICM is mechanically inapplicable here. This overturns the s1 H3 mechanism guess.
- verdict: KILLED
- kill_scope: class
- measured_on: s2 candidate (per-arm-duplicated zero-init), current toolchain (-mel -msoft-float), zero FAKE constructs
- predicate_cite: tools/gcc-2.7.2/loop.c (LICM only operates within LOOP_BEG/LOOP_END regions; func_8002C22C's .loop dump at tmp/grind/func_8002C22C/dumps/code6cac_b.loop:6365-7445 has none, proving loop.c has no region to act on in this function)

## [s2] The s1 residual's real mechanism is cse1's cse_end_of_basic_block extending its processing region across the conditional branch (LABEL_NUSES==1 on the else-arm entry label), so redundant scratchpad-address pseudo computations forward across the if/else join, whereas target recomputes lui/ori for every store independently.
- mechanism: cse.c:8102-8184 cse_end_of_basic_block block-extension test
- probe: Read .cse dump (lines 6352-7439) for the address CONST_INTs of the six zero-init scratchpad stores (528483168/172/176/184/188/192); count occurrences across the whole function body; cross-check against --diff hunks showing target's per-store lui/ori re-materialization vs our build's batched/reused base registers.
- result: Most of these constants appear exactly once in the .cse dump despite 3+ C-level writes to the same address across the pre-branch code and both arms, confirming cse1 forwards the address pseudo across the branch join. --diff on the s1 candidate independently confirmed target NEVER reuses a materialized scratchpad base register across two different store statements, while our build did (hunks 1-10).
- verdict: CONFIRMED

## [s2] Duplicating the six shared, unconditional pre-branch scratchpad zero-init stores into BOTH if/else arms (instead of leaving them above the branch) shrinks the CSE-shared span and lowers the honest floor, without changing program semantics (both arms always executed the zero-init before).
- mechanism: cse1 block-extension (same as above) — duplicating the statement into each arm makes the in-arm zero-store the first reference to that address within the arm's own straight-line run, changing which stores end up sharing a pseudo
- probe: Moved the six *(s32*)0x1F80036x = 0; statements from before the first if to the start of both the if and else bodies (verbatim duplicate); measured sandbox --disable all before/after.
- result: 211 -> 199 (build_insns 230 -> 246, target 252). --diff afterward: 36 hunks, 32 source-level / 4 operand-only / 0 not-scored; hunk 1 shows the same address-batching pattern now operating intra-arm rather than cross-arm (mechanism unchanged, span shrunk). This is the session's confirmed floor-lowering lever, banked live in src/code6cac_b.c and memory/grind/func_8002C22C/candidate.c.
- verdict: CONFIRMED

## [s2] Wrapping one arm's six-store zero-init block in do { ... } while (0); (FAKE-annotated per do-while-zero-exception) trips one of cse_end_of_basic_block's three backward-scan escapes (e.g. the NOTE_INSN_LOOP_END break) and closes more of the residual than plain per-arm duplication alone.
- mechanism: do-while-zero-exception (owner ruling 2026-07-06) interacting with cse.c:8109-8114's backward-scan loop-note break
- probe: Applied the wrap on top of the per-arm-duplicated (199) chassis, on the if-arm's zero-init block only, with a /* FAKE */ annotation naming the mechanism; measured sandbox --disable all before/after.
- result: 199 -> 200 (build_insns unchanged at 246) — worse, not better: same instruction count, a worse masked operand arrangement. Reverted immediately; saved to memory/grind/func_8002C22C/rejected/dowhile0-zero-init-single-arm.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s2 candidate (per-arm-duplicated zero-init chassis, D_80102314 base+offset decl), current toolchain (-mel -msoft-float), one FAKE do-while(0) construct (reverted after measurement), zero pins

## [s2] Volatile typing of the 0x1F800000-0x1F8003FF scratchpad addresses in this function is unavailable as a lever: a sibling function in the same file/cluster (func_80017FA0) already tried volatile on this exact address range and the Judge FAILed it, closing the function instead via a genuine loop.c-class goto/do-while restructuring.
- mechanism: N/A — this is a project-record precedent search, not a codegen mechanism
- probe: grep -rn scratchpad address literals (0x1F8000xx / 0x1F80036x-0x1F80037x) across src/*.c for any matched (COMPLETED-C) function already using these addresses, per the s1 frontier item.
- result: src/code6cac.c:237-238 (func_80017FA0's doc comment, same cluster/file family) states verbatim: 'This supersedes the s4 volatile form (Judge FAIL 2026-08-20 02:54, construct BANNED: volatile on scratchpad 0x1F800000-0x1F8003FF)'. That function's real closing form was a goto/do-while loop restructuring, not volatile. This directly answers and closes the s1 frontier item: the sibling exists, and its answer is a Judge FAIL on volatile for this address range, reinforcing the s1 H3 instance-kill and the standing mmio-volatile-type-level / legitimate-volatile-interrupt-touched scope exclusions for scratchpad RAM.
- verdict: KILLED
- kill_scope: class
- measured_on: N/A (precedent citation, not a measurement on this function's chassis)
- predicate_cite: src/code6cac.c:237-238

## H4 (s3, 2026-09-22) — the per-arm zero-init of 0x1F800374/0x1F800378 is genuinely DEAD
**CONFIRMED / applied.** Ground-truth read of `asm/funcs/func_8002C22C.s` lines
2-18 shows the ORIGINAL zero-init is a single unconditional 6-word block
(0x1F800360/364/368/370/374/378 all `sw zero`) BEFORE the `andi/beqz` branch —
never duplicated. The s2 candidate's per-arm-duplicated 6-word zero-init
(211->199) included 0x374/0x378 in the duplication, but the ONLY writes those
two addresses ever receive afterward are the real `d_v1`/`d_a0` stores after
the if/else join (`*(s32*)0x1F800374 = d_v1; *(s32*)0x1F800378 = d_a0; ...`),
so the per-arm zero-store to those two addresses is dead in both target's
dataflow AND ours — it was never load-bearing for the CSE-defeat effect H4/H5
in the s2 hypotheses targeted (which is about 0x360/364/368/370, the four
addresses actually READ BACK inside the arms). Dropping the two dead stores
from both arms measured `sandbox --disable all` = 196 (build_insns 238,
target 252) on the current toolchain (-mel -msoft-float), zero FAKE
constructs, zero pins — ordinary dead-code removal, not a construct change.
`kill_scope: instance` is not applicable here (this is a CONFIRMED
improvement, not a kill) but see H5/H6 below for what it ISN'T.

## H5 (s3, 2026-09-22) — literal single-unconditional-6-word zero-init (matching the .s exactly) is WORSE than per-arm duplication
**KILLED — instance.** Moved the (now 6-word) zero-init entirely out of both
arms to a single unconditional block immediately before the first `if`,
exactly mirroring `asm/funcs/func_8002C22C.s` lines 2-18's structure (single
block, no duplication, before the branch). Measured `sandbox --disable all` =
211 (build_insns 230) — WORSE than the s3 chassis's 196. This confirms (again,
after s2's do-while(0)-wrapped single-arm probe) that our fork's cse1 handles
this join-spanning single zero-init block differently than whatever produced
the target's bytes at the C level: literal structural fidelity to the target's
own join topology does NOT reproduce the target's instruction count here,
while the sanctioned per-arm duplicated-statement-into-arms spelling
([[cse-block-extension-controls-fold-span]]) does measurably better despite
not mirroring the join point. Measured on: s3 chassis with the zero-init
consolidated to one unconditional 6-word block before the `if`, current
toolchain (-mel -msoft-float), zero FAKE constructs, zero pins. Saved:
`rejected/literal-single-unconditional-6word-zero.c`. `kill_scope: instance`
— this exact single-block-before-branch spelling on this chassis; NOT a claim
that no source form can ever reproduce a single-block zero-init at target's
instruction count.

## H6 (s3, 2026-09-22) — hoisting all six per-arm scratchpad/global field reads into named locals ahead of the first store (matching the .s's literal load order) is WORSE
**KILLED — instance.** `asm/funcs/func_8002C22C.s` lines 22-33 show target
loading v1/v0/a2/a0/a1/a3(=D_80102108) all up front before any store/add
begins. Reproduced that literally: added named locals `a0, a1, a3` per arm,
preloaded all six fields before the first store, matching the target's own
apparent load order exactly. Measured `sandbox --disable all` = 211
(build_insns 231) — worse than the s3 chassis's 196. Measured on: s3 chassis
with a0/a1/a3 added and every inline `*(s32*)ADDR` read in the arm body
replaced by a reference to the matching preloaded local, current toolchain
(-mel -msoft-float), zero FAKE constructs, zero pins. Saved:
`rejected/upfront-preload-all-arm-fields.c`. `kill_scope: instance` — this
exact "hoist every field into a named local before any store" spelling on
this chassis; partial preloads (e.g. only the D_80102108/D_801020E4 constant)
were not tried and remain open frontier.

## [s3] The s2 candidate's per-arm-duplicated zero-init of 0x1F800374/0x1F800378 is dead code: those two addresses are never read back or re-stored with a real value inside either arm (their only real writes are the post-join d_v1/d_a0 stores), so the zero-store to them inside each arm is unnecessary and can be dropped without changing behavior.
- mechanism: ordinary dead-store elimination at the C source level (not a GCC-pass-dependent coercion) -- verified by reading asm/funcs/func_8002C22C.s and tracing which addresses are actually consumed inside each arm
- probe: Remove the *(s32*)0x1F800374 = 0; and *(s32*)0x1F800378 = 0; statements from both if/else arms of the s2 candidate; sandbox --disable all
- result: sandbox --disable all dropped from 199 to 196 (build_insns 246 -> 238); confirmed and applied to candidate.c
- verdict: CONFIRMED

## [s3] Restructuring the zero-init to a single unconditional 6-word block immediately before the if/else (literally mirroring asm/funcs/func_8002C22C.s lines 2-18's structure, un-duplicated) reproduces the target's instruction count better than the per-arm-duplicated 4-word form.
- mechanism: cse1's basic-block extension through a conditional-branch join label (cse_end_of_basic_block, cse.c:8102-8184) -- hypothesized to treat a single pre-branch zero-init differently than a duplicated per-arm one when forwarding/merging the scratchpad address pseudos
- probe: Move the (4-word, post-H4) zero-init out of both arms into one unconditional block before the first if; sandbox --disable all
- result: sandbox --disable all measured 211 (build_insns 230), worse than the 196 floor of the duplicated-arm chassis -- reverted
- verdict: KILLED
- kill_scope: instance
- measured_on: s3 chassis with the zero-init consolidated to a single unconditional 6-word block before the first if, current toolchain (-mel -msoft-float), zero FAKE constructs, zero pins

## [s3] Hoisting all six per-arm scratchpad/global field reads (v1, v0, a2, a0, a1, a3) into named locals immediately before the first store -- matching the literal load order visible in asm/funcs/func_8002C22C.s lines 22-33 -- reproduces the target's instruction count better than reading a0/a1/a3's values inline at point of use.
- mechanism: hypothesized register-allocation/scheduling effect of pre-materializing all arm-local values before the store/accumulate sequence begins, matching the apparent source order in the original compiled output
- probe: Declare a0, a1, a3 as named locals per arm, preload them from their scratchpad/global addresses before any store, and replace every inline *(s32*)ADDR read in the arm body with the corresponding preloaded local; sandbox --disable all
- result: sandbox --disable all measured 211 (build_insns 231), worse than the 196 floor of the inline-read chassis -- reverted
- verdict: KILLED
- kill_scope: instance
- measured_on: s3 chassis with a0/a1/a3 added as named locals in both arms replacing every inline scratchpad/global read, current toolchain (-mel -msoft-float), zero FAKE constructs, zero pins

## [s4] `sandbox --disable all --diff` on the s3 196-floor chassis shows the residual is DOMINATED by source-level hunks (28 of 36 hunks) concentrated entirely in the second if/else block (the `t0[0xA8..0xC0/4] +=` accumulate region) and its immediate surroundings — the first block (zero-init + vec-delta stores) is fully closed (no hunks before hunk 9).
- mechanism: n/a — this is a diagnostic finding, not a lever. `--diff` classes each hunk source-level (C says something different, no reg-seat/scheduling lever can close it) vs operand-only (2 hunks, 16 and 27, both inside the accumulate region) vs not-scored (none observed).
- probe: `tools/wteng.ps1 main sandbox func_8002C22C --disable all --diff` on the s3 candidate spliced into src/code6cac_b.c.
- result: Target's accumulate-region asm interleaves "load old t0 slot -> load next table field -> add -> store to slot" across BOTH tables (0x1F800060.. and d_tbl[0x234/4]..) in a single fused sequence per output field, and reads the SAME field's running total back out mid-sequence (e.g. loads t0[0xAC] together with the *next* field's source operand, before storing to t0[0xA8]) — a materially different statement order/fusion than the candidate's two-pass per-arm accumulation (all `t0[X] += a` then all `t0[X] += b`, per table, sequentially by field). This is real evidence the ORIGINAL C statement order for this block interleaves the two source tables' contributions per-field rather than doing two separate per-table passes; the residual is answerable by restructuring the C exactly as noted in the s3 frontier item #2, not by any register/scheduling lever.
- verdict: CONFIRMED (as a diagnostic fact, not a codegen lever — recorded so s5 does not re-run --diff to rediscover this)

## [s4] A decomp-permuter campaign on the s3/196-floor chassis (whole-TU, --no-prune-equivalent context; two workspace-fidelity bugs found and fixed: (1) this project's multi-line-string `__asm__ volatile(...)` asm blocks broke pycparser — joined into single-line `\n`-escaped literals codegen-neutral for func_8002C22C, verified by direct objdump diff of the func's own bytes before/after; (2) import.py's `-D__asm__(...)` stub macro doesn't fire on `__asm__ volatile(`, so 37 asm statements were re-encoded as `#pragma _permuter b64literal` via a reused script from func_8002D780/s4) does NOT find a form that improves on the real engine sandbox floor of 196.
- mechanism: the permuter's random C-source mutations (statement/declaration/expr reorderings, type randomization, staged-variable introduction) explore local rewrites; the base permuter weighted score was 13260 (NOT comparable to the engine's raw-insn-diff floor of 196 -- see reference/scoring-systems, confirmed again here) and dropped to a best-of 11665 over 794 iterations / ~28s wall (2 launch+harvest cycles, fresh seed each time, both harvested with --stop; zero orphaned campaigns).
- probe: Workspace built via `tools/decomp-permuter/import.py --keep src/code6cac_b.c asm/funcs/func_8002C22C.s` (kept in tmp/grind/func_8002C22C/s4/nonmatchings/func_8002C22C/), target.o rebuilt from tools/decomp-permuter/prelude.inc (gp=64 line stripped) + asm/funcs/func_8002C22C.s assembled with mipsel-linux-gnu-as -march=r3000 (import.py's own vr4300/mips-linux-gnu-as target.o build is WRONG for this project -- big-endian tradbigmips vs the project's little-endian tradlittlemips; rebuilding little-endian did not change the permuter's own weighted score, since its diff operates on decoded mnemonics not raw endianness, but the corrected target.o is the one banked for any future campaign on this workspace). Launched via `tools/permuter_campaign.py launch --func func_8002C22C -j 8 --stop-on-zero`, waited in-turn via the `wait` subcommand, harvested with `--stop` both times (no orphaned campaigns).
- result: The single best-scoring find (output-11665-1, permuter weighted 11665 vs base 13260) re-spliced into src/code6cac_b.c and measured on the real sandbox: still 196 (238 build insns) -- ZERO improvement. Its only structural change from the s3 candidate.c is staging `*(s32*)0x1F80005C` through a fresh once-written/once-read local (`new_var`) before the `a2 = a2 + new_var;` add in the first if-arm -- a SOTN-sanctioned named-intermediate shape (no-new-park-categories.md), but codegen-neutral here (cse.c folds it back to the same inline-read form; confirmed by identical sandbox score and identical build_insns). All other permuter finds in the 11665-13260 band were either the same score via reordering/type-randomization noise or worse; none were re-tested individually since the campaign's own weighted metric already showed no finds below the 11665 floor by score, and the anti-correlation with the real metric (established on func_8002D780's s4, reference/scoring-systems) means only genuinely novel STRUCTURAL rewrites (which the permuter's local-mutation pass set cannot produce -- it cannot restructure the accumulate region's per-field pass order into the interleaved fused form the diagnostic above identified as the real residual) would be worth individually re-scoring, and none of the finds inspected exhibited that restructuring.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3/196-floor chassis (D_80102314 base+offset decl, per-arm-duplicated 4-word zero-init), current toolchain (-mel -msoft-float), zero FAKE constructs, zero pins; permuter's best-of-794-iterations find re-spliced and measured directly

## [s4] A decomp-permuter campaign run on the s3 candidate chassis (per-arm-duplicated 4-word zero-init, D_80102314 base+offset decl, current toolchain -mel -msoft-float, zero FAKE constructs, zero pins) — two launch/harvest cycles, fresh seed each, 794 iterations total, both harvested with --stop, zero orphaned campaigns — does not produce any re-spliced form that improves the real engine sandbox score below 196 on this chassis.
- mechanism: decomp-permuter's local C-source mutation passes (statement/declaration reordering, type randomization, staged-variable introduction) explored around the existing chassis; the permuter's own weighted score (base 13260, best-of 11665) is a different metric than the engine's raw-insn-diff floor and is not directly comparable (reference/scoring-systems)
- probe: tools/decomp-permuter/import.py workspace built for func_8002C22C in tmp/grind/func_8002C22C/s4/ (two workspace-fidelity bugs found and fixed: multi-line __asm__ volatile(...) string literals joined to single-line to unbreak pycparser; asm-stub regex miss fixed via #pragma _permuter b64literal re-encoding; target.o rebuilt with mipsel-linux-gnu-as -march=r3000 to correct import.py's default big-endian vr4300 target). Launched via tools/permuter_campaign.py launch --func func_8002C22C -j 8 --stop-on-zero, waited in-turn via the wait subcommand, harvested --stop both times. Re-spliced the single best-scoring find (output-11665-1) into src/code6cac_b.c and measured sandbox --disable all directly.
- result: Best find measured 196 (build_insns 238) — identical to the s3 chassis's own floor, zero improvement. Its only structural change (staging *(s32*)0x1F80005C through a fresh once-written/once-read local before the a2 += add) is codegen-neutral here (cse.c folds it back to the same inline-read form). Reverted; src/code6cac_b.c left clean at HEAD.
- verdict: KILLED
- kill_scope: instance
- measured_on: s3/196-floor chassis (D_80102314 base+offset decl, per-arm-duplicated 4-word zero-init, current toolchain -mel -msoft-float), zero FAKE constructs, zero pins; the permuter's best-of-794-iterations find re-spliced and measured directly on the real sandbox, then reverted
