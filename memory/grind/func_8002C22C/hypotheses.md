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
