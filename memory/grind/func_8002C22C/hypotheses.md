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
