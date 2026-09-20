# Hypothesis ledger — func_800747D8

## Session 1 (recon)

### H1 — declaration fix for D_8009BD20 / D_800A35D0 (DATA MODEL signals)
**Statement:** the candidate's existing declarations
(`u8 D_8009BD20[][2]`, `s16 D_800A35D0` scalar) already match the object
model evidenced by other functions in this TU; no declaration change is
needed to make further progress.
**Mechanism:** n/a — this is an object-model check, not a codegen lever.
**Probe:** cross-checked `D_8009BD20` indexing against
`asm/funcs/func_80074488.s:150-160` (row<<1 stride-2 pair access, matches
`[][2]`); checked every `D_800A35D0` reference across the TU (this
function + func_80075F80 + func_800768DC + two already-COMPLETED-C call
sites) — every one takes only its address, so scalar-vs-array is
address-identical codegen.
**Result:** MATCHES for D_8009BD20 (measured: score 6 achieved with this
declaration, no residual hunk touches this access); D_800A35D0 measured
moot (no possible codegen difference from a size change in this TU).
**Verdict:** CONFIRMED
**kill_scope:** n/a (not a kill)
**measured_on:** candidate.c applied verbatim (minus the D_800A35DC type
fix below) to src/text1b.c, `sandbox --disable all` → score 6/208/208.

### H2 — D_800A35DC must be `s8`, not `u8`
**Statement:** the candidate's `extern u8 D_800A35DC;` conflicts with the
type already committed for this symbol elsewhere in the same TU
(`extern s8 D_800A35DC;`, at the already-COMPLETED-C body around
src/text1b.c:8974/9120) and must be corrected to `s8`.
**Mechanism:** plain C declaration-conflict (compiler error), not a
codegen question — no ambiguity.
**Probe:** applied candidate.c verbatim; cc1 build failed
(`conflicting types for D_800A35DC`); grepped every declaration of the
symbol in the TU; all other sites use `s8`.
**Result:** fixed to `s8`; build succeeds, reproduces the banked floor 6
exactly (score 6, build_insns 208 == target_insns 208).
**Verdict:** CONFIRMED
**kill_scope:** n/a (not a kill; a correction, not a probe)
**measured_on:** src/text1b.c with candidate.c applied + this one-line fix,
`sandbox --disable all` → score 6/208/208.

### H3 — selection_sound block: two-named-locals if/else (`flag`/`sound` split)
**Statement:** splitting the `field64` test value and the call-arg result
value into two distinct C locals (matching the apparent v0-vs-a0 register
split visible in target's asm) closes or narrows hunks 17-19.
**Mechanism:** value-flow structure (test value vs. result value are
logically distinct quantities) — ordinary C, not a GCC-internals lever.
**Probe:** `s32 flag = field64; s32 sound; if (flag==0) sound=4; else
sound=0;` in place of baseline's `s32 sound=field64; if(sound==0)
sound+=4; else sound-=sound;`. `sandbox --disable all`.
**Result:** score 10, build_insns 205 (3 insns SHORT of target 208) — worse
on both axes than baseline.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** this chassis (candidate.c + H2 fix), no FAKE constructs,
sandbox --disable all.
Banked: `rejected/selection_sound-two-var-split.c`.

### H4 — selection_sound block: default-then-override spelling
**Statement:** initializing the result to the delay-slot literal (4) and
overriding it in the branch (mirroring the asm's literal control-flow
order instead of H3's data-flow order) closes or narrows hunks 17-19.
**Mechanism:** same as H3 — control-flow-literal C structure.
**Probe:** `s32 flag = field64; s32 sound = 4; if (flag != 0) sound = 0;`.
**Result:** score 10, build_insns 205 — identical to H3.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** this chassis, no FAKE constructs, sandbox --disable all.
Banked: `rejected/selection_sound-default-then-override.c`.

### H5 — selection_sound block: single-shot, no intermediate `flag` local
**Statement:** removing the extra `flag` local entirely and testing
`MENU_800747D8->field64` directly in the `if` condition closes or narrows
hunks 17-19.
**Mechanism:** same as H3/H4 — minimal-local-count C structure.
**Probe:** `s32 sound; if (MENU_800747D8->field64==0) sound=4; else
sound=0;`.
**Result:** score 10, build_insns 205 — identical to H3 and H4.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** this chassis, no FAKE constructs, sandbox --disable all.
Banked: `rejected/selection_sound-single-shot-no-intermediate.c`.

## Live frontier (for next session)

1. **Hunk 9 — state-switch jtbl table-offset residual (operand-only).**
   Target `lw v0,0(at)` vs ours `lw v0,24(at)` at the `field3C`-driven
   5-case dispatch. Not yet attacked this session. Next step: read the
   `.combine`/`.jump2` dumps (`pwsh tools/grinder/dump.ps1 func_800747D8`)
   for this region — is GCC emitting its own jump table with a different
   entry count/base than target's `jtbl_80015A0C`? Compare against the
   sibling func_8006B578's rodata-split residual (jtbl_80015988) for
   whether this is the SAME class of issue (a separate-TU jump-table
   split needed) or a genuinely different case-ordering bug.
2. **Hunks 17-19 — selection_sound block (source-level + operand-only,
   3 KILLED spellings).** All 3 natural if/else respellings (H3/H4/H5)
   collapse to an identical wrong shape (score 10, build_insns 205)
   regardless of local-variable count or branch-literal placement, while
   baseline's `sound -= sound;` self-subtract reaches build_insns == 208
   (score 6, the smaller residual). Read `.combine`/`.cse` dumps for this
   block before hand-writing a 4th spelling — understand WHY the
   self-subtract avoids whatever fold collapses the natural forms by 3
   insns. Also flag `sound -= sound` itself for a cheat-smell review once
   the residual narrows further (see evidence.md session-1 entry) — it is
   not what a human would write from a spec, though it currently measures
   as the best-known honest form and was NOT changed this session.
3. **26 not-scored hunks** — masked branch/jump-target address artifacts.
   Correctly not chased (per the diff tool's own classification); no
   action needed unless a future full-build (`verify-oracle`) check
   reveals they matter after all (they shouldn't — same class as every
   other masked hunk in this project).

## [s1] The candidate's existing declarations (u8 D_8009BD20[][2], s16 D_800A35D0 scalar) already match the object model evidenced by other functions in this TU; no declaration change is needed.
- mechanism: n/a - object-model check, not a codegen lever
- probe: Cross-checked D_8009BD20 indexing against asm/funcs/func_80074488.s:150-160 (row<<1 stride-2 pair access matches [][2]); checked every D_800A35D0 reference across the TU (this function + func_80075F80 + func_800768DC + two already-COMPLETED-C call sites) - every one takes only the address.
- result: MATCHES for D_8009BD20 (score 6 achieved with this declaration, no residual hunk touches this access); D_800A35D0 is address-identical codegen regardless of scalar-vs-array size in this TU, so no measurable difference is possible here.
- verdict: CONFIRMED

## [s1] D_800A35DC must be declared s8 (not u8) to match the type already committed for this symbol elsewhere in the same TU (the already-COMPLETED-C body near src/text1b.c:8974/9120).
- mechanism: plain C declaration-conflict (compiler error), not a codegen question
- probe: Applied candidate.c verbatim; cc1 build failed with 'conflicting types for D_800A35DC'; grepped every declaration of the symbol in the TU.
- result: Fixed to s8; build succeeds and reproduces the banked floor 6 exactly (score 6, build_insns 208 == target_insns 208).
- verdict: CONFIRMED

## [s1] Splitting the selection_sound block's test value and result value into two named locals (flag = field64; sound = 4-or-0 by branch), matching the apparent v0-vs-a0 register split visible in target's asm, narrows hunks 17-19.
- mechanism: value-flow structure (test value vs result value as distinct C objects) - ordinary C
- probe: s32 flag = field64; s32 sound; if (flag==0) sound=4; else sound=0; in place of baseline's sound-=sound spelling. Measured with sandbox --disable all.
- result: score 10, build_insns 205 (3 insns short of target 208) - worse on both axes than the baseline.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis + H2 s8 fix, no FAKE constructs, sandbox --disable all

## [s1] Initializing the selection_sound result to the delay-slot literal (4) and overriding it in the branch (mirroring the asm's literal control-flow order) narrows hunks 17-19.
- mechanism: same as prior hypothesis - control-flow-literal C structure
- probe: s32 flag = field64; s32 sound = 4; if (flag != 0) sound = 0;
- result: score 10, build_insns 205 - identical to the two-var-split attempt.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis + H2 s8 fix, no FAKE constructs, sandbox --disable all

## [s1] Removing the intermediate flag local entirely and testing MENU_800747D8->field64 directly in the if condition narrows hunks 17-19.
- mechanism: same as prior hypotheses - minimal-local-count C structure
- probe: s32 sound; if (MENU_800747D8->field64==0) sound=4; else sound=0;
- result: score 10, build_insns 205 - identical to both prior attempts.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis + H2 s8 fix, no FAKE constructs, sandbox --disable all
