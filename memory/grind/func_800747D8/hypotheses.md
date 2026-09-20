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

## [s2] Reusing the SAME C variable across load, test, and result for the selection_sound block reaches build_insns == target_insns (208), matching the self-subtract baseline exactly, without the self-subtract cheat-smell.
- mechanism: n/a — ordinary C, ordinary variable reuse (single pseudo carries load+test+result); no GCC-internals coercion, no FAKE construct.
- probe: Replaced `s32 sound = field64; if (sound==0) sound+=4; else sound-=sound;` with `s32 sound = field64; if (sound==0) { sound=4; } else { sound=0; }` in candidate.c, applied to src/text1b.c, `sandbox func_800747D8 --disable all` -> score 6, build_insns 208 == target_insns 208. `--diff` shows the IDENTICAL 30-hunk classification (same 2 source-level + 2 operand-only + 26 not-scored hunks, byte-identical hunk contents) as the self-subtract baseline.
- result: CONFIRMED and banked as the new candidate.c. Strictly better than the s1 baseline: same honest floor, removes the self-subtract construct the s1 ledger flagged for future cheat-smell review (evidence.md s1: "a self-subtract instead of a plain `= 0` is not what a human would write from a spec"). That flagged concern is now resolved — no self-subtract in the candidate.
- verdict: CONFIRMED

## [s2] Introducing a FRESH, single-use, isolated test variable ("flag") separate from the result variable for the selection_sound block triggers a branchless arithmetic fold (`a0 = (flag==0)*4` via sltiu+sll) that eats 3 real instructions vs target's branching form, on this GCC 2.7.2 fork.
- mechanism: combine.c / constant-propagation recognizes the canonical "single-use-dead-after-test flag feeding a two-constant conditional-select into a separate result pseudo" shape and folds it to `sltiu`+`sll` branchless arithmetic instead of keeping cc1's normal branch+literal-set codegen. Confirmed via `--diff` on rejected/selection_sound-default-then-override.c this session: hunks 17-19 show our build's actual emitted insns are `lbu a0,0x64(v0); sltiu a0,a0,1; sll a0,a0,0x2` where target keeps `beqz v0,...` + delay-slot `li a0,4` + fallthrough `move a0,zero` (a real branch, no shift).
- probe: Re-applied rejected/selection_sound-default-then-override.c (s1's spelling, `s32 flag=field64; s32 sound=4; if(flag!=0) sound=0;`) to candidate.c's chassis, ran `sandbox func_800747D8 --disable all --diff` this session (tmp/grind/func_800747D8/s2/diff_defaultoverride.txt): score 10, build_insns 205 (3 short), hunks 17-19 show the exact branchless sltiu+sll fold. This explains (not just re-confirms) all 3 of s1's "natural if/else" rejections plus this session's own re-derivation — all 4 isolated-test-variable spellings collapse to this identical branchless shape regardless of which local holds which literal or which branch is written first.
- result: KILLED (explained). The isolated-test-variable family is dead for this block on this chassis; the same-variable-reuse spelling (see the CONFIRMED entry above) is the correct honest form and is now banked as candidate.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/selection_sound-default-then-override.c spliced onto candidate.c's chassis (s2), src/text1b.c, sandbox --disable all --diff; zero FAKE constructs anywhere in the candidate or the rejected form

## [s2] Hunk 9's jtbl_80015A0C table-offset residual (24 vs 0) is the same masked cross-TU-placement mechanism that sibling func_8006B578's jtbl_80015988 residual was (rotated 2026-09-16, its own s7/s12 evidence) — a GCC-synthesized-ADDR_VEC-vs-hand-transcribed-array placement conflict, not a pure src/text1b.c C-structure lever.
- mechanism: bb2.ld orders text1a_b_pre_rodata.o, text1b.o, text1a_b_mid_rodata.o. jtbl_80015A0C is CURRENTLY a hand-transcribed `const u32[6]` in src/text1a_b_mid_rodata.c:44-52 (not yet migrated to compiler-synthesized, unlike jtbl_80015988 which WAS migrated when func_8006B578 landed as C — see that file's own header comment). Once func_800747D8's real `switch(state)` is the committed C, GCC's own ADDR_VEC for it needs to land in build/src/text1b.o's own .rodata at exactly 0x80015A0C, which requires deleting the hand-transcribed jtbl_80015A0C array from text1a_b_mid_rodata.c in the SAME change — a precedented, already-executed-once (for jtbl_80015988) mechanical step, not a novel lever and not a wall.
- probe: Read src/text1a_b_mid_rodata.c (jtbl_80015A0C at lines 44-52, still hand-transcribed) and its header comment (documents doing this migration for jtbl_80015988 only so far). Read bb2.ld ordering (unchanged, not edited). Cross-referenced sibling func_8006B578's s7/s12 ledger entries, which independently derived and executed the identical migration for jtbl_80015988 with zero bb2.ld edits.
- result: Not attacked this session (out of the func_800747D8/src/text1b.c-only surface — the fix touches src/text1a_b_mid_rodata.c) and not measured — this is a read-only mechanism identification, not a probe with a negative result. Left on the frontier for the session that assembles the final submission: delete the jtbl_80015A0C array from text1a_b_mid_rodata.c in the same change that lands func_800747D8's C body, matching the func_8006B578 precedent exactly.
- verdict: (not scored this session — no measurement taken; see frontier)

## [s2] Reusing the SAME C variable across load, test, and result for the selection_sound block reaches build_insns == target_insns (208), matching the s1 self-subtract baseline exactly, without the self-subtract cheat-smell.
- mechanism: n/a - ordinary C, ordinary variable reuse; no GCC-internals coercion, no FAKE construct
- probe: Replaced `s32 sound = field64; if (sound==0) sound+=4; else sound-=sound;` with `s32 sound = field64; if (sound==0) { sound=4; } else { sound=0; }` in candidate.c, applied to src/text1b.c, sandbox --disable all
- result: score 6, build_insns 208 == target_insns 208; --diff shows the IDENTICAL 30-hunk classification (same 2 source-level + 2 operand-only + 26 not-scored, byte-identical hunk contents) as the self-subtract baseline
- verdict: CONFIRMED

## [s2] Introducing a fresh, single-use, isolated test variable ("flag") separate from the result variable for the selection_sound block triggers a branchless arithmetic fold (a0 = (flag==0)*4 via sltiu+sll) that eats 3 real instructions vs target's branching form, on this chassis.
- mechanism: combine.c / constant-propagation recognizes the canonical single-use-dead-after-test flag feeding a two-constant conditional-select into a separate result pseudo, and folds it to sltiu+sll branchless arithmetic instead of a branch+literal-set; confirmed via --diff on rejected/selection_sound-default-then-override.c, hunks 17-19
- probe: Re-applied rejected/selection_sound-default-then-override.c to candidate.c's chassis, ran sandbox --disable all --diff
- result: score 10, build_insns 205 (3 short); --diff shows the emitted insns are lbu a0,0x64(v0); sltiu a0,a0,1; sll a0,a0,0x2, where target keeps a real beqz branch + delay-slot li a0,4 + fallthrough move a0,zero. Explains (not just re-confirms) all 4 of the isolated-test-variable spellings banked in rejected/ (3 from s1, 1 new this session) as the same branchless-fold collapse.
- verdict: KILLED
- kill_scope: instance
- measured_on: rejected/selection_sound-default-then-override.c spliced onto candidate.c's chassis (s2), src/text1b.c, sandbox --disable all --diff; zero FAKE constructs anywhere in the candidate or the rejected form

## [s2] Branch source order (sound == 0 written first vs sound != 0 written first) has no effect on the emitted hunks 17-19 for the selection_sound block on this chassis.
- mechanism: GCC normalizes branch polarity independent of source statement order for this shape
- probe: Swapped the if/else arm order in the new same-variable candidate spelling, applied to src/text1b.c, sandbox --disable all --diff
- result: score 6, build_insns 208; hunks 17-19 byte-identical to the non-swapped form (tmp/grind/func_800747D8/s2/diff_swapped.txt)
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis with if/else arms swapped, src/text1b.c, sandbox --disable all --diff; zero FAKE constructs

## Session 3 (permuter)

### H9 — decomp-permuter's `import.py` full-TU import of src/text1b.c is broken for this function; a hand-built minimal-context workspace (candidate.c body + own decls, compiled through the real pipeline) is the working substitute
**Statement:** `python3 tools/decomp-permuter/import.py src/text1b.c asm/funcs/func_800747D8.s` (with or without `--no-prune`) produces a `nonmatchings/func_800747D8/` workspace whose generated `compile.sh` crashes maspsx (`too many values to unpack`) on this TU, both via a pruning artifact (a `.comm` line corrupted by one of the TU's many `_permuter_ignore_line __asm__(...)` canonical-asm placeholder stubs) and via a `.file` directive whose realpath'd filename contains the repo's own space-containing directory name (`Bushido Blade 2 Decompile`), which breaks maspsx's naive `line.split()` `.file`-renumbering logic (`tools/maspsx/maspsx/__init__.py:1079`). The second bug is latent (the real permuter, via `tempfile.NamedTemporaryFile`, writes candidates outside the repo path and never hits it) but the first (the pruning/placeholder corruption) is real and specific to text1b.c's size/asm-block density.
**Mechanism:** import.py's static pruning pass mishandles one of the many whole-function canonical-asm placeholder replacements it performs when scrubbing a huge multi-thousand-line TU down to only what `func_800747D8` needs; the corrupted output collides with maspsx's `.comm` parser. Neither `tools/decomp-permuter/` nor `tools/maspsx/` may be modified from this session's surface (contract: never edit `tools/`) so the fix is procedural, not a tool patch.
**Probe:** Built a minimal, self-contained `base.c` (candidate.c's body plus its own typedefs/externs/prototypes, no `#include`) in `tmp/grind/func_800747D8/s3/perm_ws/`, reusing the `target.o`/`settings.toml` that `import.py`'s first (failed) run had already generated correctly (target.o comes straight from `asm/funcs/func_800747D8.s` via `as`, independent of the base.c bug). Verified this compiles cleanly through the exact real pipeline (cpp | cc1 | prologue_fix | maspsx | multu_pad | as) and disassembles to the expected function body. Launched a real campaign against it via `tools/permuter_campaign.py launch --func func_800747D8 --dir tmp/grind/func_800747D8/s3/perm_ws -j 8` (base_score 265 in the permuter's own unmasked unstripped metric — not comparable to the sandbox's masked score 6).
**Result:** CONFIRMED as a working substitute — base_score computed, campaign ran 21,884 iterations with 5 novel finds, cleanly stopped/harvested (0 orphaned processes). Practical for any future permuter session on this function or (with the same recipe) other text1b.c functions that hit the same import.py failure.
**Verdict:** CONFIRMED

### H10 — a permuter campaign on the minimal-workspace chassis (21,884 iterations, 8 jobs, 5 novel finds, best score 100 of base 265) finds no improvement on the hunks 17-19 selection_sound register-seat/branch-shape residual
**Statement:** none of the 5 novel finds (scores 100/120/130/170/220) improve on candidate.c's honest floor of 6; the best-scoring find (100) is a pure whitespace/parenthesization reformat with zero semantic or codegen difference from candidate.c's own same-variable-reuse spelling, and the others are either dead-store injections (worse, forbidden-family shaped) or a semantically-altered reordering (moves a call inside one branch only — not equivalent code).
**Mechanism:** the permuter's random-mutation passes (perm_add_self_assignment, perm_reorder_stmts, perm_sameline, etc.) explore the same narrow local-spelling space this ledger's s1/s2 sessions already exhausted by hand (isolated test var -> branchless fold per the s2 H-mechanism entry; dead stores are a different, forbidden axis) — no mutation pass in decomp-permuter's catalog expresses the specific v0-vs-a0 register-seat swap or the beqz-vs-bnez+move branch-polarity difference the target actually has, because both are downstream RA/scheduling decisions the permuter's source-level mutations don't directly control.
**Probe:** `tools/permuter_campaign.py launch/wait/harvest --stop` on `tmp/grind/func_800747D8/s3/perm_ws`, label `s3-permuter`, `-j 8`; 4 wait windows totalling 613.8s / 21,884 iterations; telemetry in `metrics/events.jsonl`; finds under `tmp/grind/func_800747D8/s3/perm_ws/output-*`.
**Result:** KILLED (explained, not just re-confirmed) — the local C-spelling search space around the selection_sound block is now doubly exhausted (hand-written s1/s2 forms + permuter's own randomized search), converging on the same conclusion: the same-variable-reuse spelling in candidate.c is the best honest form reachable by C-structure search alone. Separately hand-tested output-170-1's `do { ... } while (0);` wrap of the if/else (a SOTN-sanctioned family per `.claude/rules/do-while-zero-exception.md`) directly on src/text1b.c: **REGRESSED score 6 -> 7** (added a real source-level instruction the target doesn't have) — banked as `rejected/selection_sound-do-while-zero-wrap.c`.
**Verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c chassis (same-variable-reuse baseline unchanged) plus the do-while(0)-wrapped variant, both on src/text1b.c, `sandbox --disable all --diff`; zero FAKE constructs in either form; campaign workspace `tmp/grind/func_800747D8/s3/perm_ws`, 0 orphaned processes at session end.

## [s3] decomp-permuter's import.py full-TU import of src/text1b.c is broken for this function (maspsx crash on a pruning artifact); a hand-built minimal-context workspace (candidate.c body + own decls, compiled through the real pipeline, reusing import.py's correctly-generated target.o) is a working substitute.
- mechanism: import.py's static pruning pass corrupts a `.comm` line while scrubbing one of text1b.c's many whole-function canonical-asm placeholder stubs (`_permuter_ignore_line __asm__(...)`) during the huge-TU minimization pass; unrelated to func_800747D8's own C. A separate latent bug (maspsx's `.file` line handler breaking on a realpath'd filename containing the repo's own space-containing directory name) does not affect real permuter runs since decomp-permuter writes candidates via tempfile.NamedTemporaryFile outside the repo path.
- probe: Built tmp/grind/func_800747D8/s3/perm_ws/base.c as a minimal self-contained C file (candidate.c's function body plus its own typedefs/externs/prototypes, no #include), verified it compiles cleanly through the exact real pipeline (cpp | cc1 | prologue_fix | maspsx | multu_pad | as) and disassembles to the expected function body, then launched a real campaign against it.
- result: CONFIRMED as a working substitute: base_score computed (265, permuter's own unmasked metric), campaign ran 21,884 iterations with 5 novel finds, cleanly stopped/harvested with 0 orphaned processes.
- verdict: CONFIRMED

## [s3] A permuter campaign on the minimal-workspace chassis (21,884 iterations, 8 jobs, 5 novel finds) finds no C-structure improvement on the hunks 17-19 selection_sound register-seat/branch-shape residual, and the sanctioned do-while(0) wrap of the if/else (one of the campaign's finds) regresses the honest floor from 6 to 7 when hand-tested on the real chassis.
- mechanism: The permuter's mutation catalog (perm_add_self_assignment, perm_reorder_stmts, perm_sameline, do-while wraps, etc.) re-explores the same narrow local-spelling space this ledger's s1/s2 sessions already exhausted by hand (isolated test var -> branchless combine.c fold per the s2 mechanism entry); no mutation pass expresses the specific v0-vs-a0 register-seat swap or beqz-vs-bnez+move branch-polarity difference the target has, since both are downstream RA/scheduling decisions untouched by source-level C mutation of this already-minimal block. The do-while(0) wrap left a genuine extra branch/jump on this already-branching if/else instead of folding away.
- probe: tools/permuter_campaign.py launch/wait/harvest --stop on tmp/grind/func_800747D8/s3/perm_ws, label s3-permuter, -j 8; 4 wait windows totalling 613.8s; telemetry in metrics/events.jsonl; finds under tmp/grind/func_800747D8/s3/perm_ws/output-*. Then hand-applied output-170-1's do-while(0) wrap verbatim to src/text1b.c and measured with sandbox --disable all --diff.
- result: Best permuter find (score 100 of base 265) is a pure whitespace/parenthesization reformat with zero codegen difference from candidate.c. Other finds are either dead-store injections (worse, forbidden-family-shaped) or a semantically-altered call reordering. The do-while(0) wrap measured score 6 -> 7 (regression, one extra real instruction) -- banked as rejected/selection_sound-do-while-zero-wrap.c. Floor confirmed unchanged at 6 after reverting.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (same-variable-reuse baseline, unchanged) plus the do-while(0)-wrapped variant, both applied to src/text1b.c; zero FAKE constructs in either form; sandbox --disable all --diff; campaign workspace tmp/grind/func_800747D8/s3/perm_ws, 0 orphaned processes at session end.

## s4 (enumerate modality, 2026-09-20)

- statement: Reordering the field67 block's (`case 1: if ((ret & 0xFF) != 0) { ... }`, currently byte-matching, zero diff vs target) four plain-assign statements (`menu = (S_800747D8 *)D_800A36A0;` x2, `work = (S_800747D8 *)D_800A36A0;`, `row = work->field67;`) into any of the 6 valid def-before-use orderings that hoist them ahead of the interleaved `menu->field67 += 1;` / `menu->field67 &= 1;` compound-assigns is worse than the current interleaved spelling, on this chassis.
- mechanism: hoisting the two identical `menu = (S_800747D8 *)D_800A36A0;` reloads together (no longer separated by an intervening compound-assign statement) lets GCC's local-CSE (cse.c) merge them into a single load, changing register/instruction count (208 -> 206 build_insns) even though the reload was already redundant value-wise; the interleaved placement in the current candidate.c defeats that merge and is required to keep the block at its current (already correct) byte count.
- probe: `tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum --no-swaps` (6 distinct spellings from 4 assigns, 0 named decls) then `tools/sweep_variants.py --func func_800747D8 --file text1b --variants tmp/grind/func_800747D8/s4/enum --json`.
- result: baseline 6; all 6 variants scored 24 (build_insns 206, worse in both score and instruction count). Zero improvement, all strictly worse. This block was NOT part of the open residual (it already matches target) — this was a control run validating the new enum+sweep tool pairing works correctly and confirming the current candidate.c's exact statement order in this block is load-bearing.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis with the field67 block's statement order permuted (6/6 valid orderings tested), src/text1b.c, sandbox --disable all; zero FAKE constructs.

- statement: The actual open residual (hunks 17-19, the `selection_sound` block: `s32 sound = MENU_800747D8->field64; if (sound == 0) { sound = 4; } else { sound = 0; } func_8005C650(sound, 0x7F, 0x7F);`) cannot be usefully swept by `tools/spelling_enum.py` because the tool's statement model has no concept of if/else nesting — it always sorts every decl/assign statement ahead of every "anchor" line (which is what an un-parsed `if (...) {` / `} else {` / `}` becomes), so any region containing per-arm assignments inside a conditional gets its assigns hoisted OUT of the branches, changing behavior (or, when the tool's inlining axis drops the region's only named local while leaving an assign to that now-undeclared name, producing invalid C).
- mechanism: `spelling_enum.py`'s `orderings()` (spelling_enum.py:128-159) unconditionally appends all collected `anchors` after all permuted decls/assigns — a structural limitation of the tool for control-flow-bearing regions, not a GCC codegen mechanism.
- probe: `tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum_ss --no-swaps` (region = the selection_sound block's decl + if/else + call) -> 4 variants; manually inspected all 4 (v0-v3) rather than blind-sweeping them, since v0/v1 hoist `sound = 4;`/`sound = 0;` out of the if/else (semantically wrong — the diamond becomes a no-op) and v2/v3 drop the `s32 sound = ...;` declaration while keeping the now-orphaned `sound = 4;`/`sound = 0;` assigns (invalid C — confirmed by an accidental partial-apply during `sweep_variants.py` producing `text1b.c:8475: 'sound' undeclared` when the tool's restore-on-finally hit an unrelated transient `OSError: [Errno 22] Invalid argument` mid-run on the WSL/NTFS 9p mount; manually reverted src/text1b.c to candidate.c's known-good text and re-verified floor 6 before continuing).
- probe continued: no sandbox measurement was taken on v0-v3 since manual inspection already showed 2/4 semantically wrong and 2/4 non-compiling — sweeping them would only reproduce the same non-informative result the tool already structurally guarantees for this region shape.
- result: no new evidence for or against any hunk-17-19 respelling. The s1 (3 kills), s2 (2 kills), s3 (permuter 21,884 iters + do-while(0) hand-test) evidence remains the complete search record for this block. The `enumerate` modality's tool is confirmed NOT applicable to if/else-diamond-shaped residuals without a region-shape restriction the tool does not currently support (a future improvement would need to treat each branch arm as its own flat sub-region, out of scope for a src/text1b.c-only session).
- verdict: KILLED
- kill_scope: instance
- measured_on: N/A (no compiling/semantically-valid variant existed to measure; the negative result is about tool applicability, not a measured chassis form) — recorded as an instance kill of "spelling_enum.py sweep of the selection_sound if/else region" specifically, not of any respelling within it.

## [s4] Reordering the field67 block's (switch(state) case 1, currently byte-matching, zero diff vs target) four plain-assign statements into any of the 6 valid def-before-use orderings that separate the two identical 'menu = (S_800747D8 *)D_800A36A0;' reloads from the intervening 'menu->field67 += 1;' / '&= 1;' compound-assigns is worse than the current interleaved spelling.
- mechanism: GCC's local CSE (cse.c) merges the two identical menu reloads into one once they are no longer separated by an intervening statement, changing build_insns from 208 to 206 and the sandbox score from 6 to 24 -- the interleaved placement in candidate.c defeats that merge and is required to keep this block at its current (already correct) instruction count.
- probe: tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum --no-swaps (6 spellings) then tools/sweep_variants.py --func func_800747D8 --file text1b --variants tmp/grind/func_800747D8/s4/enum --json
- result: baseline score 6; all 6 generated orderings scored 24 with build_insns 206 (strictly worse on both score and instruction count, zero improving variants).
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis with the field67 block's 4 plain-assign statements permuted through all 6 valid def-before-use orderings, src/text1b.c, sandbox --disable all, zero FAKE constructs

## [s4] tools/spelling_enum.py's flat statement model (orderings() unconditionally sorts every decl/assign ahead of every anchor line) cannot usefully enumerate the selection_sound if/else diamond (hunks 17-19's open residual) because the region's assigns live inside conditional branch arms -- hoisting them out changes program behavior or, when the tool's inlining axis also drops the block's only named local, produces C that fails to compile.
- mechanism: spelling_enum.py:128-159 orderings() structurally appends the collected anchor list after all permuted decl/assign statements regardless of their original position relative to control-flow anchors -- a tool limitation, not a GCC codegen mechanism.
- probe: tools/spelling_enum.py --candidate tmp/grind/func_800747D8/s4/enum_input.c --out tmp/grind/func_800747D8/s4/enum_ss --no-swaps on the selection_sound decl+if/else+call region -> 4 variants, manually inspected (not blind-swept).
- result: 2 of 4 variants (v0, v1) hoist 'sound = 4;'/'sound = 0;' out of the if/else, leaving an empty-armed diamond that changes behavior; 2 of 4 (v2, v3) drop the 's32 sound = ...;' declaration entirely while keeping the now-orphaned assigns, which is invalid C (confirmed via an accidental partial-apply during sweep_variants.py's restore step hitting a transient WSL/NTFS OSError mid-run: build failed with "text1b.c:8475: 'sound' undeclared"; src/text1b.c was manually reverted to candidate.c's known-good text and floor 6 was re-verified before continuing). No sandbox measurement of v0-v3 was taken since manual inspection already showed all 4 non-informative for this residual.
- verdict: CONFIRMED
